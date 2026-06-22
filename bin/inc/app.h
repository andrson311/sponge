#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "glfw.h"
#include "util.h"
#include "camera.h"
#include "renderer.h"

#include "lighting_technique.h"
#include "basic_mesh.h"
#include "world_transform.h"
#include "framebuffer.h"
#include "shadow_mapping_technique.h"
#include "shadow_cube_map_fbo.h"
#include "shadow_mapping_technique_point_light.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define SHADOW_MAP_SIZE 4096

struct CameraDirection
{
    GLenum CubemapFace;
    glm::vec3 Target;
    glm::vec3 Up;
};

extern CameraDirection gCameraDirections[NUM_CUBE_MAP_FACES];

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
    void ShadowMapPass();
    void LightingPass();

    void KeyBoardCB(u_int key, int state);
    void PassiveMouseCB(int x, int y);
    void MouseCB(int button, int action, int x, int y);

private:
    void ProcessHeldKeys();
    void CreateWindow();
    void CreateShadowMap();
    void InitCallbacks();
    void InitCamera();
    // void InitRenderer();
    void InitShaders();
    void InitMesh();

    GLFWwindow *window = NULL;
    Camera *m_pGameCamera = NULL;
    // PhongRenderer m_phongRenderer;
    LightingTechnique m_lightingTech;
    ShadowMappingPointLightTechnique m_shadowMapTech;
    BasicMesh *m_pMesh1 = NULL;
    BasicMesh *m_pMesh2 = NULL;
    BasicMesh *m_pTerrain = NULL;
    PersProjInfo m_persProjInfo;
    glm::mat4 m_lightPersProjMatrix;
    glm::mat4 m_cameraOrthoProjMatrix;
    // DirectionalLight m_dirLight;
    PointLight m_pointLight;
    // Framebuffer m_shadowMapFBO;
    ShadowCubeMapFBO m_shadowCubeMapFBO;
    glm::vec3 m_cameraPos;
    glm::vec3 m_cameraTarget;
    bool m_cameraOnLight = false;
    glm::vec3 m_housePositions[4] = {};
    glm::vec3 m_cylinderPositions[4] = {};
    bool m_isOrthoCamera = false;
};