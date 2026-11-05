#include <GLFW/glfw3.h>

#include "glfw_camera_handler.h"

bool GLFWCameraHandler(CameraMovement &movement, int key, int action, int mods)
{
    const bool pressed = action != GLFW_RELEASE;
    bool *state = nullptr;
    switch (key)
    {
    case GLFW_KEY_W: state = &movement.forward; break;
    case GLFW_KEY_S: state = &movement.backward; break;
    case GLFW_KEY_A: state = &movement.strafeLeft; break;
    case GLFW_KEY_D: state = &movement.strafeRight; break;
    case GLFW_KEY_E: state = &movement.up; break;
    case GLFW_KEY_Q: state = &movement.down; break;
    case GLFW_KEY_LEFT_SHIFT:
    case GLFW_KEY_RIGHT_SHIFT: state = &movement.fastSpeed; break;
    case GLFW_KEY_LEFT_CONTROL:
    case GLFW_KEY_RIGHT_CONTROL: state = &movement.ctrl; break;
    case GLFW_KEY_EQUAL:
    case GLFW_KEY_KP_ADD: state = &movement.plus; break;
    case GLFW_KEY_MINUS:
    case GLFW_KEY_KP_SUBTRACT: state = &movement.minus; break;
    default: return false;
    }
    *state = pressed;
    return true;
}
