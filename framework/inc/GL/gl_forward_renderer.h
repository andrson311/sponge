#pragma once

#include "rendering_system.h"
#include "gl_scene.h"
#include "core_model.h"
#include "framebuffer.h"
#include "gl_buffer.h"
#include "shadow_cube_map_fbo.h"

#include "gl_base_lighting_technique.h"
#include "gl_forward_lighting.h"
#include "gl_forward_skinning.h"
#include "gl_normal_technique.h"
#include "gl_shadow_mapping_technique.h"
#include "gl_shadow_mapping_technique_point_light.h"
#include "gl_picking_technique.h"
#include "gl_picking_texture.h"
#include "gl_pbr_forward_lighting.h"
#include "gl_full_screen_technique.h"
#include "gl_bright_filter_technique.h"
#include "gl_ssao_technique.h"
#include "gl_ssao_combine_technique.h"
#include "gl_tone_map_technique.h"
#include "gl_hdr_technique.h"
#include "gl_geometry_technique.h"
#include "gl_ssgi_technique.h"
#include "gl_blur_filter1_technique.h"
#include "gl_blur_filter2_technique.h"
#include "gl_infinite_grid.h"
#include "gl_skybox.h"
#include "gl_terrain_technique.h"

enum RENDER_PASS
{
    RENDER_PASS_UNINITIALIZED = 0,
    RENDER_PASS_LIGHTING_DIR = 1,
    RENDER_PASS_LIGHTING_SPOT = 2,
    RENDER_PASS_LIGHTING_POINT = 3,
    RENDER_PASS_SHADOW_DIR = 4,
    RENDER_PASS_SHADOW_SPOT = 5,
    RENDER_PASS_SHADOW_POINT = 6,
    RENDER_PASS_PICKING = 7,
    RENDER_PASS_NORMAL = 8,
    RENDER_PASS_GBUFFER = 9
};

enum LIGHTING_TECHNIQUE
{
    FORWARD_LIGHTING,
    FORWARD_SKINNING,
    PBR_GLTF2_LIGHTING,
    PBR_GLTF2_SKINNING,
    UNDEFINED_TECHNIQUE
};

enum LIGHT_TYPE
{
    LIGHT_TYPE_DIR,
    LIGHT_TYPE_POINT,
    LIGHT_TYPE_SPOT
};

struct LightSource
{
    glm::vec3 Color;
    LIGHT_TYPE LightType;

    glm::vec3 Direction;
    float AmbientIntensity;

    glm::vec3 WorldPos;
    float DiffuseIntensity;

    float Atten_Constant;
    float Atten_Linear;
    float Atten_Exp;
    float Cutoff;
};

static_assert(sizeof(LightSource) == 64, "LightSource struct must be 64 bytes");

#define MAX_NUM_LIGHTS 4

class RenderingSystemGL;

class ForwardRenderer : public RenderCallbacks
{
public:
    ForwardRenderer();
    ~ForwardRenderer();

    void InitForwardRenderer(RenderingSystemGL *pRenderingSystemGL);
    void ReloadShaders();
    void SetCamera(CameraFPS *pCamera)
    {
        m_pCurCamera = pCamera;
    }

    void Render(void *pWindow, GLScene *pScene, GameCallbacks *pGameCallbacks,
                double totalRuntime, double deltaTime);

    virtual void DrawStartCB(u_int drawIndex);
    virtual void ControlSpecularExponentCB(bool isEnabled);
    virtual void SetMaterialCB(const CoreMaterial &material);
    virtual void SetWorldMatrixCB(const glm::mat4 &world);

    GLuint GetSSAOTextureHandle() const { return m_ssaoFBO.GetTexture(); }

private:
    void HandleEmptyRenderList(GLScene *pScene);
    void RenderTerrain(SceneConfig *pConfig);
    void ExecuteRenderGraph(GLScene *pScene, double totalRuntime);

    void PickingPass(void *pWindow, GLScene *pScene);
    void PickingRenderScene(GLScene *pScene);
    int GetPickedObjectIndex(void *pWindow, GLScene *pScene);
    void PostPickingPass(void *pWindow, GLScene *pScene);
    void SavePickedObject(GLScene *pScene, int objectIndex);

    void PrePass(GLScene *pScene);

    void ShadowMapPass(GLScene *pScene);
    void ShadowMapPassPoint(const std::list<CoreSceneObject *> &renderList,
                            const std::vector<PointLight> &pointLights);
    void ShadowMapPassDirAndSpot(const std::list<CoreSceneObject *> &renderList);

    void PostProcessPass(GLScene *pScene);

    void NormalPass(GLScene *pScene);

    void LightingPass(GLScene *pScene, double totalRuntime);
    void LightingPassFBOSetup(GLScene *pScene);

    void HDRPassGPU(float &avgLogLum, float &exposure);
    void HDRPassCPU(float &avgLogLum, float &exposure);

    void UpdateLightSources(GLScene *pScene);
    void SetupLightSourcesArray(GLScene *pScene);

    void SSAOPass(GLScene *pScene);
    void SSAOCombinePass();

    void ToneMappingPass(GLScene *pScene, float averageLuminance, float exposure);
    void GBufferPass(GLScene *pScene);

    void BrightPass(GLScene *pScene, float averageLuminance);

    void BlurFilter1Pass(GLScene *pScene);
    void BlurFilter2Pass(GLScene *pScene);

    void SSGIPass(GLScene *pScene);

    void FullScreenQuadBlit(GLScene *pScene);

    void BindShadowMaps();

    void RenderObjectList(GLScene *pScene, double totalRuntime);
    void RenderWithForwardLighting(GLScene *pScene, CoreSceneObject *pSceneObject,
                                   double totalRuntime);
    void StartRenderWithForwardLighting(GLScene* pScene, CoreSceneObject* pSceneObject, double totalRuntime);
    void RenderInfiniteGrid(GLScene *pScene);
    void GetWVP(CoreSceneObject *pSceneObject, glm::mat4 &WVP);

    void SwitchToLightingTech(LIGHTING_TECHNIQUE tech);
    LIGHTING_TECHNIQUE GetLightingTech(GLScene *pScene, CoreModel *pModel);

    void ApplySceneConfig(GLScene *pScene);

    void InitShadowMapping();

    void InitTechniques();

    void SetWorldMatrix_CB_ShadowPassDir(const glm::mat4 &world);
    void SetWorldMatrix_CB_ShadowPassSpot(const glm::mat4 &world);
    void SetWorldMatrix_CB_ShadowPassPoint(const glm::mat4 &world);
    void SetWorldMatrix_CB_LightingPass(const glm::mat4 &world);
    void SetWorldMatrix_CB_PickingPass(const glm::mat4 &world);
    void SetWorldMatrix_CB_NormalPass(const glm::mat4 &world);
    void SetWorldMatrix_CB_GBufferPass(const glm::mat4 &world);

    void RenderEntireRenderList(const std::list<CoreSceneObject *> &renderList);

    glm::mat4 GetViewProjectionMatrix();

    void RenderSingleObject(CoreSceneObject *pSceneObject);
    void SetRenderToDefaultFB();

    int m_windowWidth = -1;
    int m_windowHeight = -1;

    RENDER_PASS m_curRenderPass = RENDER_PASS_UNINITIALIZED;
    CoreSceneObject *m_pCurSceneObject = NULL;

    RenderingSystemGL *m_pRenderingSystemGL = NULL;
    CameraFPS *m_pCurCamera = NULL;

    Framebuffer m_lightingFBO;
    Framebuffer m_normalFBO;
    Framebuffer m_ssaoFBO;
    Framebuffer m_hdrFBO;
    Framebuffer m_ssgiFBO;
    Framebuffer m_brightFilterFBO[2];
    GLBuffer m_ssaoParams;
    Texture m_ssaoRotTexture;
    GLBuffer m_lightParams;
    std::vector<LightSource> m_lightSources;
    std::vector<float> m_hdrData;
    GLBuffer m_luminanceBuffer;

    // shadows
    Framebuffer m_shadowMapFBO;
    ShadowCubeMapFBO m_shadowCubeMapFBO;
    glm::mat4 m_lightPersProjMatrix;
    glm::mat4 m_lightOrthoProjMatrix;
    glm::mat4 m_lightViewMatrix;

    // techniques
    LIGHTING_TECHNIQUE m_curLightingTechId = UNDEFINED_TECHNIQUE;
    BaseLightingTechnique *m_pCurLightingTech = &m_lightingTech;
    ForwardLightingTechnique m_lightingTech;
    PBRForwardLightingTechnique m_pbrLightingTech;
    ForwardSkinningTechnique m_skinningTech;
    PBRSkinningTechnique m_pbrSkinningTech;
    NormalTechnique m_normalTech;
    ShadowMappingTechnique m_shadowMapTech;
    ShadowMappingPointLightTechnique m_shadowMapPointLightTech;

    PickingTechnique m_pickingTech;
    PickingTexture m_pickingTexture;

    FullScreenTechnique m_fullScreenTech;
    BrightFilterTechnique m_brightFilterTech;
    SSAOTechnique m_ssaoTech;
    SSAOCombineTechnique m_ssaoCombineTech;
    ToneMapTechnique m_toneMapTech;
    ToneMapTechnique m_toneMapTechWithBloom;
    HDRTechnique m_hdrTech;
    GeometryTechnique m_geometryTech;
    SSGITechnique m_ssgiTech;
    BlurFilter1Technique m_blurFilter1Tech;
    BlurFilter2Technique m_blurFilter2Tech;
    TerrainTechnique m_terrainTech;
    
    int m_hdrNumGroupsX = 0;
    int m_hdrNumGroupsY = 0;

    InfiniteGrid m_infiniteGrid;
    Skybox m_skybox;
};