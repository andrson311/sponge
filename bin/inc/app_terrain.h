#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include <unistd.h>

#include "util.h"
#include "camera.h"
#include "glfw.h"

#include "texture_config.h"
#include "midpoint_disp_terrain.h"

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
    void InitGUI();

    GLFWwindow* window = NULL;
    Camera* m_pGameCamera = NULL;
    bool m_isWireframe = false;
    MidpointDispTerrain m_terrain;
    bool m_showGui = false;
    bool m_isPaused = false;
    int m_terrainSize = 513;
    float m_roughness = 1.0f;
    float m_minHeight = 0.0f;
    float m_maxHeight = 150.0f;
    int m_patchSize = 33;
    float m_counter = 0.0f;
};
