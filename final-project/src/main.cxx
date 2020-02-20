/*! \file main.cxx
 *
 * \author John Reppy
 */

/* CMSC23700 Final Project sample code (Autumn 2015)
 *
 * COPYRIGHT (c) 2015 John Reppy (http://cs.uchicago.edu/~jhr)
 * All rights reserved.
 */

#include "cs237.hxx"
#include "map.hxx"
#include "map-cell.hxx"
#include "view.hxx"
#include <unistd.h>
#include <map>





//! parameters for the view controls
// #define ROTATE_ANGLE    1.0f        //!< angle of rotation in degrees
// #define ZOOM_DIST       0.1f        //! distance to move camera

static float uspf = 1000000.0f / 60.0f;
double  _lastFrameTime; //!< time of last frame
double  _lastStep;  //!< time of last animation step

static float totalFPS = 0.0;
static int numF = 0;

bool printFrame = false;
bool readyFrame = true;


// keys
std::map<int,bool> keys_pressed;
void init_key_map();
bool is_key_down(int key);
void press_key(int key);
void release_key(int key);

// velocities
float calculate_cam_speed(float value, bool is_angle, bool is_activated);

static float v_move_forwards = 0;
static float v_move_backwards = 0;
static float v_turn_right = 0;
static float v_turn_left = 0;
static float v_turn_up = 0;
static float v_turn_down = 0; // for what
static float v_move_left = 0;
static float v_move_right = 0;
static float v_move_up = 0;
static float v_move_down = 0;

static float jump_a = -0.5f;
static float jump_v = 0;

static float a_linear = 0.8f * 0.3f;
static float a_angular = 0.1f;
static float m_linear = 8.0f * 0.3f;
static float m_angular = 1.0f;

static float mult = 1.0f;


/* Next set of four functions are regarding storing whether a key
 * is pressed, etc.
 */

bool is_key_down(int key) {
    return keys_pressed[key];
}

void press_key(int key) {
    keys_pressed[key] = true;
}

void release_key(int key) {
    keys_pressed[key] = false;
}


/* calculates the new velocity of the camera
 * based on predefined accelerations and max velocities.
 *
 * value:           the old value for velocity
 * is_angle:        is the value for an angular velocity
 * is_activated:    is the key pressed for this velocity, or not
 */
float calculate_cam_speed(float value, bool is_angle, bool is_activated) {
    float a;
    float m;

    if (is_key_down(GLFW_KEY_R)) {
        if (mult < 10.0f) {
            mult += 0.02f;
        }
    } else if (mult > 1.0f) {
        if (mult > 1.6f) {

            mult -= 0.02f;
        } else {
            mult = 1.0f;
        }
    }

    if (is_angle) {
        a = a_angular;
        m = m_angular;
    } else {
        a = a_linear * mult;
        m = m_linear * mult;
    }

    if(is_activated) {
        if (value < m)
            value += a;
        else
            value = m;
    } else {
        if (value > a)
            value -= a;
        else
            value = 0;
    }

    return value;
}

/* actually goes and updates camera positions and velocities
 * based on data from other functions and keys, etc.
 */
void update(GLFWwindow *win) {
    View *view = (View *)glfwGetWindowUserPointer(win);

    // physics and stuff
    double now = glfwGetTime();
    float dt = float(now - _lastFrameTime);
    _lastFrameTime = now;




    v_move_forwards  = calculate_cam_speed(v_move_forwards, false, is_key_down(GLFW_KEY_U));
    v_move_backwards = calculate_cam_speed(v_move_backwards, false, is_key_down(GLFW_KEY_J));
    v_move_left      = calculate_cam_speed(v_move_left, false, is_key_down(GLFW_KEY_H));
    v_move_right     = calculate_cam_speed(v_move_right, false, is_key_down(GLFW_KEY_K));

    v_move_up        = calculate_cam_speed(v_move_up, false, is_key_down(GLFW_KEY_ENTER));
    v_move_down      = calculate_cam_speed(v_move_down, false, is_key_down(GLFW_KEY_RIGHT_SHIFT));

    v_turn_up        = calculate_cam_speed(v_turn_up, true, is_key_down(GLFW_KEY_UP));
    v_turn_down      = calculate_cam_speed(v_turn_down, true, is_key_down(GLFW_KEY_DOWN));
    v_turn_left      = calculate_cam_speed(v_turn_left, true, is_key_down(GLFW_KEY_LEFT));
    v_turn_right     = calculate_cam_speed(v_turn_right, true, is_key_down(GLFW_KEY_RIGHT));

    // update physics;


    if (is_key_down(GLFW_KEY_SPACE)) {
        if (view->isJump()) {
            jump_v += 1.0f;
        } else {
            jump_v = 4.0f;
            view->Jump();
        }
    }
    if (view->isJump()) {
        jump_v += jump_a;

        view->camMoveUp(jump_v);
    }



    if (is_key_down(GLFW_KEY_V)) {
        view->Land();
        jump_v = 0.0;
    }

    bool to_update = false;

    if(v_move_forwards - v_move_backwards != 0) {
        view->camMoveForwards(dt * 60 * (v_move_forwards - v_move_backwards));
        to_update = true;
    }
    if(v_turn_up - v_turn_down != 0) {
        view->camTurnUp(dt * 60 * (v_turn_up - v_turn_down));
        to_update = true;
    }
    if(v_turn_right - v_turn_left != 0) {
        view->camTurnRight(dt * 60 * (v_turn_right - v_turn_left));
        to_update = true;
    }
    if(v_move_right - v_move_left != 0) {
        view->camMoveRight(dt * 60 * (v_move_right - v_move_left));
        to_update = true;
    }
    if(v_move_up - v_move_down != 0) {
        view->camMoveUp(dt * 60 * (v_move_up - v_move_down));
        to_update = true;
    }


    if (to_update) {
        // view->needsRedraw = true;
        // view->InitViewMatrix();
    }

}









/***** callbacks *****
 *
 * These callback functions are wrappers for the methods of the View class
 */

/*! \brief Run the simulation and then redraw the animation.
 */
void Display (GLFWwindow *win) {
    reinterpret_cast<View *>(glfwGetWindowUserPointer(win))->Render();

} /* end of Display */

/*! \brief Window resize callback.
 *  \param wid the new width of the window.
 *  \param ht the new height of the window.
 */
void Reshape (GLFWwindow *win, int wid, int ht) {
    reinterpret_cast<View *>(glfwGetWindowUserPointer(win))->Resize (wid, ht);

} /* end of Reshape */

/*! \brief Keyboard-event callback.
 *  \param win the window receiving the event
 *  \param key The keyboard code of the key
 *  \param scancode The system-specific scancode of the key.
 *  \param action GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
 *  \param mods the state of the keyboard modifier keys.
 */
void Key (GLFWwindow *win, int key, int scancode, int action, int mods) {
    if (action == GLFW_RELEASE) {
        release_key(key);
    } else {
        press_key(key);
        reinterpret_cast<View *>(glfwGetWindowUserPointer(win))->HandleKey (key, scancode, action, mods);
    }

} /* Key */

/*! \brief The mouse enter/leave callback function
 */
void MouseEnter (GLFWwindow *win, int entered) {
    reinterpret_cast<View *>(glfwGetWindowUserPointer(win))->HandleMouseEnter(entered != 0);

}

/*! \brief The mouse motion callback function
 */
void MouseMotion (GLFWwindow *win, double xPos, double yPos) {
    reinterpret_cast<View *>(glfwGetWindowUserPointer(win))->HandleMouseMove(xPos, yPos);

}

/*! \brief The mouse button callback function
 */
void MouseButton (GLFWwindow *win, int button, int action, int mods) {
    reinterpret_cast<View *>(glfwGetWindowUserPointer(win))->HandleMouseButton(button, action, mods);

}

/*! \brief Visibility-change callback: sets the visibility state of the view.
 *  \param state the current state of the window; GL_TRUE for iconified, GL_FALSE otherwise.
 */
void Visible (GLFWwindow *win, int state) {
    reinterpret_cast<View *>(glfwGetWindowUserPointer(win))->SetVisible (state == GL_TRUE);

} /* end of Visible */

//! main entrypoint
//
int main (int argc, const char **argv) {
    Map map;
    printf("\n");

  // get the mapfile
    if (argc < 2) {
    	std::cerr << "usage: proj5 <map-dir>" << std::endl;
    	return 1;
    }
    std::string mapDir(argv[1]);
    std::clog << "loading " << mapDir << std::endl;
    if (! map.LoadMap (mapDir, false)) {
	   return 1;
    }

    std::clog << "loading cells" << std::endl;
    for (int r = 0;  r < map.nRows(); r++) {
    	for (int c = 0;  c < map.nCols();  c++) {
    	    map.Cell(r, c)->Load();
    	}
    }

    std::clog << "initializing view" << std::endl;
    View *view = new View (&map);
    view->Init (1024, 768);


    printf("\n");
    printf("Controls: \n");
    printf("Movement  U\n");
    printf("         HJK\n");
    printf("\n");
    printf("Rotation  ^ \n");
    printf("         <v>\n");
    printf("\n");
    printf("Fly Up         SPACE \n");
    printf("Speed Up       R\n");
    printf("Reset Height   F\n");
    printf("Freeze Height  V\n");
    printf("\n");
    printf("Float Up: Right Enter\n");
    printf("Float Down: Right Shift\n");
    printf("\n");
    printf("w: toggle wireframe\n");
    printf("l: toggle lighting\n");
    printf("f: toggle fog\n");
    printf("g: toggle grass\n");
    printf("b: toggle rain\n");
    printf("+: increase screen-space error tolerance\n");
    printf("-: decrease screen-space error tolerance\n");
    printf("q: quit\n");



    printf("\n");





  // initialize the callback functions
    glfwSetWindowRefreshCallback (view->Window(), Display);
    glfwSetWindowSizeCallback (view->Window(), Reshape);
    glfwSetWindowIconifyCallback (view->Window(), Visible);
    glfwSetKeyCallback (view->Window(), Key);
    glfwSetCursorEnterCallback (view->Window(), MouseEnter);
    glfwSetCursorPosCallback (view->Window(), MouseMotion);
    glfwSetMouseButtonCallback (view->Window(), MouseButton);

    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_MULTISAMPLE);
    glCullFace(GL_BACK);
    glEnable(GL_PROGRAM_POINT_SIZE);



    _lastStep = _lastFrameTime = glfwGetTime();

    while (! view->shouldClose()) {

        double pre = glfwGetTime();
        update(view->Window());

        view->UpdateVAOs();
        view->Render ();
        double elapsed = (glfwGetTime() - pre);
    	// view->Animate ();

        if ( (((int) pre) % 2) ) {
            printFrame = true;
        } else {
            readyFrame = true;
        }

        numF++;
        totalFPS += elapsed;
        if (printFrame && readyFrame) {
            printf("fps: %f\n", 1 / (totalFPS / numF));
            printFrame = false;
            readyFrame = false;
            numF = 0;
            totalFPS = 0;
        }
        double t = uspf - 1000 * 1000 * elapsed;
        t = t > 0 ? t : 0;
        usleep(t);
    	glfwPollEvents ();
        // glfwWaitEvents();
    }

    glfwTerminate ();

    return EXIT_SUCCESS;

}
