#pragma once

struct CameraMovement {
    bool forward = false;
    bool backward = false;
    bool strafeLeft = false;
    bool strafeRight = false;
    bool up = false;
    bool down = false;
    bool fastSpeed = false;
    bool plus = false;
    bool minus = false;
    bool ctrl = false;
};

bool GLFWCameraHandler(CameraMovement& movement, int key, int action, int mods);