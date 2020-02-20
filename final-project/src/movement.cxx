
#include "cs237.hxx"
#include "view.hxx"
#include "camera.hxx"


// TODO: note that backface culling removes the effect of skirts
void View::camTurnRight(float amount) {
    _cam.TurnRight(amount);
}

void View::camTurnUp(float amount) {
    _cam.TurnUp(amount);
}

void View::camMoveForwards(float amount) {
    cs237::vec3f delta = _cam.MoveForwards(amount);

    float fh = FindHeight();
    if (_isJump) {
        if (fh > 0.0f && fh > _cam.position()[1])
            Land();

    } else if (fh > 0.0f)
        delta += _cam.MoveUp(- _cam.position()[1] + FindHeight());



    _skybox->delta += delta;
}

void View::camMoveUp(float amount) {
    cs237::vec3f delta = _cam.MoveUp(amount);

    // corrects height
    float fh = FindHeight();
    if (_isJump) {
        if (fh > 0.0f && fh > _cam.position()[1])
            Land();

    } else if (fh > 0.0f)
        delta += _cam.MoveUp(- _cam.position()[1] + FindHeight());

    _skybox->delta += delta;
}

void View::camMoveRight(float amount) {
    cs237::vec3f delta = _cam.MoveRight(amount);

    float fh = FindHeight();
    if (_isJump) {
        if (fh > 0.0f && fh > _cam.position()[1])
            Land();

    } else if (fh > 0.0f)
        delta += _cam.MoveUp(- _cam.position()[1] + FindHeight());



    _skybox->delta += delta;
}


void View::camMoveInUp(float amount) {
    cs237::vec3f delta = _cam.MoveInUp(amount);

    // corrects height
    float fh = FindHeight();
    if (_isJump) {
        if (fh > 0.0f && fh > _cam.position()[1])
            Land();

    } else if (fh > 0.0f)
        delta += _cam.MoveUp(- _cam.position()[1] + FindHeight());

    _skybox->delta += delta;
}









// rotates the user's viewpoint right
void Camera::TurnRight(float amount) {
    _phi += amount;

    cs237::mat4x4f rot_m = cs237::rotateY(amount);
    _dir = rotate_vec3f(_dir, rot_m);
    _up = rotate_vec3f(_up, rot_m);

    if (_phi > 360) {
        _phi -= 360;
    } else if (_phi < -360) {
        _phi += 360;
    }
}

// rotates the user's viewpoint up
/* Note that this is more difficult than TurnRight
 * since the rotation axis changes. However, it changes
 * with _phi, so we can use that angle as the rotation axis.
 */
void Camera::TurnUp(float amount) {


    // ## create rotation axis from _phi. ##
    cs237::vec3f axis = cs237::vec3f(1.0f,0.0f, 0.0f);
    cs237::mat4x4f rot_m = cs237::rotateY(_phi);
    axis = rotate_vec3f(axis, rot_m);

    // ## rotate the camAt vector ##
    rot_m = cs237::rotate(-amount, axis);
    _dir = rotate_vec3f(_dir, rot_m);

    _up = rotate_vec3f(_up, rot_m);


}

// moves user forward
cs237::vec3f Camera::MoveForwards(float amount) {


    cs237::vec3f forwards = cs237::vec3f(0.0f,0.0f,1.0f);

    cs237::mat4x4f rot_m = cs237::rotateY(_phi);

    forwards = rotate_vec3f(forwards, rot_m) * amount;


    _pos += f_to_d(forwards);
    return forwards;
}

cs237::vec3f Camera::MoveUp(float amount) {
    cs237::vec3f newPos = d_to_f(_pos);
    newPos[1] += amount;
    _pos[1] += amount;
    return cs237::vec3f(0.0f, amount, 0.0f);
}


// moves user right
cs237::vec3f Camera::MoveRight(float amount) {
    cs237::vec3f right = cs237::vec3f(-1.0f,0.0f,0.0f);

    cs237::mat4x4f rot_m = cs237::rotateY(_phi);

    right = rotate_vec3f(right, rot_m);

    cs237::vec3f amt = right * amount;
    _pos += f_to_d(amt);
    return amt;
}

cs237::vec3f Camera::MoveInUp(float amount) {
    cs237::vec3f newPos = d_to_f(_pos);
    cs237::vec3f amt = amount * this->_up;
    newPos += amount * this->_up;
    _pos = f_to_d(newPos);

    return amt;
}


// Checks to see if the new position is a 'valid movement'.
// Only move the camera if the new position is valid.
void Camera::enforceValidMovement(cs237::vec3f newPos) {
    // float currentDist = length(_pos);
    // float newDist = length(newPos);

    // if (newDist > FAR_LIM && newDist > currentDist) {
        // return false;
    // }
    // return true;
}


