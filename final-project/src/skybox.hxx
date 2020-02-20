#include "cs237.hxx"
#include "camera.hxx"
#include "map-cell.hxx"
#include "buffer-cache.hxx"
#include "texture-cache.hxx"

static std::string NAMES[6] = {
    "xneg.png",
    "xpos.png",
    "ypos.png",
    "yneg.png",
    "zneg.png",
    "zpos.png"
};

static cs237::vec3f quadVertsTop[4] = {
    cs237::vec3f(-100.0f,  99.9f, -100.0f),
    cs237::vec3f( 100.0f,  99.9f, -100.0f),
    cs237::vec3f( 100.0f,  99.9f,  100.0f),
    cs237::vec3f(-100.0f,  99.9f,  100.0f)
};

static cs237::vec3f quadVertsBottom[4] = {
    cs237::vec3f(-100.0f, -99.9f,  100.0f),
    cs237::vec3f( 100.0f, -99.9f,  100.0f),
    cs237::vec3f( 100.0f, -99.9f, -100.0f),
    cs237::vec3f(-100.0f, -99.9f, -100.0f)
};

static cs237::vec3f quadVertsFar[4] = {
    cs237::vec3f(-100.0f, -100.0f,  -99.9f),
    cs237::vec3f( 100.0f, -100.0f,  -99.9f),
    cs237::vec3f( 100.0f,  100.0f,  -99.9f),
    cs237::vec3f(-100.0f,  100.0f,  -99.9f)
};

static cs237::vec3f quadVertsNear[4] = {
    cs237::vec3f( 100.0f, -100.0f,  99.9f),
    cs237::vec3f(-100.0f, -100.0f,  99.9f),
    cs237::vec3f(-100.0f,  100.0f,  99.9f),
    cs237::vec3f( 100.0f,  100.0f,  99.9f)
};

static cs237::vec3f quadVertsLeft[4] = {
    cs237::vec3f(-99.9f, -100.0f,  100.0f),
    cs237::vec3f(-99.9f, -100.0f, -100.0f),
    cs237::vec3f(-99.9f,  100.0f, -100.0f),
    cs237::vec3f(-99.9f,  100.0f,  100.0f)
};

static cs237::vec3f quadVertsRight[4] = {
    cs237::vec3f( 99.9f, -100.0f, -100.0f),
    cs237::vec3f( 99.9f, -100.0f,  100.0f),
    cs237::vec3f( 99.9f,  100.0f,  100.0f),
    cs237::vec3f( 99.9f,  100.0f, -100.0f)
};

static uint32_t quadIndices[6] = {
    0,  2,  3, 0,  1,  2,
};

static cs237::vec2f textCoords[4] = {
    cs237::vec2f( 0.0f,  0.0f),
    cs237::vec2f( 1.0f,  0.0f),
    cs237::vec2f( 1.0f,  1.0f),
    cs237::vec2f( 0.0f,  1.0f)
};


// uh lets define the skybox coordinates here I guess
typedef struct Quad_t {
  cs237::vec3f verts[4];
  uint32_t indices[6];
  cs237::vec2f text[4];


  void Copy(cs237::vec3f* verts,uint32_t* indices, cs237::vec2f* text);
  void Load();
  void Draw();
  void Init();

  GLuint _id;
  GLuint  _vBuf;
  GLuint  _iBuf;
  GLuint  _tBuf;
  GLuint  _nIndices;

  cs237::texture2D* texture;

} Quad;



typedef struct SkyBox_t {
  // left right top bottom near far
  struct Quad_t sides[6];

  cs237::vec3f delta;


  void Load();
  void Init();
  void Draw();
  void InitCoords(Camera cam);
} SkyBox;


