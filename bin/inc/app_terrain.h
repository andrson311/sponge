#pragma once

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#include "util.h"
#include "camera.h"
#include "glfw.h"
#include "fault_formation_terrain.h"

class AppTerrain
{
public:
    AppTerrain() {}
    virtual ~AppTerrain();

    void Init();
    void Run();
    void RenderScene();

    void PassiveMouseCB(int x, int y);
    void KeyboardCB(u_int key, int state);
    void ProcessHeldKeys();
    void MouseCB(int button, int action, int x, int y);

private:
    void CreateWindow();
    void InitCallbacks();
    void InitCamera();
    void InitTerrain();

    GLFWwindow* window = NULL;
    Camera* m_pGameCamera = NULL;
    bool m_isWireframe = false;
    FaultFormationTerrain m_terrain;
};
