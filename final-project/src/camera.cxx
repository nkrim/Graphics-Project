 /*! \file camera.cxx
 *
 * \author John Reppy
 *
 * The camera class encapsulates the current view and projection matrices.
 */

/* CMSC23700 Final Project sample code (Autumn 2015)
 *
 * COPYRIGHT (c) 2015 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#include "cs237.hxx"
#include "camera.hxx"



cs237::vec3f rotate_vec3f(cs237::vec3f vector, cs237::mat4x4f matrix) {

    // put in mat4x4f for easy transformations
    cs237::mat4x4f* vec_m = new cs237::mat4x4f(cs237::vec4f(vector,1), cs237::vec4f(), cs237::vec4f(), cs237::vec4f());
    // rotate axis_m
    *vec_m = matrix * *vec_m;
    // retrive axis vector from axis_m;

    cs237::vec3f ans = cs237::vec3f((*vec_m)[0]);

    free(vec_m);

    return ans;
}


cs237::vec3f d_to_f(cs237::vec3d vec) {
    return cs237::vec3f((float)vec[0],
                        (float)vec[1],
                        (float)vec[2]);
}

cs237::vec3d f_to_d(cs237::vec3f vec) {
    return cs237::vec3d((double)vec[0],
                        (double)vec[1],
                        (double)vec[2]);
}




/***** class Camera member functions *****/

Camera::Camera ()
    : _errorFactor(-1){


}

void Camera::set_height(float height) {
    if (height > 0) {
        this->_pos[1] = height;
        //printf("%f\n", this->_pos[1]);
    }
}

void Camera::setPhi() {
        // calculate phi.
    // TODO: fix
    cs237::vec3f lookAtVec = _dir;

    cs237::vec2f lookAtXZ = cs237::vec2f(lookAtVec[0], lookAtVec[2]);
    cs237::vec2f viewVec = cs237::vec2f(0.0f,1.0f);
    double cos_theta = dot(lookAtXZ,viewVec) / lookAtXZ.length();

    _phi = cs237::degrees(acos(cos_theta));

    if (lookAtXZ[0] > 0) {
        _phi *= -1;
    }

    while (this->_phi > 360.0f) {
        this->_phi -= 360.0f;
    }
    while (this->_phi < -360.0f) {
        this->_phi += 360.0f;
    }


}

// translate a point to the coordinate system that has the camera as the origin, but is
// oriented and scaled the same as the world coordinates.
cs237::vec3d Camera::translate (cs237::vec3d const &p) const {
    return p - this->_pos;
}

// a transformation matrix that assumes that the camera is at the origin.
cs237::mat4x4f Camera::originViewTransform () const {
    return cs237::lookAt (cs237::vec3f(0.0f, 0.0f, 0.0f),
                    	  this->_dir,
                    	  this->_up);
}

// a transformation matrix that assumes that the camera is _pos, which it is.
cs237::mat4x4f Camera::viewTransform () const {
    return cs237::lookAt (d_to_f(this->_pos),
                          this->_dir + d_to_f(this->_pos),
                          this->_up);
}

// the projection transform for the camera
cs237::mat4x4f Camera::projTransform () const {
    float n_e = this->_nearZ * tanf (this->_halfFOV);  // n/e
    return cs237::frustum(-n_e, n_e,
                    	  -this->_aspect * n_e, this->_aspect * n_e,
                    	  this->_nearZ, this->_farZ);
}

// update the camera for the aspect ratio of the given viewport.  This operations will change
// the aspect ratio, but not the field of view.
void Camera::setViewport (int wid, int ht) {
    this->_errorFactor = -1.0f;  // mark the error factor as invalid
    this->_aspect = float(ht) / float(wid);
    this->_wid = wid;
}

// set the horizontal field of view in degrees
void Camera::setFOV (float angle) {
    this->_errorFactor = -1.0f;
    this->_halfFOV = cs237::radians(0.5 * angle);
}

// set the near and far planes
void Camera::setNearFar (double nearZ, double farZ) {
    assert ((0.0 < nearZ) && (nearZ < farZ));
    this->_nearZ = nearZ;
    this->_farZ = farZ;
}

// move the camera to a new position while maintaining its heading
void Camera::move (cs237::vec3d const &pos) {
    this->_pos = pos;
}

// move the camera to a new position and heading, while maintaining
// its up vector
void Camera::move (cs237::vec3d const &pos, cs237::vec3d const &at) {
    this->_pos = pos;
    this->_dir = normalize(cs237::toFloat(at - pos));
}

// move the camera to a new position, heading, and up vector
void Camera::move (cs237::vec3d const &pos, cs237::vec3d const &at, cs237::vec3d const &up) {
    this->_pos = pos;
    this->_dir = cs237::toFloat(normalize(at - pos));
    this->_up = cs237::toFloat(normalize(up));
}

// change the direction of the camera
void Camera::look (cs237::vec3f const &dir) {
    this->_dir = normalize(dir);
}

// change the direction of the camera
void Camera::look (cs237::vec3f const &dir, cs237::vec3f const &up) {
    this->_dir = normalize(dir);
    this->_up = up;
}

// compute the screen-space error for a geometric error of size err at distance dist.
float Camera::screenError (float dist, float err) const {
    if (this->_errorFactor < 0.0f) {
	   this->_errorFactor = float(this->_wid) / (2.0 * tanf(this->_halfFOV));
    }
    return this->_errorFactor * (err / dist);

}

/***** Output *****/

std::ostream& operator<< (std::ostream& s, Camera const &cam) {
    s << "Camera {"
	<< "\n  position =  " << cam.position()
	<< "\n  direction = " << cam.direction()
	<< "\n  up =        " << cam.up()
	<< "\n  near Z =    " << cam.near()
	<< "\n  far Z =     " << cam.far()
	<< "\n  aspect =    " << cam.aspect()
    << "\n  fov =       " << cam.fov()
    << "\n  phi =       " << cam.phi()
	<< "\n}";

    return s;
}









