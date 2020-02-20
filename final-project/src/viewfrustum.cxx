#include "viewfrustum.hxx"


void ViewFrustum::Update(class Camera cam) {
    cs237::vec3f p = d_to_f(cam.position());
    cs237::vec3f d = normalize(cam.direction());
    cs237::vec3f up = normalize(cam.up());

    cs237::vec3f right = cs237::vec3f(-1.0f,0.0f,0.0f);
    cs237::mat4x4f rot_m = cs237::rotateY(cam.phi());
    right = rotate_vec3f(right, rot_m);

    float near = cam.near();
    float far = cam.far();

    // these are halfs.

    float hnearw = tang * near;
    float hnearh = hnearw * cam.aspect();

    float hfarw = tang * far;
    float hfarh = hfarw * cam.aspect();

    cs237::vec3f nc = p + d * near;

    neartl = nc + (up * hnearh) - (right * hnearw);
    neartr = nc + (up * hnearh) + (right * hnearw);
    nearbl = nc - (up * hnearh) - (right * hnearw);
    nearbr = nc - (up * hnearh) + (right * hnearw);

    cs237::vec3f fc = p + d * far;

    fartl = fc + (up * hfarh) - (right * hfarw);
    fartr = fc + (up * hfarh) + (right * hfarw);
    farbl = fc - (up * hfarh) - (right * hfarw);
    farbr = fc - (up * hfarh) + (right * hfarw);


    left_n = cross(
            farbl - nearbl,
            neartl - nearbl
        );
    right_n = cross(
            fartr - neartr,
            nearbr - neartr
        );
    top_n = cross(
            fartl - neartl,
            neartr - neartl
        );
    bottom_n = cross(
            farbr - nearbr,
            nearbl - nearbr
        );

    near_n = d;
    far_n = -d;
}


bool ViewFrustum::Intersect(cs237::AABBd bbox) {

    int num_verts = 8;
    cs237::vec3f bbox_verts[8] = {
        cs237::vec3f(bbox.minX(), bbox.minY(), bbox.minZ()),
        cs237::vec3f(bbox.minX(), bbox.minY(), bbox.maxZ()),
        cs237::vec3f(bbox.minX(), bbox.maxY(), bbox.minZ()),
        cs237::vec3f(bbox.minX(), bbox.maxY(), bbox.maxZ()),
        cs237::vec3f(bbox.maxX(), bbox.minY(), bbox.minZ()),
        cs237::vec3f(bbox.maxX(), bbox.minY(), bbox.maxZ()),
        cs237::vec3f(bbox.maxX(), bbox.maxY(), bbox.minZ()),
        cs237::vec3f(bbox.maxX(), bbox.maxY(), bbox.maxZ())
    };

    int num_planes = 6;
    cs237::vec3f plane_norms[6] = {
        left_n, right_n, top_n, bottom_n,
        near_n, far_n
    };

    cs237::vec3f plane_cors[6] = {
        fartl, neartl, fartl, nearbl,
        neartl, fartr
    };

    cs237::vec3f p, n, r;
    float A, B, C, D, dist;

    bool intersected_plane;

    for (int j = 0; j < num_planes; j++) {
        intersected_plane = false;
        for (int k = 0; k < num_verts; k++) {
            p = plane_cors[j];
            n = plane_norms[j];
            r = bbox_verts[k];

            A = n[0];
            B = n[1];
            C = n[2];
            D = -dot(n, p);

            dist = dot(n, r) + D;

            if (dist >= 0) {
                intersected_plane = true;
                break;
            }
        }
    }

    return intersected_plane;
}

