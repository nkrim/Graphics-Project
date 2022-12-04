GraphicsEngine
==============
Final Project for CS2377 @UChicago

Here is a sample rendering of test-map3, with grass and texture-mapping enabled but not rain.
![sample rendering of test-map3][sample]

Build and Run
-------------
Run `make` in `final-project/build`

To run on the intended map, run `./proj5 ../data/test-map3/` (in the same build directory)

(Only tested to work on MacOSX systems)


FINAL PROJECT - grp1411
=======================

Controls
--------
```
Feature toggle controls
  w: toggle wireframe
  l: toggle lighting
  f: toggle fog
  b: toggle rain
Movement controls
  u, h, j, l: movement (think wasd controls but shifted along the keyboard [u = w, etc])
  arrow keys: camera direction
  r         : movement speed increased while held
  space     : move upwards while held (works like thrust on a jetpack)
  v         : return to the ground
Miscellanious controls
  +: increase screen-space error tolerance
  -: decrease screen-space error tolerance
  q: quit
```

Features
-----------------------
* View Frustum
    - Only draws tiles that are at least partly visible by the view frustum.
    - Attempted optimizing collision algorithm
* Skybox
    - Cleared the 'edges' of the skybox by pushing in each side a tiny amount
    - Incorporates blending of the skybox into the horizon fog color
    - Looked hard for a good skybox texture online smile emoticon
* Terrain
    - Includes a 'detail' texture for rendering of very close triangles
* Water
    - Simple animation for 'waves' through constructive interference of two
      rotated 'wave crest' textures
    - Simple reflection of skybox and nearby terrain. I do this by drawing a
      copy of the entire map essentially upside-down, using the stencil-buffer
      to speed things up (and render alpha-values correctly).
* Rain
    - You can see rain
    - The fog becomes more prevalent with rain
* Fog
    - You can see fog and toggle it on and off.
* View Animation and Movement
    - Supports collisions ('walking on top of') the mesh
    - Supports physics (using the 'jetpack')
    - Supports smooth movement of camera
* Buffer-Cache
    - Made use of the cache-buffer through pointers in tiles and a vector of tiles to render
    - Optimized in tandem with recursive level-of-detail calculations and view frustum collision detection to reduce number of operations prior to rendering
* Morphing
    - Implemented height morphing during the vao creation stage
      > Includes the ability to morph backwards while already morphing
      > Will only skip the morphing phase if the camera goes fast enough to pass two LODs in the time of one morph
      > All achieved through interpretation of one float value in the Tile class between [-1, 1]
    - Diffuse textures morph with the tiles via linear interpolation between the child and parent textures
* Optimizations
    - Organizational and algorithmic optimizations performed across the board
      > Major areas include in the FindHeight function, rendering operation, and selecting tiles to be rendered

Comments / Issues
-----------------
- Texture and LOD morphing are intentionally exaggerated to display their effect clearly. This would be tuned down in a practical application.
- Wireframe mode may be slow due to GPU branching since one shader is used for all mesh rendering
- Large maps may be slowed by the fact that water reflections are rendered by rendering the mesh again, reversed, underneath itself
- Some maps with larger bodies of water will have some artifacts from the skirts of the reflected mesh poking through the water
- Details from the detail texture and the waves from the water textures may increase in size as the camera goes up (or as the LOD of the tile changes), and while attempts were made to fix this, it became too inconvenient considering how small of an effect it had

Credits
-------
Victor Jiao and Noah Krim


[sample]: https://github.com/nkrim/Graphics-Project/raw/master/sample_image.png "Sample Rendering"
