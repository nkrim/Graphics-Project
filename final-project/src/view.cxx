/*! \file view.c
 *
 * \brief This file defines the viewer operations.
 *
 * \author John Reppy
 */

/* CMSC23700 Final Project sample code (Autumn 2015)
 *
 * COPYRIGHT (c) 2015 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#include "cs237.hxx"
#include "view.hxx"
#include "map-cell.hxx"
#include "buffer-cache.hxx"
#include "texture-cache.hxx"
#include <map>


static void Error (int err, const char *msg);
static GLFWwindow *InitGLFW (int wid, int ht, const char *title);


float sign (cs237::vec2f p1, cs237::vec2f p2, cs237::vec2f p3) {
    return (p1[0] - p3[0]) * (p2[1] - p3[1]) - (p2[0] - p3[0]) * (p1[1] - p3[1]);
}

bool PointInTriangle (cs237::vec2f pt, cs237::vec2f v1, cs237::vec2f v2, cs237::vec2f v3) {
    bool b1, b2, b3;

    b1 = sign(pt, v1, v2) >= 0.0f;
    b2 = sign(pt, v2, v3) >= 0.0f;
    b3 = sign(pt, v3, v1) >= 0.0f;

    return ((b1 == b2) && (b2 == b3));
}


bool PointInAABBxz(cs237::AABBd box, cs237::vec3d pos) {
    float minx = box.min()[0];
    float maxx = box.max()[0];
    float minz = box.min()[2];
    float maxz = box.max()[2];

    // printf("%5.2f, %5.2f, %5.2f, %5.2f\n", minx, maxx, minz, maxz);

    return minx <= pos[0] &&
           maxx >= pos[0] &&
           minz <= pos[2] &&
           maxz >= pos[2];
}

cs237::vec3f vec3fFromVertex(Vertex v) {
    return cs237::vec3f(v._x, v._y, v._z);
}

cs237::vec2f vec3fxz(cs237::vec3f v) {
    return cs237::vec2f(v[0], v[2]);
}




// animation time step (100Hz)
// #define TIME_STEP	0.001


#ifndef SHADER_FILE_BASE
    #define SHADER_FILE_BASE "../shaders/"
#endif


// Load Shader
cs237::ShaderProgram *LoadShader (std::string const & shaderPrefix) {
    static std::map<std::string, cs237::ShaderProgram *> Shaders;

    std::string shader_path = SHADER_FILE_BASE + shaderPrefix;

    auto it = Shaders.find(shaderPrefix);
    if (it == Shaders.end()) {
          // load, compile, and link the shader program
        cs237::VertexShader vsh((shader_path + ".vsh").c_str());
        cs237::FragmentShader fsh((shader_path + ".fsh").c_str());
        cs237::ShaderProgram *shader;

        shader = new cs237::ShaderProgram (vsh, fsh);

        if (shader == nullptr) {
            std::cerr << "Cannot build " << shaderPrefix << std::endl;
            exit (1);
        }
        Shaders.insert (std::pair<std::string, cs237::ShaderProgram *>(shaderPrefix, shader));
        return shader;
    }
    else {
        return it->second;
    }

}

/***** class View member functions *****/

View::View (Map *map)
    : _map(map), _errorLimit(2.0), _isVis(true), _window(nullptr),
      _bCache(new BufferCache()), _tCache(new TextureCache()) {
}

void View::Init (int wid, int ht) {
    //printf("view init\n");

    this->_window = InitGLFW(wid, ht, this->_map->Name().c_str());

    // attach the view to the window so we can get it from callbacks
    glfwSetWindowUserPointer (this->_window, this);

    // Compute the bounding box for the entire map
    this->_mapBBox = cs237::AABBd(
    	cs237::vec3d(0.0, double(this->_map->MinElevation()), 0.0),
    	cs237::vec3d( double(this->_map->hScale()) * double(this->_map->Width()),
              	    double(this->_map->MaxElevation()),
              	    double(this->_map->hScale()) * double(this->_map->Height())));

    // Place the viewer in the center of cell(0,0), just above the
    // cell's bounding box.
    cs237::AABBd bb = this->_map->Cell(0,0)->Tile(0).BBox();
    cs237::vec3d pos = bb.center();
    pos.y = bb.maxY() + 0.01 * (bb.maxX() - bb.minX());

    // The camera's direction is toward the bulk of the terrain
    cs237::vec3d  at;
    if ((this->_map->nRows() == 1) && (this->_map->nCols() == 1)) {
        at = pos + cs237::vec3d(1.0, -0.25, 1.0);
    }
    else {
        at = pos + cs237::vec3d(double(this->_map->nCols()-1), 0.0, double(this->_map->nRows()-1));
    }
    this->_cam.move(pos, at, cs237::vec3d(0.0, 1.0, 0.0));

    // set the FOV and near/far planes
    // TODO check if this is right
    this->_cam.setFOV (60.0);
    this->_cam.setNearFar (10.0, 10.5 * double(this->_map->CellWidth()) * double(this->_map->hScale()));
    this->Resize (wid, ht);

    _cam.setPhi();

    _translation = cs237::vec3f(0.0f, _map->BaseElevation(), 0.0f);
    _scale = cs237::vec3f(_map->hScale(), _map->vScale(), _map->hScale());
    _scaleToWorld = cs237::scale(cs237::translate(cs237::mat4x4f(1.0f), _translation), _scale);

    // Initialize Shader
    _shader = LoadShader("default");
    _boxshader = LoadShader("skybox");
    _grassshader = LoadShader("grass");
    _rainshader = LoadShader("rain");

    WIREFRAME = true;
    SHADOWING = true;
    FOG = _map->hasFog();

    GRASS = false;
    RAIN = false;
    _rain_t = 0.0f;

    if (FOG) {
        this->_fogColor = _map->FogColor();
        this->_fogDensity = _map->FogDensity();
    }




    cs237::image2d *img = new cs237::image2d("../data/water1a.png");
    _water1 = new cs237::texture2D(GL_TEXTURE_2D, img);
    _water1->Parameter (GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    _water1->Parameter (GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    cs237::image2d *img2 = new cs237::image2d("../data/water2a.png");
    _water2 = new cs237::texture2D(GL_TEXTURE_2D, img2);
    _water2->Parameter (GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    _water2->Parameter (GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    cs237::image2d *img3 = new cs237::image2d("../data/grass_detail.png");
    _grassD = new cs237::texture2D(GL_TEXTURE_2D, img3);
    _grassD->Parameter (GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    _grassD->Parameter (GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    cs237::image2d *img4 = new cs237::image2d("../data/gravel_detail.png");
    _gravelD = new cs237::texture2D(GL_TEXTURE_2D, img4);
    _gravelD->Parameter (GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    _gravelD->Parameter (GL_TEXTURE_MAG_FILTER, GL_LINEAR);



    // set uniform locations and stuff
    this->_mvLoc = _shader->UniformLocation("modelView");
    this->_twLoc = _shader->UniformLocation("toWorld");
    this->_tLoc = _shader->UniformLocation("t");
    this->_scalingLoc = _shader->UniformLocation("scalingVector");
    this->_timeLoc = _shader->UniformLocation("timestamp");

    this->_projLoc = _shader->UniformLocation("projection");
    this->_colLoc = _shader->UniformLocation("vColor");
    this->_fogDensityLoc = _shader->UniformLocation("fogDensity");
    this->_fogColorLoc = _shader->UniformLocation("fogColor");
    this->_camLoc = _shader->UniformLocation("camLoc");

    this->_dirLoc = _shader->UniformLocation("direction");
    this->_ambLoc = _shader->UniformLocation("ambient");
    this->_intLoc = _shader->UniformLocation("intensity");


    this->_texLoc = _shader->UniformLocation("diffuseTexture");
    this->_normLoc = _shader->UniformLocation("normalTexture");

    this->_water1Loc = _shader->UniformLocation("water1Texture");
    this->_water2Loc = _shader->UniformLocation("water2Texture");

    this->_grassDLoc = _shader->UniformLocation("grassDTexture");
    this->_gravelDLoc = _shader->UniformLocation("gravelDTexture");

    this->_heightLoc = _shader->UniformLocation("heightFieldTexture");
    this->_watermaskLoc = _shader->UniformLocation("waterMaskTexture");

    this->_tileWidthLoc = _shader->UniformLocation("tileWidth");
    this->_cellWidthLoc = _shader->UniformLocation("cellWidth");
    this->_tileNWCLoc = _shader->UniformLocation("tileNWC");
    this->_tileNWRLoc = _shader->UniformLocation("tileNWR");

    this->_multOffsetLoc = _shader->UniformLocation("tCoordMultOffset");
    this->_addOffsetXLoc = _shader->UniformLocation("tCoordAddOffsetX");
    this->_addOffsetZLoc = _shader->UniformLocation("tCoordAddOffsetZ");

    this->_wireframeLoc = _shader->UniformLocation("wireframe_mode");
    this->_shadowingLoc = _shader->UniformLocation("shadowing_mode");
    this->_fogLoc = _shader->UniformLocation("fog_mode");
    this->_reflectionLoc = _shader->UniformLocation("isReflection");

    this->_lodLoc = _shader->UniformLocation("levelOfDetail");
    this->_rainLoc = _shader->UniformLocation("isRain");
    this->_hasWaterLoc = _shader->UniformLocation("hasWaterMap");
    this->_rainTLoc = _shader->UniformLocation("rain_t");

    //skybox variables
    this->_boxmvLoc = _boxshader->UniformLocation("modelView");
    this->_boxprojLoc = _boxshader->UniformLocation("projection");
    this->_boxTexture = _boxshader->UniformLocation("diffuseTexture");
    this->_boxFogColorLoc = _boxshader->UniformLocation("fogColor");
    this->_boxFaceLoc = _boxshader->UniformLocation("boxFace");
    this->_boxFogLoc = _boxshader->UniformLocation("hasFog");
    this->_boxDeltaLoc = _boxshader->UniformLocation("delta");
    this->_boxAlphaLoc = _boxshader->UniformLocation("alpha");
    this->_boxRainLoc = _boxshader->UniformLocation("isRain");
    this->_boxRainTLoc = _boxshader->UniformLocation("rain_t");

    // grass variables
    this->_gmvLoc = _grassshader->UniformLocation("modelView");
    this->_gprojLoc = _grassshader->UniformLocation("projection");
    this->_gtextLoc = _grassshader->UniformLocation("grassTexture");
    this->_gtimeLoc = _grassshader->UniformLocation("timestamp");
    this->_gtboLoc = _grassshader->UniformLocation("grassTBO");
    this->_gCamPosLoc = _grassshader->UniformLocation("camPos");

    // rain variables
    this->_rmvLoc = _rainshader->UniformLocation("modelView");
    this->_rprojLoc = _rainshader->UniformLocation("projection");
    this->_rtextLoc = _rainshader->UniformLocation("rainTexture");
    this->_rtimeLoc = _rainshader->UniformLocation("timestamp");
    this->_rtboLoc = _rainshader->UniformLocation("TextureBufferObject");


        //Morph delt uniforms
    this->_tDeltLoc = _shader->UniformLocation("tDelt");
    this->_pTexLoc = _shader->UniformLocation("parentTexture");
    this->_tileNWCwrtPLoc = _shader->UniformLocation("tileNWCwrtP");
    this->_tileNWRwrtPLoc = _shader->UniformLocation("tileNWRwrtP");
    this->_blendTexLoc = _shader->UniformLocation("blendTextures");





    // this->_ = _grassshader->UniformLocation("");

    this->_init_cam_pos = d_to_f(_cam.position());

    this->_shader->Use();

    /* ADDITIONAL INITIALIZATION */

    // put vertex buffers and shit into the whatever.

    // Initialize textures per cell
    for (auto r = 0; r < _map->nRows(); r++) {
      for (auto c = 0; c < _map->nCols(); c++) {
        class Cell *cell = _map->Cell(r, c);
        cell->InitTextures(this);

        // Go through all tiles and assign pointers to the textures they will be using
        AssignTextures(cell);
      }
    }




    // create standardized view frustum.
    _stdVF = new ViewFrustum();

    _stdVF->Update(this->_cam);
    _stdVF->tang = tan(_cam.radHalfFOV());


    // Assign visible tiles VAO pointers - uses frustrum culling
    this->CreateVAOs();




    CS237_CHECK( glGenBuffers(1, &this->_texBufObj) );
    CS237_CHECK( glBindBuffer(GL_TEXTURE_BUFFER, this->_texBufObj) );
    CS237_CHECK( glBufferData(GL_TEXTURE_BUFFER, (NUM_GRASS + NUM_DROPS) * sizeof(cs237::vec4f), NULL, GL_DYNAMIC_COPY) );
    CS237_CHECK( glBindBuffer(GL_TEXTURE_BUFFER, 0) );

    CS237_CHECK( glGenTextures(1, &this->_texBuf) );
    CS237_CHECK( glBindTexture(GL_TEXTURE_BUFFER, this->_texBuf) );
    CS237_CHECK( glBindTexture(GL_TEXTURE_BUFFER, 0) );

                                            //  float
    CS237_CHECK( glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, this->_texBufObj) );



    // load the skybox
    _skybox = new SkyBox();
    _skybox->InitCoords(_cam);
    _skybox->Init();
    _skybox->Load();

    // load grass



    _rain = new Rain();
    _rain->Init(this);
    _rain->InitCoords(this);

    _rain->Load();

}

#define SIN_ONE_DEGREE	0.0174524064373f
#define COS_ONE_DEGREE	0.999847695156f
#define SQRT_2 1.41421356237f
static bool init_grass = true;

void View::HandleKey (int key, int scancode, int action, int mods) {

    switch (key) {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
          	if (mods == 0)
          	    glfwSetWindowShouldClose (this->_window, true);
          	break;
        case GLFW_KEY_F: // toggle wireframe mode
          	this->FOG = !this->FOG;
          	break;
        case GLFW_KEY_G: // toggle wireframe mode
            this->GRASS = !this->GRASS;
            if (init_grass) {
                _grass = new Grass();
                _grass->Init(this);
                _grass->InitCoords(this);
                _grass->Load();
                init_grass = false;
            }


            break;
        case GLFW_KEY_B: // toggle wireframe mode
            this->RAIN = !this->RAIN;
            break;
        case GLFW_KEY_L: // toggle wireframe mode
            this->SHADOWING = !this->SHADOWING;
            break;
        case GLFW_KEY_W: // toggle wireframe mode
            this->WIREFRAME = !this->WIREFRAME;
            break;

        case GLFW_KEY_EQUAL:
          	if (mods == GLFW_MOD_SHIFT) { // shift+'=' is '+'
          	  // decrease error tolerance
          	    if (this->_errorLimit > 0.5)
          		this->_errorLimit /= SQRT_2;
          	}
          	break;
        case GLFW_KEY_KP_ADD:  // keypad '+'
          	if (mods == 0) {
          	  // decrease error tolerance
          	    if (this->_errorLimit > 0.5)
          		this->_errorLimit /= SQRT_2;
          	}
          	break;
        case GLFW_KEY_MINUS:
          	if (mods == 0) {
          	  // increase error tolerance
          	    this->_errorLimit *= SQRT_2;
          	}
          	break;
        case GLFW_KEY_KP_SUBTRACT:  // keypad '-'
          	if (mods == 0) {
          	  // increase error tolerance
          	    this->_errorLimit *= SQRT_2;
          	}
          	break;
        default: // ignore all other keys
            // std::cout << _cam;
            break;
    }

//std::cout << "after handleKey: " << this->_cam << std::endl;

}

void View::HandleMouseEnter (bool entered) {
}

void View::HandleMouseMove (double x, double y) {
}

void View::HandleMouseButton (int button, int action, int mods) {
}

void View::Animate () {
    // wtf
 //    double now = glfwGetTime();
 //    double dt = now - this->_lastStep;
 //    if (dt >= TIME_STEP) {
	// this->_lastStep = now;
	// /* ANIMATE STUFF */
 //    }

}

void View::Resize (int wid, int ht) {
    glfwGetFramebufferSize (this->_window, &this->_fbWid, &this->_fbHt);
    glViewport(0, 0 , this->_fbWid, this->_fbHt);

  // default error limit is 1%
    this->_errorLimit = float(this->_fbHt) / 100.0f;

    this->_cam.setViewport (this->_fbWid, this->_fbHt);

}

/***** Local utility functions *****/

static void Error (int err, const char *msg) {
    std::cerr << "[GLFW ERROR " << err << "] " << msg << "\n" << std::endl;
}

static GLFWwindow *InitGLFW (int wid, int ht, const char *title) {
    glfwSetErrorCallback (Error);

    glfwInit ();

  // Check the GLFW version
    {
	int major;
	glfwGetVersion (&major, NULL, NULL);
	if (major < 3) {
	    std::cerr << "CS237 requires GLFW 3.0 or later\n" << std::endl;
	    exit (EXIT_FAILURE);
	}
    }

    glfwWindowHint (GLFW_RESIZABLE, GL_TRUE);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // glfwWindowHint(GLFW_SAMPLES, 4);


    GLFWwindow *window = glfwCreateWindow(wid, ht, title, NULL, NULL);
    if (window == nullptr) {
	exit (EXIT_FAILURE);
    }



    glfwMakeContextCurrent (window);

  // Check the OpenGL version
    {
	GLint major, minor;
	glGetIntegerv (GL_MAJOR_VERSION, &major);
	glGetIntegerv (GL_MINOR_VERSION, &minor);
	if ((major < 4) || ((major == 4) && (minor < 1))) {
	    std::cerr << "CS237 requires OpenGL 4.1 or later; found " << major << "." << minor << std::endl;
	    exit (EXIT_FAILURE);
	}
    }

    return window;
}






// i.e. the init textures
void View::CreateVAOs ()    {
    for (auto r = 0; r < _map->nRows(); r++) {
        for (auto c = 0; c < _map->nCols(); c++) {
            // this is a cell
            class Cell *cell = _map->Cell(r, c);
            // recurse through cell's root tile to find appropriate LOD to render
            // get the vector of tiles to make vaos for
            std::vector<Tile *> ts = this->GetLODTiles(&cell->Tile(0));
            // assign vaos with each tile's data
            for(auto it = ts.begin(); it != ts.end(); it++) {
                Tile *t = (*it);
                // Acquire a new VAO and load it with this tile's information
                VAO *v = this->_bCache->Acquire();
                v->Load(t->Chunk()); //Load tile data into vao

                // activate textures
                if(! t->normTex->isActive())  {
                    t->normTex->Activate();
                    t->colTex->Activate();
                }

                // Set vao pointer in the tile
                t->vao = v;

                this->tiles.push_back(t);
            }
        }
    }
}

// fucking magic
void View::UpdateVAOs ()    {
    // Amount tDelt is changed by each pass
    float tdChange = 0.05;
    // Temporary vector for holding new tiles to be used - will be swapped with this->tiles
    std::vector<Tile *> tempTiles;

    for (auto r = 0; r < _map->nRows(); r++) {
        for (auto c = 0; c < _map->nCols(); c++) {
            // this is a cell
            class Cell *cell = _map->Cell(r, c);

            std::vector<Tile *> ts = this->GetLODTiles(&cell->Tile(0));

            for(auto it = ts.begin(); it != ts.end(); it++) {
                Tile *t = (*it);
                // Already has been rendered in the scene, so just readd it
                if(t->tDelt >= 1)   {
                    // If the vao doesn't exist, acquire a new one and load information into it
                    if(t->vao == nullptr) {
                        VAO *v = this->_bCache->Acquire();
                        v->Load(t->Chunk()); //Load chunk data into vao

                        if(! t->normTex->isActive())  {
                            t->normTex->Activate();
                            t->colTex->Activate();
                        }

                        t->vao = v;
                    }
                    // Set inUse boolean tag for the tile
                    t->inUse = true;
                    t->tDelt = 1;
                    // Add to tempTiles vector
                    tempTiles.push_back(t);
                }
                // Is currently being morphed from it's children or is ready to be rendered
                else if(t->tDelt <= -1) {
                    // If morphing is done, render tile
                    if(t->Child(0)->tDelt + tdChange >= 1)    {
                        // If the vao doesn't exist, acquire a new one and load information into it
                        if(t->vao == nullptr) {
                            VAO *v = this->_bCache->Acquire();
                            v->Load(t->Chunk()); //Load chunk data into vao

                            if(! t->normTex->isActive())  {
                                t->normTex->Activate();
                                t->colTex->Activate();
                            }

                            t->vao = v;
                        }
                        // Set inUse boolean tag for the tile
                        t->inUse = true;
                        // Set tDelt of tile to 1 to show that it is now being rendered
                        t->tDelt = 1;
                        // Add to tempTiles vector
                        tempTiles.push_back(t);
                    }
                    // Morphing is not done, render it's children
                    else    {
                        for(int i=0;i<t->NumChildren();i++) {
                            Tile *c = t->Child(i);

                            c->tDelt += tdChange;

                            // If the vao doesn't exist, acquire a new one and load information into it
                            if(c->vao == nullptr) {
                                VAO *v = this->_bCache->Acquire();
                                v->Load(c->Chunk()); //Load chunk data into vao

                                if(! c->normTex->isActive())  {
                                    c->normTex->Activate();
                                    c->colTex->Activate();
                                }

                                c->vao = v;
                            }
                            // Set inUse boolean tag for the tile
                            c->inUse = true;
                            // Add to tempTiles vector
                            tempTiles.push_back(c);
                        }
                    }
                }
                // Is not currently morphing to anything, have to find out where it came from and perform proper morphing
                else if(t->tDelt == 0)  {
                    Tile *p = &cell->Tile(QTree::Parent(t->ID()));
                    // If the parent was previosuly active last frame, morph from it regardless
                    if(t->LOD() != 0 && p->tDelt != 0)  {
                        // If the vao doesn't exist, acquire a new one and load information into it
                        if(t->vao == nullptr) {
                            VAO *v = this->_bCache->Acquire();
                            v->Load(t->Chunk()); //Load chunk data into vao

                            if(! t->normTex->isActive())  {
                                t->normTex->Activate();
                                t->colTex->Activate();
                            }

                            t->vao = v;
                        }
                        // Set inUse boolean tag for the tile
                        t->inUse = true;
                        // Set tDelt of tile to -tdChange because it is just beginning to morph
                        t->tDelt = -tdChange;
                        // Add to tempTiles vector
                        tempTiles.push_back(t);
                    }
                    // If the children were previsouly active last frame, figure out what to do (will eventually morph from children)
                    else if(t->NumChildren() != 0 && t->Child(0)->tDelt != 0)    {
                        // Set this tile's tDelt to -1 to show that it is being morphed to
                        t->tDelt = -1;

                        float ctDelt = t->Child(0)->tDelt;

                        // Child is fully rendered or is being morphed too from it's own child, morph from children
                        if(ctDelt >= 1 || ctDelt <= -1)    {
                            for(int i=0;i<t->NumChildren();i++) {
                                Tile *c = t->Child(i);
                                // Since this is guarenteed to be the start of a new morphing, start the tDelt from tdChange
                                c->tDelt = tdChange;

                                // If the vao doesn't exist, acquire a new one and load information into it
                                if(c->vao == nullptr) {
                                    VAO *v = this->_bCache->Acquire();
                                    v->Load(c->Chunk()); //Load chunk data into vao

                                    if(! c->normTex->isActive())  {
                                        c->normTex->Activate();
                                        c->colTex->Activate();
                                    }

                                    c->vao = v;
                                }
                                // Set inUse boolean tag for the tile
                                c->inUse = true;
                                // Add to tempTiles vector
                                tempTiles.push_back(c);
                            }
                        }
                        // Child is morphing from this tile, morph back
                        else if(ctDelt < 0) {
                            for(int i=0;i<t->NumChildren();i++) {
                                Tile *c = t->Child(i);
                                // Since this is morphing from the parent, it is negative, so negate it to change direction and add tdChange
                                c->tDelt = (-1 * c->tDelt) + tdChange;

                                // If the vao doesn't exist, acquire a new one and load information into it
                                if(c->vao == nullptr) {
                                    VAO *v = this->_bCache->Acquire();
                                    v->Load(c->Chunk()); //Load chunk data into vao

                                    if(! c->normTex->isActive())  {
                                        c->normTex->Activate();
                                        c->colTex->Activate();
                                    }

                                    c->vao = v;
                                }
                                // Set inUse boolean tag for the tile
                                c->inUse = true;
                                // Add to tempTiles vector
                                tempTiles.push_back(c);
                            }
                        }
                        // Child is morphing to this tile already, continue morphing to this tile
                        else    {
                            for(int i=0;i<t->NumChildren();i++) {
                                Tile *c = t->Child(i);
                                // Since this is morphing from the parent, it is negative, so negate it to change direction and add tdChange
                                c->tDelt += tdChange;

                                // If the vao doesn't exist, acquire a new one and load information into it
                                if(c->vao == nullptr) {
                                    VAO *v = this->_bCache->Acquire();
                                    v->Load(c->Chunk()); //Load chunk data into vao

                                    if(! c->normTex->isActive())  {
                                        c->normTex->Activate();
                                        c->colTex->Activate();
                                    }

                                    c->vao = v;
                                }
                                // Set inUse boolean tag for the tile
                                c->inUse = true;
                                // Add to tempTiles vector
                                tempTiles.push_back(c);
                            }
                        }
                    }
                    // Otherwise, an LOD has probably been skipped, so just flat-out render this tile
                    else    {
                        // If the vao doesn't exist, acquire a new one and load information into it
                        if(t->vao == nullptr) {
                            VAO *v = this->_bCache->Acquire();
                            v->Load(t->Chunk()); //Load chunk data into vao

                            if(! t->normTex->isActive())  {
                                t->normTex->Activate();
                                t->colTex->Activate();
                            }

                            t->vao = v;
                        }
                        // Set inUse boolean tag for the tile
                        t->inUse = true;
                        // Set tDelt of tile to 1 to show that it is now being rendered
                        t->tDelt = 1;
                        // Add to tempTiles vector
                        tempTiles.push_back(t);
                    }
                }
                // This tile is currently morphing, so we want to morph back into itself or continue morphing into itself
                else    {
                    // If currently morphing to parent, set parent's tDelt from -1 to 1, and negate tile's tDelt to adjust for change in direction
                    if(t->tDelt > 0)    {
                        (&cell->Tile(QTree::Parent(t->ID())))->tDelt = 0;
                        // Negate tDelt because direction changed
                        t->tDelt = 1 - t->tDelt;
                    }
                    // Morph back towards the tile
                    t->tDelt -= tdChange;
                    // If fully morphed, render tile as normal
                    if(t->tDelt <= -1)  {
                        t->tDelt = 1;
                    }
                    // If the vao doesn't exist, acquire a new one and load information into it
                    if(t->vao == nullptr) {
                        VAO *v = this->_bCache->Acquire();
                        v->Load(t->Chunk()); //Load chunk data into vao

                        if(! t->normTex->isActive())  {
                            t->normTex->Activate();
                            t->colTex->Activate();
                        }

                        t->vao = v;
                    }
                    // Set inUse boolean tag for the tile
                    t->inUse = true;
                    // Add to tempTiles vector
                    tempTiles.push_back(t);
                }
            }
        }
    }
    // Iterate through current tiles and release information for old tiles
    // Also reset the inUse tags to prevent future collisiosn
    for(auto it = this->tiles.begin(); it != this->tiles.end(); it++) {
        Tile *t = (*it);
        // If the tile is not in use, release it's textures and vao
        if(!t->inUse)    {
            // Release textures, if active
            if(t->normTex->isActive()) {
                t->normTex->Release();
                t->colTex->Release();
            }
            // Release the vao
            if(t->vao != nullptr)   {
                _bCache->Release(t->vao);
                t->vao = nullptr;
            }

            // Reset tDelt values
            // Reset parents tDelt (just in case it wasn't cleared previously)
            if(t->tDelt > 0)    {
                Tile *p =  &t->TileCell()->Tile(QTree::Parent(t->ID()));
                if(p->tDelt <= -1) {
                    p->tDelt = 0;
                }
                t->tDelt = 0;
            }
            // But don't reset it if the cell is being morphed to from it's children
            else if( ! (t->tDelt == -1 && t->Child(0)->tDelt > 0 && t->Child(0)->tDelt < 1))    {
                t->tDelt = 0;
            }
        }
        else    {
            t->inUse = false;
        }
    }
    // Swap contents of the temporary vector with the persistent view vector
    this->tiles.swap(tempTiles);
}

std::vector<Tile *> View::GetLODTiles(Tile *root) {
    std::vector<Tile *> out;
    // If it's the highest level of detail, return the tile's chunk
    if(root->NumChildren() == 0) {
        if (this->_stdVF->Intersect(root->BBox())) {
            out.push_back(root);
        }
    }
    else    {
        // compute screenspace error for chunk
        float d = root->BBox().distanceToPt(this->_cam.position());
        float ss_error = (this->_fbWid/(2*tan(this->_cam.fov()))) * (root->Chunk()._maxError/d);
        // If the screen-space error is within the threshold, return the chunk
        if(ss_error <= this->_errorLimit)  {
            if (this->_stdVF->Intersect(root->BBox())) {
                out.push_back(root);
            }
        }
        // Else recurse on the children of the tile
        // and concatenate their resutls in the out vector
        else    {
            for(int i=0; i<root->NumChildren(); i++) {
                std::vector<Tile *> result = GetLODTiles(root->Child(i));
                for(auto it = result.begin(); it != result.end(); it++) {
                    out.push_back(*it);
                }
            }
        }
    }
    return out;
}


std::vector<Tile *> View::GetAllTiles(Tile *root) {
    std::vector<Tile *> out;
    // If it's the highest level of detail, return the tile's chunk
    if(root->NumChildren() == 0) {
        if (this->_stdVF->Intersect(root->BBox())) {
            out.push_back(root);
        }
    }
    else {
            for(int i=0; i<root->NumChildren(); i++) {
                std::vector<Tile *> result = GetAllTiles(root->Child(i));
                for(auto it = result.begin(); it != result.end(); it++) {
                    out.push_back(*it);
                }
            }
        }
    return out;
}

void View::AssignTexturesHelp (class Cell *c, class Tile *t)  {
    if(t->LOD() < c->NormTQT()->Depth())  {
        t->normTex = _tCache->Make(
            c->NormTQT(),
            t->LOD(),
            (t->NWRow() *  (1 << t->LOD())) / (c->Width()),
            (t->NWCol() * (1 << t->LOD())) / (c->Width()));

        t->colTex = _tCache->Make(
            c->ColorTQT(),
            t->LOD(),
            (t->NWRow() *  (1 << t->LOD())) / (c->Width()),
            (t->NWCol() * (1 << t->LOD())) / (c->Width()));
    }
    // If the tile is deeper than the texture tree's depth
    else    {
        Tile *p = t;
        do  {
            p = &c->Tile(QTree::Parent(p->ID()));
        } while (p->LOD() >= c->NormTQT()->Depth());
        t->normTex = _tCache->Make(
            c->NormTQT(),
            p->LOD(),
            (p->NWRow() *  (1 << p->LOD())) / (c->Width()),
            (p->NWCol() * (1 << p->LOD())) / (c->Width()));

        t->colTex = _tCache->Make(
            c->ColorTQT(),
            p->LOD(),
            (p->NWRow() *  (1 << p->LOD())) / (c->Width()),
            (p->NWCol() * (1 << p->LOD())) / (c->Width()));
    }

    for(int i=0; i<t->NumChildren(); i++) {
        this->AssignTexturesHelp(c, t->Child(i));
    }
}





