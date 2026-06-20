#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "glfw.h"
#include "camera.h"
#include "renderer.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

class App
{
public:
    App();
    virtual ~App();

    void Init();
    void Run();
    void RenderSceneCB();
    // void PickingPhase();
    // void RenderPhase();
    // void HandleMouseButtonPressed();
    // void DragTheObject();
    // void RenderObjects();

    void KeyBoardCB(u_int key, int state);
    void PassiveMouseCB(int x, int y);
    void MouseCB(int button, int action, int x, int y);

private:
    void CreateWindow();
    void InitCallbacks();
    void InitCamera();
    void InitRenderer();
    void InitMesh();
    
    GLFWwindow *window = NULL;
    Camera *m_pGameCamera = NULL;
    PhongRenderer m_phongRenderer;
    BasicMesh *m_pMesh1 = NULL;
    BasicMesh *m_pTerrain = NULL;
    PersProjInfo m_persProjInfo;
    PointLight m_pointLights[LightingTechnique::MAX_POINT_LIGHTS];
    DirectionalLight m_directionalLight;
    bool m_isRimLightEnabled = false;
    bool m_isCellShadingEnabled = false;
    bool m_mobileCamera = false;

    struct
    {
        bool IsPressed = false;
        bool FirstTime = true;
        int x;
        int y;
    } m_leftMouseButton;

    glm::vec3 m_worldPos[3];

    glm::vec4 m_objViewSpacePos;
    glm::vec3 m_intersectionPoint;
    glm::vec3 m_translation;
    int m_clicked_object_id = -1;
};