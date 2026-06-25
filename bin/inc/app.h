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
#include "framebuffer_object.h"
#include "shadow_map_offset_texture.h"

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
    void InitRenderer();
    void InitShaders();
    void InitMesh();

    GLFWwindow *window = NULL;
    Camera *m_pGameCamera = NULL;
    PhongRenderer m_phongRenderer;
    LightingTechnique m_lightingTech;
    ShadowMappingTechnique m_shadowMapTech;
    SkinnedMesh *m_pMesh = NULL;
    BasicMesh *m_pMesh1 = NULL;
    BasicMesh *m_pTerrain = NULL;
    PersProjInfo m_persProjInfo;
    glm::mat4 m_lightOrthoProjMatrix;
    glm::mat4 m_cameraOrthoProjMatrix;
    DirectionalLight m_dirLight;
    FramebufferObject m_shadowMapFBO;
    glm::vec3 m_cameraPos;
    glm::vec3 m_cameraTarget;
    bool m_cameraOnLight = false;
    bool m_isOrthoCamera = false;
    ShadowMapOffsetTexture *m_pShadowMapOffsetTexture = NULL;
    int m_shadowMapFilterSize = 0;
    float m_shadowMapSampleRadius = 0.0f;
    int m_shadowMapOffsetTextureSize = 16;
    int m_shadowMapOffsetFilterSize = 8;

    long long m_startTime = 0;
    long long m_currentTime = 0;
    bool m_runAnimation = true;
    long long m_totalPauseTime = 0;
    long long m_pauseStart = 0;
    int m_animationIndex = 0;
    float m_blendFactor = 0.0f;
};