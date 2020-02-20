/*! \file view.hxx
 *
 * \author John Reppy
 */

/* CMSC23700 Final Project sample code (Autumn 2015)
 *
 * COPYRIGHT (c) 2015 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#ifndef _VIEW_HXX_
#define _VIEW_HXX_


#include "cs237.hxx"
#include "map.hxx"
#include "camera.hxx"
#include "buffer-cache.hxx"
#include "skybox.hxx"
#include "grass.hxx"
#include "rain.hxx"
#include "viewfrustum.hxx"
#include <vector>
#include <map>





cs237::ShaderProgram *LoadShader (std::string const & shaderPrefix);


std::vector<const struct Chunk> *GetChunks (class Tile *);

float sign (cs237::vec2f p1, cs237::vec2f p2, cs237::vec2f p3);
bool PointInTriangle (cs237::vec2f pt, cs237::vec2f v1, cs237::vec2f v2, cs237::vec2f v3);


bool PointInAABBxz(cs237::AABBd box, cs237::vec3d pos);
cs237::vec3f vec3fFromVertex(Vertex v);
cs237::vec2f vec3fxz(cs237::vec3f v);

class View {
  public:

    //! construct a viewer for the map
    View (Map *map);

    //! return the view's window
    GLFWwindow *Window() const { return this->_window; }

    //! initialize the view (i.e., allocate its window)
    void Init (int wid, int ht);

    //! method to handle display of the view
    void Render ();

    //! animation method
    void Animate ();

    //! handle kwyboard input
    void HandleKey (int key, int scancode, int action, int mods);

    //! handle mouse entering/leaving the view's window
    void HandleMouseEnter (bool entered);

    //! handle mouse movement
    void HandleMouseMove (double x, double y);

    //! handle mouse buttons
    //! \param button the mouse button that was pressed
    //! \param action was the button pressed or released?
    //! \param mods   the state of the modifier keys
    void HandleMouseButton (int button, int action, int mods);

    //! handle resizing the view
    void Resize (int wid, int ht);

    //! handle visibility change
    void SetVisible (bool isVis) { this->_isVis = isVis; }

    //! should this view close?
    bool shouldClose () const { return glfwWindowShouldClose(this->_window); }

    //! the view's current camera state
    class Camera const &Camera () const { return this->_cam; }

    //! the view's current error limit
    float ErrorLimit () const { return this->_errorLimit; }

    //! the cache of VAO objects for representing chunks
    class BufferCache *VAOCache () const { return this->_bCache; }

    //! the cache of OpenGL textures for the map tiles
    class TextureCache *TxtCache () const { return this->_tCache; }

    // vao/buffer-cache functions
    void CreateVAOs();
    void UpdateVAOs();
    std::vector<Tile *> GetLODTiles(Tile *root);
    std::vector<Tile *> GetAllTiles(Tile *root);

    void camTurnRight(float amount);
    void camTurnUp(float amount);
    void camMoveForwards(float amount);
    void camMoveUp(float amount);
    void camMoveRight(float amount);
    void camMoveInUp(float amount);

    float FindHeight();
    float FindHeightAt(cs237::vec3d pos);
    float FindHeightHelp(Tile *tile, cs237::vec3d pos);
    void DrawSkybox(bool isReflection);
    void DrawVAOs(bool isReflection);
    void DrawGrass();
    void DrawRain();

    // Assign the textures for each tile
    void AssignTextures(class Cell *c) {
      AssignTexturesHelp(c, &c->Tile(0));
    }

    void Jump() {_isJump = true;}
    void Land() {_isJump = false;}
    bool isJump() {return _isJump;}



    GLuint _texBuf;
    GLuint _texBufObj;



  private:
    Map		*_map;		//!< the map being rendered
    class Camera _cam;		//!< tracks viewer position, etc.
    float	_errorLimit;	//!< screen-space error limit
    bool	_isVis;		//!< true when this window is visible
    GLFWwindow	*_window;	//!< the main window
    int		_fbWid;		//!< current framebuffer width
    int		_fbHt;		//!< current framebuffer height
    cs237::AABBd _mapBBox;	//!< a bounding box around the entire map

    // resource management
    class BufferCache	*_bCache;	//! cache of OpenGL VAO objects used for chunks
    class TextureCache	*_tCache;	//! cache of OpenGL textures

    bool _isJump;

    /* ADDITIONAL STATE HERE */
    cs237::ShaderProgram *_shader;
    cs237::ShaderProgram *_boxshader;
    cs237::ShaderProgram *_grassshader;
    cs237::ShaderProgram *_rainshader;

    cs237::vec3f _translation;
    cs237::vec3f _scale;
    cs237::mat4x4f _scaleToWorld;

    cs237::color3f _fogColor;
    float _fogDensity;

    // locations of uniform variables
    // Standard/Miscellanious uniforms
    int _mvLoc;
    int _twLoc;
    int _tLoc;
    int _scalingLoc;
    int _timeLoc;

    int _projLoc;
    int _colLoc;
    int _fogDensityLoc;
    int _fogColorLoc;
    int _camLoc;

    // Directional light uniforms
    int _dirLoc;
    int _ambLoc;
    int _intLoc;

    // General terrain mesh texture uniforms
    int _texLoc;
    int _normLoc;

    // Water texture uniforms
    int _water1Loc;
    int _water2Loc;

    // Grass uniforms for the grass shader
    int _grassDLoc;
    int _gravelDLoc;

    // Tile attribute uniforms for texture coordinate calculations
    int _tileWidthLoc;
    int _cellWidthLoc;
    int _tileNWCLoc;
    int _tileNWRLoc;
    int _lodLoc;

    // Water attribute uniforms
    int _heightLoc;
    int _watermaskLoc;
    int _hasWaterLoc;
    int _rainTLoc;

    // Offsets for calculating texture coordinates for tiles of too high detail
    int _multOffsetLoc;
    int _addOffsetXLoc;
    int _addOffsetZLoc;

    // Boolean values for toggling certain features
    bool WIREFRAME;
    bool SHADOWING;
    bool FOG;
    bool GRASS;
    bool RAIN;
    // Uniforms for above toggling booleans
    int _wireframeLoc;
    int _shadowingLoc;
    int _fogLoc;
    int _reflectionLoc;
    int _rainLoc;

    // Sky box uniforms
    int _boxmvLoc;
    int _boxprojLoc;
    int _boxTexture;
    int _boxFogColorLoc;
    int _boxFaceLoc;
    int _boxFogLoc;
    int _boxDeltaLoc;
    int _boxAlphaLoc;
    int _boxRainLoc;
    int _boxRainTLoc;
    float _rain_t;

    // Grass uniforms
    int _gmvLoc;
    int _gprojLoc;
    int _gtextLoc;
    int _gtimeLoc;
    int _gtboLoc;
    int _gCamPosLoc;

    // Rain uniforms
    int _rmvLoc;
    int _rprojLoc;
    int _rtextLoc;
    int _rtimeLoc;
    int _rtboLoc;

    // Morphing uniforms
    int _tDeltLoc;
    int _pTexLoc;
    int _tileNWCwrtPLoc;
    int _tileNWRwrtPLoc;
    int _blendTexLoc;

    // Additional feature objects
    ViewFrustum* _stdVF;
    SkyBox* _skybox;
    Grass* _grass;
    Rain* _rain;

    cs237::vec3f _init_cam_pos;

    // Additional feature textures
    cs237::texture2D* _water1;
    cs237::texture2D* _water2;
    cs237::texture2D* _grassD;
    cs237::texture2D* _gravelD;

    // Vector of tiles that will be rendered in the frame
    // Updated before each frame
    std::vector<Tile *> tiles;

    // Helper function for the AssignTextures function
    void AssignTexturesHelp(class Cell *c, class Tile *t);
};

#endif // !_VIEW_HXX_
