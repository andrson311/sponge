#include "framebuffer.h"
#include "gl_engine_common.h"
#include "gl_forward_renderer.h"
#include "gl_rendering_system.h"
#include "gl_model.h"

#define SHADOW_MAP_WIDTH 2048
#define SHADOW_MAP_HEIGHT 2048

extern bool usePVP;
extern bool useIndirectRender;
static bool useBlitForFinalCopy = true;

#define SSAO_UBO_INDEX 0
#define LIGHT_UBO_INDEX 1

struct CameraDirection
{
    GLenum CubemapFace;
    glm::vec3 Target;
    glm::vec3 Up;
};

CameraDirection gCameraDirections[NUM_CUBE_MAP_FACES] = {
    {GL_TEXTURE_CUBE_MAP_POSITIVE_X, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_X, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
    {GL_TEXTURE_CUBE_MAP_POSITIVE_Y, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
    {GL_TEXTURE_CUBE_MAP_POSITIVE_Z, glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, glm::vec3(1.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)}};

struct SSAOParamsInternal
{
    SSAOParams params;
    float zNear = 0.0f;
    float zFar = 0.0f;
    float OutputWidth = 0.0f;
    float OutputHeight = 0.0f;
};

static bool IsLightingPass(RENDER_PASS renderPass)
{
    switch (renderPass)
    {
    case RENDER_PASS_LIGHTING_DIR:
    case RENDER_PASS_LIGHTING_SPOT:
    case RENDER_PASS_LIGHTING_POINT:
        return true;
    }

    return false;
}

ForwardRenderer::ForwardRenderer() : m_ssaoRotTexture(GL_TEXTURE_2D),
                                     m_toneMapTech(false), m_toneMapTechWithBloom(true) {}

ForwardRenderer::~ForwardRenderer() {}

void ForwardRenderer::InitForwardRenderer(RenderingSystemGL *pRenderingSystemGL)
{
    if (!pRenderingSystemGL)
    {
        printf("%s:%d - must provide a rendering system\n", __FILE__, __LINE__);
        exit(1);
    }

    m_pRenderingSystemGL = pRenderingSystemGL;
    m_pRenderingSystemGL->GetWindowSize(m_windowWidth, m_windowHeight);

    InitTechniques();
    InitShadowMapping();

    m_pickingTexture.Init(m_windowWidth, m_windowHeight);
    m_infiniteGrid.Init(SHADOW_TEXTURE_UNIT_INDEX);
    m_skybox.Init(SKYBOX_TEXTURE_UNIT, SKYBOX_TEXTURE_UNIT_INDEX);
    m_lightingFBO.Init(m_windowWidth, m_windowHeight, 4, false, true, false);
    m_normalFBO.Init(m_windowWidth, m_windowHeight, 3, false, true, false);
    m_ssaoFBO.Init(m_windowWidth, m_windowHeight, 3, false, false, false);
    m_hdrFBO.Init(m_windowWidth, m_windowHeight, 3, true, true, false);
    m_ssgiFBO.Init(m_windowWidth, m_windowHeight, 4, false, true, true);
    m_brightFilterFBO[0].Init(m_windowWidth / 8, m_windowHeight / 8, 3, true, false, false);
    m_brightFilterFBO[1].Init(m_windowWidth / 8, m_windowHeight / 8, 3, true, false, false);

    int size = m_windowWidth * m_windowHeight;
    m_hdrData.resize(size * 3);

    int local_size_x = 10;
    int local_size_y = 10;

    m_hdrNumGroupsX = (int)AlignUpToMultiple(m_windowWidth, local_size_x) / local_size_x;
    m_hdrNumGroupsY = (int)AlignUpToMultiple(m_windowWidth, local_size_y) / local_size_y;

    int numTiles = m_hdrNumGroupsX * m_hdrNumGroupsY;

    m_luminanceBuffer.InitBuffer(numTiles * sizeof(float),
                                 NULL, GL_MAP_READ_BIT | GL_CLIENT_STORAGE_BIT);
    m_ssaoParams.InitBuffer(sizeof(SSAOParamsInternal), NULL, GL_DYNAMIC_STORAGE_BIT);
    m_lightSources.resize(MAX_NUM_LIGHTS);
    m_lightParams.InitBuffer(std::size(m_lightSources), NULL, GL_DYNAMIC_STORAGE_BIT);
    m_ssaoRotTexture.Load("res/textures/rot_texture.bmp");

    glUseProgram(0);
}

void ForwardRenderer::ReloadShaders()
{
    InitTechniques();
}

void ForwardRenderer::InitTechniques()
{
    if (!m_lightingTech.Init())
    {
        printf("Error initializing the lighting technique\n");
        exit(1);
    }

    m_lightingTech.Enable();
    m_lightingTech.SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetShadowMapTextureUnit(SHADOW_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetShadowCubeMapTextureUnit(SHADOW_CUBE_MAP_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetNormalMapTextureUnit(NORMAL_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetHeightMapTextureUnit(HEIGHT_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetSpecularExponentTextureUnit(SPECULAR_EXPONENT_UNIT_INDEX);
    m_lightingTech.SetCubeMapTextureUnit(SKYBOX_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetProjectedTextureUnit(PROJECTED_TEXTURE_UNIT_INDEX);

    if (!m_skinningTech.Init())
    {
        printf("Error initializing the skinning technique\n");
        exit(1);
    }

    m_skinningTech.Enable();
    m_skinningTech.SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    m_skinningTech.SetShadowMapTextureUnit(SHADOW_TEXTURE_UNIT_INDEX);
    m_skinningTech.SetShadowCubeMapTextureUnit(SHADOW_CUBE_MAP_TEXTURE_UNIT_INDEX);
    m_skinningTech.SetNormalMapTextureUnit(NORMAL_TEXTURE_UNIT_INDEX);
    m_skinningTech.SetHeightMapTextureUnit(HEIGHT_TEXTURE_UNIT_INDEX);
    m_skinningTech.SetSpecularExponentTextureUnit(SPECULAR_EXPONENT_UNIT_INDEX);

    if (!m_pbrLightingTech.Init())
    {
        printf("Error initializing the PBR lighting technique\n");
        exit(1);
    }

    m_pbrLightingTech.Enable();
    m_pbrLightingTech.SetAlbedoTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetAmbientOcclusionTextureUnit(AO_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetEmissiveTextureUnit(EMISSIVE_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetRoughnessTextureUnit(ROUGHNESS_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetNormalTextureUnit(NORMAL_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetEnvmapTextureUnit(ENVMAP_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetBRDF_LUTTextureUnit(BRDF_LUT_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetIrradianceTextureUnit(IRRADIANCE_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetClearCoatTextureUnit(CLEARCOAT_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetClearCoatRoughnessTextureUnit(CLEARCOAT_ROUGHNESS_TEXTURE_UNIT_INDEX);
    m_pbrLightingTech.SetClearCoatNormalTextureUnit(CLEARCOAT_NORMAL_TEXTURE_UNIT_INDEX);

    if (!m_pbrSkinningTech.Init())
    {
        printf("Error initializing the PBR skinning technique\n");
        exit(1);
    }

    m_pbrSkinningTech.Enable();
    m_pbrSkinningTech.SetAlbedoTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    m_pbrSkinningTech.SetAmbientOcclusionTextureUnit(AO_TEXTURE_UNIT_INDEX);
    m_pbrSkinningTech.SetEmissiveTextureUnit(EMISSIVE_TEXTURE_UNIT_INDEX);
    m_pbrSkinningTech.SetRoughnessTextureUnit(ROUGHNESS_TEXTURE_UNIT_INDEX);
    m_pbrSkinningTech.SetNormalTextureUnit(NORMAL_TEXTURE_UNIT_INDEX);
    m_pbrSkinningTech.SetEnvmapTextureUnit(ENVMAP_TEXTURE_UNIT_INDEX);
    m_pbrSkinningTech.SetBRDF_LUTTextureUnit(BRDF_LUT_TEXTURE_UNIT_INDEX);
    m_pbrSkinningTech.SetIrradianceTextureUnit(IRRADIANCE_TEXTURE_UNIT_INDEX);
    m_pbrSkinningTech.SetClearCoatTextureUnit(CLEARCOAT_TEXTURE_UNIT_INDEX);
    m_pbrSkinningTech.SetClearCoatRoughnessTextureUnit(CLEARCOAT_ROUGHNESS_TEXTURE_UNIT_INDEX);
    m_pbrSkinningTech.SetClearCoatNormalTextureUnit(CLEARCOAT_NORMAL_TEXTURE_UNIT_INDEX);

    if (!m_shadowMapTech.Init())
    {
        printf("Error initializing the shadow mapping technique\n");
        exit(1);
    }

    if (!m_shadowMapPointLightTech.Init())
    {
        printf("Error initializing the shadow mapping point light technique\n");
        exit(1);
    }

    if (!m_pickingTech.Init())
    {
        printf("Error initializing the picking technique\n");
        exit(1);
    }

    if (!m_fullScreenTech.Init("framework/shaders/GL/tex.fs"))
    {
        printf("Error initializing the full screen quad technique\n");
        exit(1);
    }

    if (!m_ssaoTech.Init())
    {
        printf("Error initializing the SSAO technique\n");
        exit(1);
    }

    if (!m_ssaoCombineTech.Init())
    {
        printf("Error initializing the SSAO combine technique\n");
        exit(1);
    }

    if (!m_normalTech.Init())
    {
        printf("Error initializing the normal technique\n");
        exit(1);
    }

    if (!m_toneMapTech.Init())
    {
        printf("Error initializing the tone mapping technique\n");
        exit(1);
    }

    m_toneMapTech.Enable();
    m_toneMapTech.SetHDRSampler(0);

    if (!m_toneMapTechWithBloom.Init())
    {
        printf("Error initializing the tone mapping technique with bloom\n");
        exit(1);
    }

    m_toneMapTechWithBloom.Enable();
    m_toneMapTechWithBloom.SetHDRSampler(0);
    m_toneMapTechWithBloom.SetBlurSampler(1);

    if (!m_hdrTech.Init())
    {
        printf("Error initializing the HDR technique\n");
        exit(1);
    }

    if (!m_geometryTech.Init())
    {
        printf("Error initializing the geometry technique\n");
        exit(1);
    }

    m_geometryTech.Enable();
    m_geometryTech.SetAlbedoTextureUnit(COLOR_TEXTURE_UNIT_INDEX);

    if (!m_ssgiTech.Init())
    {
        printf("Error initializing the SSGI technique\n");
        exit(1);
    }

    m_ssgiTech.Enable();
    m_ssgiTech.SetAlbedoTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    m_ssgiTech.SetNormalTextureUnit(NORMAL_TEXTURE_UNIT_INDEX);
    m_ssgiTech.SetDepthTextureUnit(DEPTH_TEXTURE_UNIT_INDEX);

    if (!m_brightFilterTech.Init())
    {
        printf("Error initializing the bright filter technique\n");
        exit(1);
    }

    if (!m_blurFilter1Tech.Init())
    {
        printf("Error initializing the blur filter 1 technique\n");
        exit(1);
    }

    if (!m_blurFilter2Tech.Init())
    {
        printf("Error initializing the blur filter 2 technique\n");
        exit(1);
    }

    if (!m_terrainTech.Init())
    {
        printf("Error initializing the heightmap technique\n");
        exit(1);
    }
}

void ForwardRenderer::InitShadowMapping()
{
    float FOV = 90.0f; // TODO: get from light
    float zNear = 1.0f;
    float zFar = 100.0;

    PersProjInfo shadowPersProjInfo{
        FOV,
        SHADOW_MAP_WIDTH,
        SHADOW_MAP_HEIGHT,
        zNear,
        zFar};

    m_lightPersProjMatrix = glm::perspectiveFov(glm::radians(FOV),
                                                (float)SHADOW_MAP_WIDTH,
                                                (float)SHADOW_MAP_HEIGHT,
                                                zNear,
                                                zFar);

    // initialize an orthographic projection for directional light
    OrthoProjInfo shadowOrthoProjInfo;
    shadowOrthoProjInfo.l = -20.0f;
    shadowOrthoProjInfo.r = 20.0f;
    shadowOrthoProjInfo.b = -20.0f;
    shadowOrthoProjInfo.t = 20.0f;
    shadowOrthoProjInfo.n = -20.0f;
    shadowOrthoProjInfo.f = 20.0f;

    m_lightOrthoProjMatrix = glm::ortho(shadowOrthoProjInfo.l,
                                        shadowOrthoProjInfo.r,
                                        shadowOrthoProjInfo.b,
                                        shadowOrthoProjInfo.t,
                                        shadowOrthoProjInfo.n,
                                        shadowOrthoProjInfo.f);

    m_shadowMapFBO.Init(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, 0, false, true, false);

    if (!m_shadowCubeMapFBO.Init(SHADOW_MAP_WIDTH))
    {
        exit(1);
    }
}

void ForwardRenderer::SwitchToLightingTech(LIGHTING_TECHNIQUE techID)
{
    if (m_curLightingTechId != techID)
    {
        switch (techID)
        {
        case FORWARD_LIGHTING:
            m_pCurLightingTech = &m_lightingTech;
            break;

        case FORWARD_SKINNING:
            m_pCurLightingTech = &m_skinningTech;
            break;

        case PBR_GLTF2_LIGHTING:
            m_pCurLightingTech = &m_pbrLightingTech;
            break;

        case PBR_GLTF2_SKINNING:
            m_pCurLightingTech = &m_pbrSkinningTech;
            break;

        default:
            assert(0);
        }
    }

    m_pCurLightingTech->Enable();
}

LIGHTING_TECHNIQUE ForwardRenderer::GetLightingTech(GLScene *pScene, CoreModel *pModel)
{
    LIGHTING_TECHNIQUE lightingTech = UNDEFINED_TECHNIQUE;

    bool isPBR = pModel->IsPBR();

    if (pScene->GetConfig()->IsPBRDisabled())
    {
        isPBR = false;
    }

    if (pModel->IsAnimated())
    {
        if (isPBR)
        {
            lightingTech = PBR_GLTF2_SKINNING;
        }
        else
        {
            lightingTech = FORWARD_SKINNING;
        }
    }
    else
    {
        if (isPBR)
        {
            lightingTech = PBR_GLTF2_LIGHTING;
        }
        else
        {
            lightingTech = FORWARD_LIGHTING;
        }
    }

    return lightingTech;
}

void ForwardRenderer::Render(void *pWindow, GLScene *pScene, GameCallbacks *pGameCallbacks,
                             double totalRuntime, double deltaTime)
{
    if (pScene->IsClearFrame())
    {
        const glm::vec4 &clearColor = pScene->GetClearColor();
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    pGameCallbacks->OnFrame(deltaTime);

    if (!m_pCurCamera)
    {
        printf("ForwardRenderer: camera not initialized\n");
        return;
    }

    if (pScene->GetRenderList().size() == 0)
    {
        HandleEmptyRenderList(pScene);
    }
    else
    {
        if (pScene->GetConfig()->IsPickingEnabled())
        {
            PickingPass(pWindow, pScene);
            pScene->GetConfig()->ControlPicking(false);
        }

        ExecuteRenderGraph(pScene, totalRuntime);
    }

    pGameCallbacks->OnFrameEnd();
    m_curRenderPass = RENDER_PASS_UNINITIALIZED;
}

void ForwardRenderer::ExecuteRenderGraph(GLScene *pScene, double totalRuntime)
{
    PrePass(pScene);
    ShadowMapPass(pScene);
    LightingPass(pScene, totalRuntime);
    PostProcessPass(pScene);
}

void ForwardRenderer::PrePass(GLScene *pScene)
{
    bool isSSGI = pScene->GetConfig()->IsSSGIEnabled();

    if (isSSGI)
    {
        GBufferPass(pScene); // must be before the shadow map pass ...
    }
}

void ForwardRenderer::PostProcessPass(GLScene *pScene)
{
    bool isHDR = pScene->GetConfig()->IsHDREnabled();
    bool isSSGI = pScene->GetConfig()->IsSSGIEnabled();
    bool isBloom = pScene->GetConfig()->IsBloomEnabled();

    float averageLuminance = 0.0f;
    float exposure = 0.0f;

    if (isHDR)
    {
        // HDRPassCPU(averageLuminance, exposure);
        HDRPassGPU(averageLuminance, exposure);
        pScene->GetConfig()->SetHDRParams(averageLuminance, exposure);

        if (isBloom)
        {
            BrightPass(pScene, averageLuminance);
            BlurFilter1Pass(pScene);
            BlurFilter2Pass(pScene);
        }
    }

    if (pScene->GetConfig()->IsSkyboxEnabled())
    {
        m_skybox.Render(pScene->GetSkyboxTex(), m_pCurCamera->GetVPMatrixNoTranslate());
    }

    if (isSSGI)
    {
        SSGIPass(pScene);
    }
    else if (pScene->GetConfig()->IsSSAOEnabled())
    {
        NormalPass(pScene);
        SSAOPass(pScene);
        SSAOCombinePass();
    }
    else if (isHDR)
    {
        ToneMappingPass(pScene, averageLuminance, exposure);
    }
}

void ForwardRenderer::HandleEmptyRenderList(GLScene *pScene)
{
    SceneConfig *pConfig = pScene->GetConfig();

    if (pConfig->IsSkyboxEnabled())
    {
        m_skybox.Render(pScene->GetSkyboxTex(), m_pCurCamera->GetVPMatrixNoTranslate());
    }
    else if (pConfig->GetTerrainGrid())
    {
        RenderTerrain(pConfig);
    }
    else if (pConfig->GetInfiniteGrid().Enabled)
    {
        RenderInfiniteGrid(pScene);
    }
    else
    {
        printf("Warning! Render list is empty, no main model, no skybox");
    }
}

void ForwardRenderer::RenderTerrain(SceneConfig *pConfig)
{
    TerrainGrid *pTerrainGrid = (TerrainGrid *)pConfig->GetTerrainGrid();
    m_terrainTech.Enable();
    glm::mat4 GlobalWorldRotation = m_pCurCamera->GetGlobalWorldRotation();
    glm::mat4 WVP = m_pCurCamera->GetVPMatrix() * GlobalWorldRotation;
    m_terrainTech.SetWVP(WVP);
    int heightMap = pConfig->GetTerrainHeightMap();
    if (heightMap != -1)
    {
        Texture *pHeightMap = (Texture *)m_pRenderingSystemGL->GetTexture(heightMap);
        pHeightMap->Bind(GL_TEXTURE0);
    }

    for (int i = 0; i < pConfig->GetTerrainTextureCount(); i++)
    {
        int textureMap = pConfig->GetTerrainTexture(i);
        if (textureMap != -1)
        {
            Texture *pTex = (Texture *)m_pRenderingSystemGL->GetTexture(textureMap);
            pTex->Bind(GL_TEXTURE1 + i);
        }
    }

    m_terrainTech.SetMaxTerrainHeight(pConfig->GetTerrainMaxHeight());
    m_terrainTech.SetHorizontalScale(pConfig->GetTerrainHorizontalScale());
    m_terrainTech.SetHeightPercents(pConfig->GetTerrainLowHeightPercent(),
                                    pConfig->GetTerrainHighHeightPercent());
    m_terrainTech.SetSunlightDir(pConfig->GetTerrainSunlightDir());
    m_terrainTech.SetAmbientLightFactor(pConfig->GetTerrainAmbientFactor());
    m_terrainTech.SetTexTileSizeInWorldUnits(pConfig->GetTerrainTexTileSizeInWorldUnits());
    m_terrainTech.SetRenderMode(pConfig->GetTerrainRenderMode());
    pTerrainGrid->Render();
}

void ForwardRenderer::ApplySceneConfig(GLScene *pScene)
{
    SceneConfig *pConfig = pScene->GetConfig();

    m_pCurLightingTech->ControlShadows(pConfig->IsShadowMappingEnabled());
    m_pCurLightingTech->ControlRefRefract(pConfig->IsRefRefractEnabled());
    m_pCurLightingTech->SetReflectionFactor(pConfig->GetReflectionFactor());
    m_pCurLightingTech->SetMaterialToRefRefractFactor(pConfig->GetMatRefRefractFactor());
    m_pCurLightingTech->SetRefractETA(1.0f / pConfig->GetIndexOfRefraction());
    m_pCurLightingTech->SetFresnelPower(pConfig->GetFresnelPower());
    m_pCurLightingTech->SetRenderMode(pConfig->GetRenderMode());

    int projectedTex = pConfig->GetProjectedTexture();

    if (projectedTex != -1)
    {
        m_pCurLightingTech->SetProjectionMatrix(pConfig->GetProjectionMatrix());
        Texture *pProjectedTexture = (Texture *)m_pRenderingSystemGL->GetTexture(projectedTex);
        pProjectedTexture->Bind(PROJECTED_TEXTURE_UNIT);
    }

    int EnvMap = pConfig->GetEnvMap();

    if (EnvMap >= 0)
    {
        CubemapTexture *pCubemapTex = (CubemapTexture *)m_pRenderingSystemGL->GetTexture(EnvMap);
        pCubemapTex->Bind(ENVMAP_TEXTURE_UNIT);
    }

    if (pConfig->pBRDF_LUT)
    {
        pConfig->pBRDF_LUT->Bind(BRDF_LUT_TEXTURE_UNIT);
    }

    int irradianceMap = pConfig->GetIrradianceMap();

    if (irradianceMap >= 0)
    {
        CubemapTexture *pCubemapTex = (CubemapTexture *)m_pRenderingSystemGL->GetTexture(irradianceMap);
        pCubemapTex->Bind(IRRADIANCE_TEXTURE_UNIT);
    }
}

void ForwardRenderer::UpdateLightSources(GLScene *pScene)
{
    SetupLightSourcesArray(pScene);

    // TODO: can update only the correct number of lights
    m_lightParams.Update(m_lightSources.data(), std::size(m_lightSources));
}

void ForwardRenderer::SetupLightSourcesArray(GLScene *pScene)
{
    int lightIndex = 0;

    for (const SpotLight &l : pScene->GetSpotLights())
    {
        assert(lightIndex < MAX_NUM_LIGHTS);
        m_lightSources[lightIndex].LightType = LIGHT_TYPE_SPOT;
        m_lightSources[lightIndex].AmbientIntensity = l.AmbientIntensity;
        m_lightSources[lightIndex].Atten_Constant = l.Attenuation.Constant;
        m_lightSources[lightIndex].Atten_Linear = l.Attenuation.Linear;
        m_lightSources[lightIndex].Atten_Exp = l.Attenuation.Exp;
        m_lightSources[lightIndex].Color = l.Color;
        m_lightSources[lightIndex].Cutoff = cosf(glm::radians(l.Cutoff));
        m_lightSources[lightIndex].DiffuseIntensity = l.DiffuseIntensity;
        glm::vec3 dir = l.WorldDirection;
        m_lightSources[lightIndex].Direction = glm::normalize(dir);
        m_lightSources[lightIndex].WorldPos = l.WorldPosition;
        lightIndex++;
    }

    for (const DirectionalLight &l : pScene->GetDirLights())
    {
        assert(lightIndex < MAX_NUM_LIGHTS);
        m_lightSources[lightIndex].LightType = LIGHT_TYPE_DIR;
        m_lightSources[lightIndex].AmbientIntensity = l.AmbientIntensity;
        m_lightSources[lightIndex].Atten_Constant = 0.0f;
        m_lightSources[lightIndex].Atten_Linear = 0.0f;
        m_lightSources[lightIndex].Atten_Exp = 0.0f;
        m_lightSources[lightIndex].Color = l.Color;
        m_lightSources[lightIndex].Cutoff = 0.0f;
        m_lightSources[lightIndex].DiffuseIntensity = l.DiffuseIntensity;
        glm::vec3 dir = l.WorldDirection;
        m_lightSources[lightIndex].Direction = glm::normalize(dir);
        m_lightSources[lightIndex].WorldPos = glm::vec3(0.0f);
        lightIndex++;
    }

    for (const PointLight &l : pScene->GetPointLights())
    {
        assert(lightIndex < MAX_NUM_LIGHTS);
        m_lightSources[lightIndex].LightType = LIGHT_TYPE_POINT;
        m_lightSources[lightIndex].AmbientIntensity = l.AmbientIntensity;
        m_lightSources[lightIndex].Atten_Constant = l.Attenuation.Constant;
        m_lightSources[lightIndex].Atten_Linear = l.Attenuation.Linear;
        m_lightSources[lightIndex].Atten_Exp = l.Attenuation.Exp;
        m_lightSources[lightIndex].Color = l.Color;
        m_lightSources[lightIndex].Cutoff = 0.0f;
        m_lightSources[lightIndex].DiffuseIntensity = l.DiffuseIntensity;
        m_lightSources[lightIndex].Direction = glm::vec3(0.0f);
        m_lightSources[lightIndex].WorldPos = l.WorldPosition;
        lightIndex++;
    }

    m_pCurLightingTech->SetNumLights(lightIndex);
}

void ForwardRenderer::PickingPass(void *pWindow, GLScene *pScene)
{
    m_curRenderPass = RENDER_PASS_PICKING;
    m_pickingTexture.EnableWriting();

    // TODO: change clear color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_pickingTech.Enable();
    PickingRenderScene(pScene);
    m_pickingTexture.DisableWriting();
    PostPickingPass(pWindow, pScene);
}

void ForwardRenderer::PickingRenderScene(GLScene *pScene)
{
    const std::list<CoreSceneObject *> &renderList = pScene->GetRenderList();

    for (std::list<CoreSceneObject *>::const_iterator it = renderList.begin(); it != renderList.end(); it++)
    {
        int objectIndex = (*it)->GetID() + 1; // background is zero, the real objects start at 1
        m_pickingTech.SetObjectIndex(objectIndex);

        m_pCurSceneObject = *it;
        RenderSingleObject(m_pCurSceneObject);
    }
}

void ForwardRenderer::PostPickingPass(void *pWindow, GLScene *pScene)
{
    int objectIndex = GetPickedObjectIndex(pWindow, pScene);

    if (objectIndex == 0)
    {
        pScene->SetPickedSceneObject(NULL);
    }
    else
    {
        SavePickedObject(pScene, objectIndex);
    }
}

int ForwardRenderer::GetPickedObjectIndex(void *pWindow, GLScene *pScene)
{
    int mousePosX = 0, mousePosY = 0;
    m_pRenderingSystemGL->GetMousePos(pWindow, mousePosX, mousePosY);
    PickingTexture::PixelInfo pixel = m_pickingTexture.ReadPixel(mousePosX, m_windowHeight - mousePosY - 1);
    return pixel.ObjectID;
}

void ForwardRenderer::SavePickedObject(GLScene *pScene, int objectIndex)
{
    const std::list<CoreSceneObject *> &renderList = pScene->GetRenderList();

    for (std::list<CoreSceneObject *>::const_iterator it = renderList.begin(); it != renderList.end(); it++)
    {
        if (((*it)->GetID() + 1) == objectIndex)
        {
            pScene->SetPickedSceneObject(*it);
            return;
        }
    }

    // should never get here
    assert(0);
}

void ForwardRenderer::ShadowMapPass(GLScene *pScene)
{
    const std::vector<SpotLight> &spotLights = pScene->GetSpotLights();
    int numSpotLights = (int)spotLights.size();

    if (numSpotLights > 0)
    {
        m_lightViewMatrix = glm::lookAt(spotLights[0].WorldPosition,
                                        spotLights[0].WorldDirection,
                                        spotLights[0].Up);
    }

    int numDirLights = (int)pScene->GetDirLights().size();

    if (numDirLights > 0)
    {
        const std::vector<DirectionalLight> &dirLights = pScene->GetDirLights();
        OrthoProjInfo lightOrthoProjInfo;
        glm::vec3 lightWorldPos;

        CalcTightLightProjection(m_pCurCamera->GetViewMatrix(),
                                 dirLights[0].WorldDirection,
                                 m_pCurCamera->GetPersProjInfo(),
                                 lightWorldPos,
                                 lightOrthoProjInfo);

        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        m_lightViewMatrix = glm::lookAt(lightWorldPos, dirLights[0].WorldDirection, up);
    }

    int numPointLights = (int)pScene->GetPointLights().size();

    if (numDirLights > 0)
    {
        m_curRenderPass = RENDER_PASS_SHADOW_DIR;
        ShadowMapPassDirAndSpot(pScene->GetRenderList());
    }
    else if (numPointLights)
    {
        m_curRenderPass = RENDER_PASS_SHADOW_POINT;
        ShadowMapPassPoint(pScene->GetRenderList(), pScene->GetPointLights());
    }
    else
    {
        m_curRenderPass = RENDER_PASS_SHADOW_SPOT;
        ShadowMapPassDirAndSpot(pScene->GetRenderList());
    }
}

void ForwardRenderer::ShadowMapPassPoint(const std::list<CoreSceneObject *> &renderList,
                                         const std::vector<PointLight> &pointLights)
{
    m_shadowMapPointLightTech.Enable();
    m_shadowMapPointLightTech.SetLightWorldPos(pointLights[0].WorldPosition);

    glClearColor(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);

    for (u_int i = 0; i < NUM_CUBE_MAP_FACES; i++)
    {
        m_shadowCubeMapFBO.BindForWriting(gCameraDirections[i].CubemapFace);
        glViewport(0, 0, SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT); // TODO: should be done in the FBO
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        m_lightViewMatrix = glm::lookAt(pointLights[0].WorldPosition,
                                        gCameraDirections[i].Target,
                                        gCameraDirections[i].Up);
        RenderEntireRenderList(renderList);
    }
}

void ForwardRenderer::ShadowMapPassDirAndSpot(const std::list<CoreSceneObject *> &renderList)
{
    m_shadowMapFBO.BindForWriting();
    glClear(GL_DEPTH_BUFFER_BIT);
    m_shadowMapTech.Enable();
    if (useIndirectRender)
    {
        glm::mat4 VP = m_lightPersProjMatrix * m_lightViewMatrix;
        m_shadowMapTech.SetVP(VP);
    }

    m_shadowMapTech.ControlIndirectRender(useIndirectRender); // TODO: same for point
    m_shadowMapTech.ControlPVP(usePVP);                       // TODO: same for point
    RenderEntireRenderList(renderList);
}

void ForwardRenderer::GBufferPass(GLScene *pScene)
{
    m_curRenderPass = RENDER_PASS_GBUFFER;

    m_ssgiFBO.BindForWriting();
    m_ssgiFBO.Clear();

    m_geometryTech.Enable();

    if (useIndirectRender)
    {
        glm::mat4 VP = m_pCurCamera->GetVPMatrix();
        m_geometryTech.SetVP(VP);
    }

    m_geometryTech.ControlIndirectRender(useIndirectRender);
    m_geometryTech.ControlPVP(usePVP);

    RenderEntireRenderList(pScene->GetRenderList());
}

void ForwardRenderer::BrightPass(GLScene *pScene, float averageLuminance)
{
    m_hdrFBO.BindColorForReading(GL_TEXTURE0);

    m_brightFilterFBO[0].BindForWriting();
    m_brightFilterFBO[0].Clear();

    m_brightFilterTech.Enable();
    float luminanceThreshold = pScene->GetConfig()->GetLuminanceThreshold();
    m_brightFilterTech.SetLuminanceThreshold(luminanceThreshold);
    m_brightFilterTech.Render();
}

void ForwardRenderer::BlurFilter1Pass(GLScene *pScene)
{
    m_brightFilterFBO[0].BindColorForReading(GL_TEXTURE0);
    m_brightFilterFBO[1].BindForWriting();
    m_brightFilterFBO[1].Clear();

    m_blurFilter1Tech.Enable();
    float sigmaV = pScene->GetConfig()->GetBloomSigmaV();
    m_blurFilter1Tech.SetSigmaV(sigmaV);
    m_blurFilter1Tech.Render();
}

void ForwardRenderer::BlurFilter2Pass(GLScene *pScene)
{
    m_brightFilterFBO[1].BindColorForReading(GL_TEXTURE0);
    m_brightFilterFBO[0].BindForWriting();
    m_brightFilterFBO[0].Clear();

    m_blurFilter2Tech.Enable();
    float sigmaH = pScene->GetConfig()->GetBloomSigmaH();
    m_blurFilter2Tech.SetSigmaH(sigmaH);
    m_blurFilter2Tech.Render();
}

void ForwardRenderer::RenderEntireRenderList(const std::list<CoreSceneObject *> &renderList)
{
    for (std::list<CoreSceneObject *>::const_iterator it = renderList.begin(); it != renderList.end(); it++)
    {
        m_pCurSceneObject = *it;
        RenderSingleObject(m_pCurSceneObject);
    }
}

void ForwardRenderer::NormalPass(GLScene *pScene)
{
    m_curRenderPass = RENDER_PASS_NORMAL;

    m_normalFBO.BindForWriting();
    m_normalFBO.Clear();

    m_normalTech.Enable();

    if (useIndirectRender)
    {
        glm::mat4 VP = m_pCurCamera->GetVPMatrix();
        m_normalTech.SetVP(VP);
    }

    m_normalTech.ControlIndirectRender(useIndirectRender);
    m_normalTech.ControlPVP(usePVP);

    RenderEntireRenderList(pScene->GetRenderList());
}

void ForwardRenderer::LightingPass(GLScene *pScene, double totalRuntime)
{
    LightingPassFBOSetup(pScene);

    if (pScene->IsClearFrame())
    {
        const glm::vec4 &clearColor = pScene->GetClearColor();
        glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    BindShadowMaps();

    m_lightParams.BindUBO(LIGHT_UBO_INDEX);

    if (pScene->GetConfig()->GetTerrainGrid())
    {
        TerrainGrid *pTerrainGrid = (TerrainGrid *)pScene->GetConfig()->GetTerrainGrid();
        m_terrainTech.Enable();
        pTerrainGrid->Render();
    }
    else if (pScene->GetConfig()->GetInfiniteGrid().Enabled)
    {
        RenderInfiniteGrid(pScene);
    }

    if (pScene->GetConfig()->IsSkyboxEnabled())
    {
        BaseCubemapTexture *pCubeMapTex = pScene->GetSkyboxTex();
        if (pCubeMapTex)
        {
            pCubeMapTex->Bind(SKYBOX_TEXTURE_UNIT);
        }
    }

    RenderObjectList(pScene, totalRuntime);
}

void ForwardRenderer::LightingPassFBOSetup(GLScene *pScene)
{
    if (pScene->GetConfig()->IsSSAOEnabled())
    {
        if (pScene->GetConfig()->IsHDREnabled())
        {
            NOT_IMPLEMENTED;
        }
        else
        {
            m_lightingFBO.BindForWriting();
        }
    }
    else if (pScene->GetConfig()->IsHDREnabled())
    {
        m_hdrFBO.BindForWriting();
    }
    else
    {
        SetRenderToDefaultFB();
    }
}

void ForwardRenderer::HDRPassGPU(float &avgLogLum, float &exposure)
{
    m_hdrFBO.BindColorForReading(GL_TEXTURE0);
    m_luminanceBuffer.BindSSBO(1);

    m_hdrTech.Enable();
    glDispatchCompute(m_hdrNumGroupsX, m_hdrNumGroupsY, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    float *p = (float *)m_luminanceBuffer.MapForReading();
    float sum = 0.0f;
    int numTiles = m_hdrNumGroupsX * m_hdrNumGroupsY;

    for (int i = 0; i < numTiles; ++i)
    {
        sum += p[i];
    }

    m_luminanceBuffer.Unmap();

    int numPixels = numTiles * 100;
    avgLogLum = sum / (float)numPixels;
    avgLogLum = expf(avgLogLum);
    exposure = 0.18f / avgLogLum;
}

void ForwardRenderer::HDRPassCPU(float &avgLogLum, float &exposure)
{
    m_hdrFBO.BindColorForReading(GL_TEXTURE0);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, m_hdrData.data());

    float sum = 0.0f;
    int size = m_windowWidth * m_windowHeight;

    for (int i = 0; i < size; i++)
    {
        glm::vec3 pixel(m_hdrData[i * 3 + 0],
                        m_hdrData[i * 3 + 1],
                        m_hdrData[i * 3 + 2]);

        float lum = glm::dot(pixel, glm::vec3(0.2126f, 0.7152f, 0.0722f));
        sum += logf(lum + 0.0001f);
    }

    avgLogLum = sum / size;
    avgLogLum = expf(avgLogLum);
    exposure = 0.18f / avgLogLum;
}

void ForwardRenderer::BindShadowMaps()
{
    switch (m_curRenderPass)
    {
    case RENDER_PASS_SHADOW_DIR:
        m_shadowMapFBO.BindDepthForReading(SHADOW_TEXTURE_UNIT);
        m_curRenderPass = RENDER_PASS_LIGHTING_DIR;
        break;

    case RENDER_PASS_SHADOW_SPOT:
        m_shadowMapFBO.BindDepthForReading(SHADOW_TEXTURE_UNIT);
        m_curRenderPass = RENDER_PASS_LIGHTING_SPOT;
        break;

    case RENDER_PASS_SHADOW_POINT:
        m_shadowCubeMapFBO.BindForReading(SHADOW_CUBE_MAP_TEXTURE_UNIT);
        m_curRenderPass = RENDER_PASS_LIGHTING_POINT;
        break;

    default:
        // TODO: handle the case when there is no shadow pass
        assert(0);
    }
}

void ForwardRenderer::RenderObjectList(GLScene *pScene, double totalRuntime)
{
    bool firstTimeForwardLighting = true;
    const std::list<CoreSceneObject *> &renderList = pScene->GetRenderList();

    for (std::list<CoreSceneObject *>::const_iterator it = renderList.begin(); it != renderList.end(); it++)
    {
        m_pCurSceneObject = *it;

        if (firstTimeForwardLighting)
        {
            StartRenderWithForwardLighting(pScene, m_pCurSceneObject, totalRuntime);
            // firstTimeForwardLighting = false; TODO: currently disabled
        }

        RenderWithForwardLighting(pScene, m_pCurSceneObject, totalRuntime);
    }
}

void ForwardRenderer::StartRenderWithForwardLighting(GLScene *pScene,
                                                     CoreSceneObject *pSceneObject,
                                                     double totalRuntime)
{
    LIGHTING_TECHNIQUE lightingTech = GetLightingTech(pScene, pSceneObject->GetModel());

    SwitchToLightingTech(lightingTech);
    ApplySceneConfig(pScene);
    UpdateLightSources(pScene);

    if (useIndirectRender)
    {
        glm::mat4 VP = m_pCurCamera->GetVPMatrix();
        m_pCurLightingTech->SetVP(VP);
        glm::mat4 lightVP = m_lightPersProjMatrix * m_lightViewMatrix; // TODO: get the correct projection matrix
        m_pCurLightingTech->SetLightVP(lightVP);
    }

    m_pCurLightingTech->ControlIndirectRender(useIndirectRender);
    m_pCurLightingTech->ControlPVP(usePVP);
    m_pCurLightingTech->SetCameraWorldPos(m_pCurCamera->GetPosition());
}

void ForwardRenderer::RenderWithForwardLighting(GLScene *pScene,
                                                CoreSceneObject *pSceneObject,
                                                double totalRuntime)
{
    LIGHTING_TECHNIQUE lightingTech = GetLightingTech(pScene, pSceneObject->GetModel());
    SwitchToLightingTech(lightingTech);

    if (pSceneObject->GetModel()->IsAnimated())
    {
        float animationTimeSec = (float)totalRuntime;
        int animationIndex = 0;
        std::vector<glm::mat4> transforms;
        pSceneObject->GetModel()->GetBoneTransforms(animationTimeSec, transforms, animationIndex);

        for (u_int i = 0; i < transforms.size(); i++)
        {
            if (lightingTech == PBR_GLTF2_SKINNING)
            {
                m_pbrSkinningTech.SetBoneTransform(i, transforms[i]);
            }
            else
            {
                m_skinningTech.SetBoneTransform(i, transforms[i]);
            }
        }
    }

    GLModel *pModel = (GLModel *)pSceneObject->GetModel();
    bool normalMapEnabled = pModel->GetNormalMap() != NULL;
    bool heightMapEnabled = pModel->GetHeightMap() != NULL;

    m_pCurLightingTech->ControlNormalMap(normalMapEnabled);
    m_pCurLightingTech->ControlParallaxMap(heightMapEnabled);

    const glm::vec4 &flatColor = m_pCurSceneObject->GetFlatColor();

    if (flatColor.x == -1.0f)
    {
        m_pCurLightingTech->SetColorMod(glm::vec4(pSceneObject->GetColorMod(), 1.0f));
    }
    else
    {
        m_pCurLightingTech->SetColorMod(glm::vec4(0.0f));
        m_pCurLightingTech->SetColorAdd(flatColor);
    }

    bool isCubeMapping = pSceneObject->IsCubeMapping();
    m_pCurLightingTech->ControlCubemapping(isCubeMapping);
    int cubeMipmapLevel = pSceneObject->GetCubeMipmapLevel();
    m_pCurLightingTech->SetCubeMipmapLevel(cubeMipmapLevel);

    RenderSingleObject(pSceneObject);
}

void ForwardRenderer::RenderSingleObject(CoreSceneObject *pSceneObject)
{
    GLModel *pModel = (GLModel *)pSceneObject->GetModel();

    if (useIndirectRender)
    {
        glm::mat4 objectMatrix = m_pCurSceneObject->GetMatrix();
        glm::mat4 globalWorldRotation = m_pCurCamera->GetGlobalWorldRotation();
        objectMatrix = objectMatrix * globalWorldRotation;
        pModel->RenderIndirect(objectMatrix);
    }
    else
    {
        pModel->Render(this);
    }
}

void ForwardRenderer::SetRenderToDefaultFB()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_windowWidth, m_windowHeight);
}

void ForwardRenderer::RenderInfiniteGrid(GLScene *pScene)
{
    glm::mat4 VP = m_pCurCamera->GetVPMatrix();
    glm::mat4 lightVP = m_lightPersProjMatrix * m_lightViewMatrix;

    int numSpotLights = (int)pScene->GetSpotLights().size();
    glm::vec3 lightDir = glm::vec3(0.0f, -1.0f, 0.0f);

    if (numSpotLights > 0)
    {
        lightDir = pScene->GetSpotLights()[0].WorldDirection;
    }

    InfiniteGridConfig &config = pScene->GetConfig()->GetInfiniteGrid();
    config.ShadowsEnabled = pScene->GetConfig()->IsShadowMappingEnabled();
    m_infiniteGrid.Render(config, VP, m_pCurCamera->GetPosition(), lightVP, lightDir);
}

void ForwardRenderer::SSAOPass(GLScene *pScene)
{
    glDisable(GL_DEPTH_TEST);

    m_ssaoFBO.BindForWriting();
    m_ssaoFBO.ClearColorBuffer(glm::vec4(0.0f));
    m_ssaoParams.BindUBO(SSAO_UBO_INDEX);

    SSAOParamsInternal params;
    params.params = pScene->GetConfig()->GetSSAOParams();
    params.zNear = m_pCurCamera->GetPersProjInfo().zNear;
    params.zFar = m_pCurCamera->GetPersProjInfo().zFar;
    params.OutputHeight = (float)m_ssaoFBO.GetHeight();
    params.OutputWidth = (float)m_ssaoFBO.GetWidth();

    m_ssaoParams.Update(&params, sizeof(SSAOParamsInternal));

    m_lightingFBO.BindDepthForReading(GL_TEXTURE0);
    m_ssaoRotTexture.Bind(GL_TEXTURE1);
    m_normalFBO.BindColorForReading(GL_TEXTURE2);
    m_ssaoTech.Enable();
    m_ssaoTech.Render();

    glEnable(GL_DEPTH_TEST);
}

void ForwardRenderer::SSGIPass(GLScene *pScene)
{
    glDisable(GL_DEPTH_TEST);
    SetRenderToDefaultFB();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_ssgiFBO.BindColorForReading(COLOR_TEXTURE_UNIT);
    m_ssgiFBO.BindNormalForReading(NORMAL_TEXTURE_UNIT);
    m_ssgiFBO.BindDepthForReading(DEPTH_TEXTURE_UNIT);

    m_ssgiTech.Enable();
    glm::mat4 proj = m_pCurCamera->GetProjectionMatrix();
    m_ssgiTech.SetProj(proj);
    glm::mat4 view = m_pCurCamera->GetViewMatrix();
    m_ssgiTech.SetViewTransform(view);
    m_ssgiTech.Render();

    glEnable(GL_DEPTH_TEST);
}

void ForwardRenderer::SSAOCombinePass()
{
    SetRenderToDefaultFB();
    m_lightingFBO.BindColorForReading(GL_TEXTURE0);
    m_ssaoFBO.BindColorForReading(GL_TEXTURE1);
    m_ssaoCombineTech.Enable();
    m_ssaoCombineTech.Render();
}

void ForwardRenderer::ToneMappingPass(GLScene *pScene, float averageLuminance, float exposure)
{
    SetRenderToDefaultFB();

    m_hdrFBO.BindColorForReading(GL_TEXTURE0);
    m_luminanceBuffer.BindSSBO(1);

    bool isBloom = pScene->GetConfig()->IsBloomEnabled();
    TONE_MAP_METHOD toneMapMethod = pScene->GetConfig()->GetToneMapMethod();
    bool isGamma = pScene->GetConfig()->IsGammaCorrectionEnabled();

    if (isBloom)
    {
        m_brightFilterFBO[0].BindColorForReading(GL_TEXTURE1);
        m_toneMapTechWithBloom.Enable();
        m_toneMapTechWithBloom.SetAverageLuminance(averageLuminance);
        m_toneMapTechWithBloom.SetExposure(exposure);
        m_toneMapTechWithBloom.SetToneMapMethod(toneMapMethod);
        m_toneMapTechWithBloom.ControlGammaCorrection(isGamma);
        float bloomStrength = pScene->GetConfig()->GetBloomStrength();
        m_toneMapTechWithBloom.SetBloomStrength(bloomStrength);
        m_toneMapTechWithBloom.Render();
    }
    else
    {
        m_toneMapTech.Enable();
        m_toneMapTech.SetAverageLuminance(averageLuminance);
        m_toneMapTech.SetExposure(exposure);
        m_toneMapTech.SetToneMapMethod(toneMapMethod);
        m_toneMapTech.ControlGammaCorrection(isGamma);
        m_toneMapTech.Render();
    }
}

void ForwardRenderer::FullScreenQuadBlit(GLScene *pScene)
{
    SetRenderToDefaultFB();
    glClear(GL_DEPTH_BUFFER_BIT);

    if (pScene->GetConfig()->IsHDREnabled())
    {
        m_hdrFBO.BindColorForReading(GL_TEXTURE0);
    }
    else
    {
        m_lightingFBO.BindColorForReading(GL_TEXTURE0);
    }

    m_fullScreenTech.Render();
}

void ForwardRenderer::GetWVP(CoreSceneObject *pSceneObject, glm::mat4 &WVP)
{
    glm::mat4 globalWorldRotation = m_pCurCamera->GetGlobalWorldRotation();
    glm::mat4 objectMatrix = pSceneObject->GetMatrix();
    glm::mat4 world = objectMatrix * globalWorldRotation;
    glm::mat4 view = m_pCurCamera->GetViewMatrix();
    glm::mat4 projection = m_pCurCamera->GetProjectionMatrix();

    WVP = projection * view * world;
}

void ForwardRenderer::DrawStartCB(u_int drawIndex)
{
    // TODO: picking technique update
}

void ForwardRenderer::ControlSpecularExponentCB(bool isEnabled)
{
    switch (m_curRenderPass)
    {
    case RENDER_PASS_LIGHTING_DIR:
    case RENDER_PASS_LIGHTING_SPOT:
    case RENDER_PASS_LIGHTING_POINT:
        if (m_pCurLightingTech)
        {
            m_pCurLightingTech->ControlSpecularExponent(isEnabled);
        }
        break;
    }
}

void ForwardRenderer::SetMaterialCB(const CoreMaterial &material)
{
    switch (m_curRenderPass)
    {
    case RENDER_PASS_LIGHTING_DIR:
    case RENDER_PASS_LIGHTING_SPOT:
    case RENDER_PASS_LIGHTING_POINT:
        m_pCurLightingTech->SetMaterial(material);
        break;
    }
}

void ForwardRenderer::SetWorldMatrixCB(const glm::mat4 &world)
{
    if (useIndirectRender)
    {
        printf("Incorrect callback with indirect rendering\n");
        assert(1);
    }

    switch (m_curRenderPass)
    {
    case RENDER_PASS_SHADOW_DIR:
        SetWorldMatrix_CB_ShadowPassDir(world);
        break;

    case RENDER_PASS_SHADOW_SPOT:
        SetWorldMatrix_CB_ShadowPassSpot(world);
        break;

    case RENDER_PASS_SHADOW_POINT:
        SetWorldMatrix_CB_ShadowPassPoint(world);
        break;

    case RENDER_PASS_LIGHTING_DIR:
    case RENDER_PASS_LIGHTING_SPOT:
    case RENDER_PASS_LIGHTING_POINT:
        SetWorldMatrix_CB_LightingPass(world);
        break;

    case RENDER_PASS_PICKING:
        SetWorldMatrix_CB_PickingPass(world);
        break;

    case RENDER_PASS_NORMAL:
        SetWorldMatrix_CB_NormalPass(world);
        break;

    case RENDER_PASS_GBUFFER:
        SetWorldMatrix_CB_GBufferPass(world);
        break;

    default:
        printf("%s:%d - Unknown render pass %d\n", __FILE__, __LINE__, m_curRenderPass);
        exit(1);
    }
}

void ForwardRenderer::SetWorldMatrix_CB_ShadowPassDir(const glm::mat4 &world)
{
    glm::mat4 objectMatrix = m_pCurSceneObject->GetMatrix();
    glm::mat4 globalWorldRotation = m_pCurCamera->GetGlobalWorldRotation();
    glm::mat4 WVP = m_lightOrthoProjMatrix * m_lightViewMatrix * objectMatrix * world * globalWorldRotation;
    m_shadowMapTech.SetWVP(WVP);
}

void ForwardRenderer::SetWorldMatrix_CB_ShadowPassSpot(const glm::mat4 &world)
{
    glm::mat4 globalWorldRotation = m_pCurCamera->GetGlobalWorldRotation();
    glm::mat4 objectMatrix = m_pCurSceneObject->GetMatrix();
    glm::mat4 WVP = m_lightPersProjMatrix * m_lightViewMatrix * objectMatrix * world * globalWorldRotation;
    m_shadowMapTech.SetWVP(WVP);
}

void ForwardRenderer::SetWorldMatrix_CB_ShadowPassPoint(const glm::mat4 &world)
{
    glm::mat4 objectMatrix = m_pCurSceneObject->GetMatrix();
    glm::mat4 WVP = m_lightPersProjMatrix * m_lightViewMatrix * objectMatrix * world;
    m_shadowMapPointLightTech.SetWorld(world);
    m_shadowMapPointLightTech.SetWVP(WVP);
}

static glm::mat3 CalcNormalMatrix(const glm::mat4 &world)
{
    return glm::transpose(glm::inverse(glm::mat3(world)));
}

void ForwardRenderer::SetWorldMatrix_CB_LightingPass(const glm::mat4 &world)
{
    // TODO: use GetWVP instead
    glm::mat4 globalWorldRotation = m_pCurCamera->GetGlobalWorldRotation();
    glm::mat4 objectMatrix = m_pCurSceneObject->GetMatrix();
    glm::mat4 finalWorldMatrix = objectMatrix * world * globalWorldRotation;
    m_pCurLightingTech->SetWorldMatrix(finalWorldMatrix);

    glm::mat4 VP = m_pCurCamera->GetVPMatrix();
    glm::mat4 WVP = VP * finalWorldMatrix;
    m_pCurLightingTech->SetWVP(WVP);

    glm::mat4 lightWVP;

    switch (m_curRenderPass)
    {
    case RENDER_PASS_LIGHTING_DIR:
        lightWVP = m_lightOrthoProjMatrix * m_lightViewMatrix * finalWorldMatrix;
        break;

    case RENDER_PASS_LIGHTING_SPOT:
    case RENDER_PASS_LIGHTING_POINT:
        lightWVP = m_lightPersProjMatrix * m_lightViewMatrix * finalWorldMatrix;
        break;

    default:
        assert(0);
    }

    m_pCurLightingTech->SetLightWVP(lightWVP);
    glm::mat3 normalMatrix = CalcNormalMatrix(finalWorldMatrix);
    m_pCurLightingTech->SetNormalMatrix(normalMatrix);
}

void ForwardRenderer::SetWorldMatrix_CB_NormalPass(const glm::mat4 &world)
{
    // TODO: use GetWVP instead
    glm::mat4 objectMatrix = m_pCurSceneObject->GetMatrix();
    glm::mat4 globalWorldRotation = m_pCurCamera->GetGlobalWorldRotation();
    glm::mat4 finalWorldMatrix = objectMatrix * world * globalWorldRotation;
    glm::mat4 VP = m_pCurCamera->GetVPMatrix();
    glm::mat4 WVP = VP * finalWorldMatrix;
    m_normalTech.SetWVP(WVP);

    glm::mat3 normalMatrix = CalcNormalMatrix(finalWorldMatrix);
    glm::mat4 view = m_pCurCamera->GetViewMatrix();
    glm::mat4 viewNormalMatrix4D = view * glm::mat4(normalMatrix);
    glm::mat3 viewNormalMatrix = glm::mat3(viewNormalMatrix4D);
    m_normalTech.SetNormalMatrix(viewNormalMatrix);
}

void ForwardRenderer::SetWorldMatrix_CB_GBufferPass(const glm::mat4 &world)
{
    // TODO: use GetWVP instead
    glm::mat4 globalWorldRotation = m_pCurCamera->GetGlobalWorldRotation();
    glm::mat4 objectMatrix = m_pCurSceneObject->GetMatrix();
    glm::mat4 finalWorldMatrix = objectMatrix * world * globalWorldRotation;
    glm::mat4 view = m_pCurCamera->GetViewMatrix(); // TODO: use VP matrix from camera
    glm::mat4 projection = m_pCurCamera->GetProjectionMatrix();
    glm::mat4 WVP = projection * view * finalWorldMatrix;
    m_geometryTech.SetWVP(WVP);

    glm::mat3 normalMatrix = CalcNormalMatrix(finalWorldMatrix);
    glm::mat4 viewNormalMatrix4D = view * glm::mat4(normalMatrix);
    glm::mat3 viewNormalMatrix = glm::mat3(viewNormalMatrix4D);
    m_geometryTech.SetNormalMatrix(viewNormalMatrix);
}

void ForwardRenderer::SetWorldMatrix_CB_PickingPass(const glm::mat4 &world)
{
    glm::mat4 globalWorldRotation = m_pCurCamera->GetGlobalWorldRotation();
    glm::mat4 objectMatrix = m_pCurSceneObject->GetMatrix();
    glm::mat4 VP = GetViewProjectionMatrix();
    glm::mat4 WVP = VP * objectMatrix * world * globalWorldRotation;
    m_pickingTech.SetWVP(WVP);
}

// TODO: replace with GetVP from camera
glm::mat4 ForwardRenderer::GetViewProjectionMatrix()
{
    glm::mat4 view = m_pCurCamera->GetViewMatrix();
    glm::mat4 projection = m_pCurCamera->GetProjectionMatrix();
    return projection * view;
}
