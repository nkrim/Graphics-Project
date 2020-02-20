#include "cs237.hxx"
#include "camera.hxx"
#include "map-cell.hxx"
#include "buffer-cache.hxx"
#include "texture-cache.hxx"
#include "view.hxx"

#define NUM_DROPS 10000


#ifndef RAIN_HXX
#define RAIN_HXX



typedef struct Rain_t {


  cs237::vec3f verts[NUM_DROPS];
  uint32_t indices[NUM_DROPS];

  float time_shifts[NUM_DROPS];


  void Load();
  void Init(View* v);
  void Draw();
  void InitCoords(View* v);

  void Update(View* v);





  GLuint _id;
  GLuint  _vBuf;
  GLuint  _iBuf;
  GLuint  _nIndices;


  cs237::texture2D* texture;


} Rain;



#endif