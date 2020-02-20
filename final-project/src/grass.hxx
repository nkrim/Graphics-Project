#include "cs237.hxx"
#include "camera.hxx"
#include "map-cell.hxx"
#include "buffer-cache.hxx"
#include "texture-cache.hxx"
#include "view.hxx"

#ifndef GRASS_HXX
#define GRASS_HXX

#define NUM_GRASS 6000





static cs237::vec3f gquadVertsFront[4] = {
    cs237::vec3f( 1.0f,  0.0f,  0.0f) * 8.0f,
    cs237::vec3f(-1.0f,  0.0f,  0.0f) * 8.0f,
    cs237::vec3f(-1.0f,  1.0f,  0.0f) * 8.0f,
    cs237::vec3f( 1.0f,  1.0f,  0.0f) * 8.0f
};


static cs237::vec3f gquadVertsLeft[4] = {
    cs237::vec3f(-0.5f,  0.0f,  -0.866f) * 8.0f,
    cs237::vec3f( 0.5f,  0.0f,   0.866f) * 8.0f,
    cs237::vec3f( 0.5f,  1.0f,   0.866f) * 8.0f,
    cs237::vec3f(-0.5f,  1.0f,  -0.866f) * 8.0f
};



static cs237::vec3f gquadVertsRight[4] = {
    cs237::vec3f(-0.5f,  0.0f,   0.866f) * 8.0f,
    cs237::vec3f( 0.5f,  0.0f,  -0.866f) * 8.0f,
    cs237::vec3f( 0.5f,  1.0f,  -0.866f) * 8.0f,
    cs237::vec3f(-0.5f,  1.0f,   0.866f) * 8.0f
};


static cs237::vec2f randoms[NUM_GRASS];
static cs237::vec3f translations[NUM_GRASS];


static uint32_t gquadIndices[6] = {
    0,  2,  3, 0,  1,  2,
};

static cs237::vec2f gtextCoords[4] = {
    cs237::vec2f( 0.0f,  0.0f),
    cs237::vec2f( 1.0f,  0.0f),
    cs237::vec2f( 1.0f,  1.0f),
    cs237::vec2f( 0.0f,  1.0f)
};




// uh lets define the skybox coordinates here I guess
typedef struct GQuad_t {
  cs237::vec3f verts[4];
  uint32_t indices[6];
  cs237::vec2f text[4];

  void Copy(cs237::vec3f* verts,uint32_t* indices, cs237::vec2f* text);
  void Load();
  void Draw();
  void Init(View* v);
  void Update(View* v);
  void InitCoords(View* v);


  GLuint _id;
  GLuint  _vBuf;
  GLuint  _iBuf;
  GLuint  _tBuf;
  GLuint  _insBuf;
  GLuint  _nIndices;


  GLuint instanceVBO;


  cs237::texture2D* texture;

} GQuad;





typedef struct Grass_t {
  // left right top bottom near far
  GQuad_t sides[3];





  void Load();
  void Init(View* v);
  void Draw();
  void InitCoords(View* v);
  void Update(View* v);


} Grass;


#endif
