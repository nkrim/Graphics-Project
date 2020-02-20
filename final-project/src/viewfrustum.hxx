#include "cs237.hxx"
#include "camera.hxx"


typedef struct ViewFrustum_t {
  cs237::vec3f neartl;
  cs237::vec3f neartr;
  cs237::vec3f nearbl;
  cs237::vec3f nearbr;

  cs237::vec3f fartl;
  cs237::vec3f fartr;
  cs237::vec3f farbl;
  cs237::vec3f farbr;

  cs237::vec3f left_n;
  cs237::vec3f right_n;
  cs237::vec3f top_n;
  cs237::vec3f bottom_n;
  cs237::vec3f near_n;
  cs237::vec3f far_n;

  float tang;

  void Update(class Camera cam);

  bool Intersect(cs237::AABBd bbox);

} ViewFrustum;
