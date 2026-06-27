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
// #include "fault_formation_terrain.h"
#include "midpoint_disp_terrain.h"
#include "texture_config.h"
#include "texture_generator.h"

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
    void InitTerrainTextureGenerator();
    void InitTerrainMultiTextures();
    void InitGUI();

    GLFWwindow* window = NULL;
    Camera* m_pGameCamera = NULL;
    bool m_isWireframe = false;
    MidpointDispTerrain m_terrain;
    bool m_showGui = false;
    bool m_isPaused = false;
    int m_terrainSize = 512;
    float m_roughness = 1.0f;
    float m_lightSoftness = 4.0f;
    float m_minHeight = 0.0f;
    float m_maxHeight = 356.0f;
    glm::vec3 m_lightDir = glm::vec3(1.0f, -0.5f, 1.0f);
    float m_counter = 0.0f;
};
