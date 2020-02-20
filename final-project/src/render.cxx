/*! \file render.cxx
 *
 * \author John Reppy
 *
 * These are additional functions for the View, Cell, Tile, and Chunk classes for
 * rendering the mesh.
 */

/* CMSC23700 Final Project sample code (Autumn 2015)
 *
 * COPYRIGHT (c) 2015 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#include "cs237.hxx"
#include "view.hxx"
#include "camera.hxx"
#include "map-cell.hxx"
#include "buffer-cache.hxx"
#include "texture-cache.hxx"



//! Colors to use for rendering wireframes at different levels of detail
static cs237::color4ub MeshColor[8] = {
    cs237::color4ub{ 255, 255,   0, 255 },
    cs237::color4ub{ 255,   0, 255, 255 },
    cs237::color4ub{   0, 255, 255, 255 },
    cs237::color4ub{ 255, 128, 128, 255 },
    cs237::color4ub{ 128, 255, 128, 255 },
    cs237::color4ub{ 128, 128, 255, 255 },
    cs237::color4ub{ 255,   0,   0, 255 },
    cs237::color4ub{   0, 255,   0, 255 }
    };



void View::Render () {
    if (! this->_isVis)
	   return;

    _stdVF->Update(this->_cam);
    if (RAIN) {
        if (_rain_t < 0.991) {
            _rain_t += 0.01;
        }
        _rain->Update(this);
    } else if (_rain_t > 0.009) {
        _rain_t -= 0.01;
    }

    // printf("_rain_t:%f\n", _rain_t);

    if (GRASS)
        _grass->Update(this);
    _skybox->Load();

    // bind display framebuffer to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, this->_fbWid, this->_fbHt);

    // set OpenGL Clear Color
    glClearColor(0.2f, 0.2f, 0.4f, 1.0f);


    // draw skybox;
    this->_boxshader->Use();
        // clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        this->DrawSkybox(false);
    this->_shader->Use();


        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        // (for rendering over skybox)
        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        //Initialize stencil clear value
        glClearStencil( 2 );
        glEnable( GL_STENCIL_TEST );
        glStencilFunc( GL_ALWAYS, 0, 0xF );
        glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );

        this->DrawVAOs(false);

        // Code for procedurally generated grass
    // this->_grassshader->Use();
    //     glStencilFunc( GL_EQUAL, 2, 0xF);
    //     glDepthMask(GL_FALSE);
    //     glClear(GL_DEPTH_BUFFER_BIT);
    //     this->DrawGrass();
    //     glDepthMask(GL_TRUE);

this->_shader->Use();

    if (!WIREFRAME) {
    //
        // right now, we are using depth buffer to cull fragments.
        // i.e. pool is not rendered, therefore no depth
        // everything else is under the regular plane
        // so only the pool renders.
        // goal: fix so that everything renders correctly lol
        //  i.e. use stencil to cull fragments.
        glCullFace(GL_BACK);
        glStencilFunc(GL_ALWAYS, 2, 0xF); // why is this not Equals lollll

        glStencilOp( GL_KEEP, GL_KEEP, GL_INCR ); // all
        this->DrawVAOs(true);



    //
        // edit THIS display pass to be semi transparent.
        // idk how lol.
        this->_shader->Use();
        glStencilFunc(GL_LESS, 2, 0xF);
        glStencilOp( GL_KEEP, GL_INCR, GL_INCR ); // all

        // TODO: fix this, its necessary
        // glEnable(GL_DEPTH_TEST);
        // glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        this->DrawVAOs(true);

    //

        // edit skybox to make it only render over non-terrain water.
        // or rather, edit the stencil.
        glDisable(GL_CULL_FACE);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        glStencilFunc(GL_EQUAL, 3, 0xF);
        glDisable(GL_DEPTH_TEST);

        this->_boxshader->Use();


        this->DrawSkybox(true);

    }
    glEnable(GL_DEPTH_TEST);

    glDisable( GL_STENCIL_TEST );


    glDisable(GL_CULL_FACE);

    if (GRASS) {
    this->_grassshader->Use();
        this->DrawGrass();
    }

    if (RAIN) {
    this->_rainshader->Use();
        this->DrawRain();
    }
        glDisable(GL_BLEND);

    glfwSwapBuffers (this->_window);

}


void View::DrawSkybox(bool isReflection) {
    cs237::setUniform(this->_boxshader->UniformLocation("diffuseTexture"), 3);

    cs237::setUniform(this->_boxRainLoc, RAIN);
    cs237::setUniform(this->_boxRainTLoc, _rain_t);

    cs237::setUniform(this->_boxprojLoc, _cam.projTransform());
    if (!isReflection) {
        cs237::setUniform(this->_boxmvLoc, _cam.viewTransform());
        cs237::setUniform(this->_boxDeltaLoc, _skybox->delta);
        cs237::setUniform(this->_boxFogLoc, _map->hasFog() && FOG);
        cs237::setUniform(this->_boxAlphaLoc, 1.0f);
    } else {
        cs237::setUniform(this->_boxmvLoc, _cam.viewTransform() * cs237::scale(cs237::vec3f(1,-1,1)));
        cs237::setUniform(this->_boxDeltaLoc, _skybox->delta * cs237::vec3f(1,-1.0f ,1) - cs237::vec3f(0, 2 * _init_cam_pos[1], 0));
        cs237::setUniform(this->_boxFogLoc, false);
        cs237::setUniform(this->_boxAlphaLoc, 0.3f);

    }
    cs237::setUniform(this->_boxFogColorLoc, this->_fogColor);



    CS237_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));



    for (int i = 0; i < 6; i++) {
        cs237::setUniform(this->_boxFaceLoc, i);
        _skybox->sides[i].Draw();
    }
}

void View::DrawVAOs(bool isReflection) {
    float asdf = glfwGetTime();
    cs237::setUniform(this->_timeLoc, asdf);

    cs237::setUniform(this->_rainLoc, RAIN);
    cs237::setUniform(this->_rainTLoc, _rain_t);

    cs237::setUniform(this->_hasWaterLoc, _map->hasWaterMask());

    cs237::setUniform(this->_water1Loc, 5);
    CS237_CHECK(glActiveTexture(GL_TEXTURE5));
    _water1->Bind();

    cs237::setUniform(this->_water2Loc, 6);
    CS237_CHECK(glActiveTexture(GL_TEXTURE6));
    _water2->Bind();

    cs237::setUniform(this->_grassDLoc, 10);
    CS237_CHECK(glActiveTexture(GL_TEXTURE10));
    _grassD->Bind();

    cs237::setUniform(this->_gravelDLoc, 11);
    CS237_CHECK(glActiveTexture(GL_TEXTURE11));
    _gravelD->Bind();

    cs237::setUniform(this->_heightLoc, 7);
    cs237::setUniform(this->_watermaskLoc, 8);

    cs237::setUniform(this->_projLoc, _cam.projTransform());
    cs237::setUniform(this->_mvLoc, _cam.originViewTransform());

    // remember to check the water level
    if (!isReflection)
        cs237::setUniform(this->_scalingLoc, cs237::vec4f(_scale, 1.0f));
    else
        cs237::setUniform(this->_scalingLoc, cs237::vec4f(_scale[0], -_scale[1], _scale[2], 1.0f));

    cs237::setUniform(this->_reflectionLoc, isReflection);

    cs237::setUniform(this->_camLoc, d_to_f(_cam.position()));

    // note light direction and stats COULD CHANGE.
    // so keep this here rather than in ShaderInit
    cs237::setUniform(this->_dirLoc, _map->SunDirection());
    cs237::setUniform(this->_intLoc, _map->SunIntensity());
    cs237::setUniform(this->_ambLoc, _map->AmbientIntensity());

    cs237::setUniform(this->_wireframeLoc, this->WIREFRAME);
    cs237::setUniform(this->_shadowingLoc, this->SHADOWING);
    cs237::setUniform(this->_fogLoc, _map->hasFog() && this->FOG);
    cs237::setUniform(this->_fogDensityLoc, this->_fogDensity);
    cs237::setUniform(this->_fogColorLoc, this->_fogColor);

    if (this->WIREFRAME)
        CS237_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    else
        CS237_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

    // render
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xffff);

    // Vector for holding parent tiles whose textures have been activated, so taht they can be released sat the end of the render loop
    // Since it is guarenteed that if 1 tile uses a parent texture than the other children of that tile will also use it, it number of activating / releasing operations
    std::vector<Tile *> parentsUsed;

    // Draw tiles via their vao pointers
    for(auto it = this->tiles.begin(); it != this->tiles.end(); it++) {
        Tile *t = (*it);

        // if (!_stdVF->Intersect(t->BBox())) {
        //     continue;
        // }

        cs237::setUniform(this->_tileWidthLoc, (float) t->Width());
        cs237::setUniform(this->_cellWidthLoc, (float) t->TileCell()->Width());
        cs237::setUniform(this->_tileNWCLoc, (float) t->NWCol());
        cs237::setUniform(this->_tileNWRLoc, (float) t->NWRow());

        cs237::setUniform(this->_lodLoc, t->TileCell()->Depth() - t->LOD());

        // If the tile has it's own designated texture, use identities
        if(t->LOD() < t->TileCell()->NormTQT()->Depth())    {
            cs237::setUniform(this->_multOffsetLoc, 1.0f);
            cs237::setUniform(this->_addOffsetXLoc, 0.0f);
            cs237::setUniform(this->_addOffsetZLoc, 0.0f);
        }
        else    {
            float multOff = 1.0f / (1 << ((t->LOD() - t->TileCell()->NormTQT()->Depth()) + 1));
            cs237::setUniform(this->_multOffsetLoc, multOff);
            // Get tile from which this tile will get the texture
            Tile *p = t;
            do  {
                p = &t->TileCell()->Tile(QTree::Parent(p->ID()));
            } while (p->LOD() >= t->TileCell()->NormTQT()->Depth());
            float addOffX = multOff * ((t->NWCol() - p->NWCol()) / t->Width());
            float addOffZ = multOff * ((t->NWRow() - p->NWRow()) / t->Width());
            cs237::setUniform(this->_addOffsetXLoc, addOffX);
            cs237::setUniform(this->_addOffsetZLoc, addOffZ);
        }

        // CURRENTLY COLOR DEPENDS ON THE LOD OF THE TILE
        cs237::setUniform(this->_colLoc, MeshColor[t->LOD()]);

        t->normTex->Use(1);
        cs237::setUniform(this->_normLoc, 1);

        t->colTex->Use(0);
        cs237::setUniform(this->_texLoc, 0);

        // MORPHING
        cs237::setUniform(this->_tDeltLoc, t->tDelt);
          // If morphing, activate and use parent's color texture
        if(t->tDelt != 1 && t->LOD() < t->TileCell()->NormTQT()->Depth())   {
            Tile *p = &t->TileCell()->Tile(QTree::Parent(t->ID()));
            if(!p->colTex->isActive())  {
                p->colTex->Activate();
            }

            p->colTex->Use(17);
            cs237::setUniform(this->_pTexLoc, 17);

            // Set tile rows and columns with respect to it's parent for
            int ci = QTree::ChildIndex(t->ID());
            cs237::setUniform(this->_tileNWCwrtPLoc, (int)((ci & 1) != ((ci & 2) >> 1)));
            cs237::setUniform(this->_tileNWRwrtPLoc, (ci & 2) >> 1);

            parentsUsed.push_back(p);

            cs237::setUniform(this->_blendTexLoc, GL_TRUE);
        }
        else    {
            cs237::setUniform(this->_blendTexLoc, GL_FALSE);
        }

        // Additional textures
        if (this->_map->hasNormalMap()) {
            CS237_CHECK(glActiveTexture(GL_TEXTURE7));
            t->TileCell()->_height->Bind();
        }

        if (this->_map->hasWaterMask()) {
            CS237_CHECK(glActiveTexture(GL_TEXTURE8));
            t->TileCell()->_water->Bind();
        }

        cs237::setUniform(this->_twLoc, cs237::vec3f(t->TileCell()->Width() * t->TileCell()->Col(), 0.0f, t->TileCell()->Width() * t->TileCell()->Row()));

        // TODO implement morphing after Noah fixes VAO's
        cs237::setUniform(this->_tLoc, 0.0f);

        //Draw vao
        t->vao->Render();
    }

    // Release parent textures that were used
    for(auto it = parentsUsed.begin(); it != parentsUsed.end(); it++)   {
        Tile *p = (*it);
        if(p->colTex->isActive())  {
            p->colTex->Release();
        }
    }

    glDisable(GL_PRIMITIVE_RESTART);
}


void View::DrawGrass() {
    float asdf = glfwGetTime();
    cs237::setUniform(this->_gtimeLoc, asdf);

    cs237::setUniform(this->_gtextLoc, 3);

    cs237::setUniform(this->_gprojLoc, _cam.projTransform());
    cs237::setUniform(this->_gmvLoc, _cam.viewTransform());
    cs237::setUniform(this->_gCamPosLoc, d_to_f(_cam.position()));

    glActiveTexture(GL_TEXTURE16);
    glUniform1i(this->_gtboLoc, 16);


    // cs237::setUniform(this->_boxFogColorLoc, this->_fogColor);


    CS237_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

    _grass->Draw();
}

void View::DrawRain() {

    float asdf = glfwGetTime();
    cs237::setUniform(this->_rtimeLoc, asdf);

    cs237::setUniform(this->_rtextLoc, 3);

    cs237::setUniform(this->_rprojLoc, _cam.projTransform());
    cs237::setUniform(this->_rmvLoc, _cam.viewTransform());
    // cs237::setUniform(this->_boxFogColorLoc, this->_fogColor);



    glActiveTexture(GL_TEXTURE15);
    glUniform1i(this->_rtboLoc, 15);

    CS237_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_POINT));

    _rain->Draw();
}
