#include <vector>
#include "renderer.h"

void PhongRenderer::InitPhongRenderer(int SubTech)
{
    m_subTech = SubTech;

    if (!m_lightingTech.Init(SubTech))
    {
        printf("Error initializing the lighting technique.");
        exit(1);
    }

    m_lightingTech.Enable();
    m_lightingTech.SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetAlbedoTextureUnit(ALBEDO_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetRoughnessTextureUnit(ROUGHNESS_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetMetallicTextureUnit(METALLIC_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetNormalTextureUnit(NORMAL_TEXTURE_UNIT_INDEX);

    if (!m_skinningTech.Init())
    {
        printf("Error initializing the skinning technique\n");
        exit(1);
    }

    m_skinningTech.Enable();
    m_skinningTech.SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    m_skinningTech.SetAlbedoTextureUnit(ALBEDO_TEXTURE_UNIT_INDEX);
    m_skinningTech.SetRoughnessTextureUnit(ROUGHNESS_TEXTURE_UNIT_INDEX);
    m_skinningTech.SetMetallicTextureUnit(METALLIC_TEXTURE_UNIT_INDEX);
    m_skinningTech.SetNormalTextureUnit(NORMAL_TEXTURE_UNIT_INDEX);

    if (!m_shadowMapTech.Init())
    {
        printf("Error initializing the shadow mapping technique\n");
        exit(1);
    }

    glUseProgram(0);
}

void PhongRenderer::StartShadowPass()
{
    m_shadowMapTech.Enable();
}

void PhongRenderer::SwitchToLightingTech()
{
    GLint cur_prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &cur_prog);

    if (cur_prog != m_lightingTech.GetProgram())
    {
        m_lightingTech.Enable();
    }
}

void PhongRenderer::SwitchToSkinningTech()
{
    GLint cur_prog = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &cur_prog);

    if (cur_prog != m_skinningTech.GetProgram())
    {
        m_skinningTech.Enable();
    }
}

void PhongRenderer::SetDirLight(const DirectionalLight &DirLight)
{
    m_dirLight = DirLight;

    SwitchToLightingTech();
    m_lightingTech.SetDirectionalLight(m_dirLight, false);

    SwitchToSkinningTech();
    m_skinningTech.SetDirectionalLight(m_dirLight, false);
}

void PhongRenderer::SetPointLights(u_int NumLights, const PointLight *pPointLights)
{
    if (!pPointLights || (NumLights == 0))
    {
        m_numPointLights = 0;
        return;
    }

    if (NumLights > LightingTechnique::MAX_POINT_LIGHTS)
    {
        printf("Number of point lights (%d) exceeds max (%d)\n", NumLights, LightingTechnique::MAX_POINT_LIGHTS);
        exit(0);
    }

    for (u_int i = 0; i < NumLights; i++)
    {
        m_pointLights[i] = pPointLights[i];
    }

    m_numPointLights = NumLights;

    SwitchToLightingTech();
    m_lightingTech.SetPointLights(NumLights, pPointLights, false);

    SwitchToSkinningTech();
    m_skinningTech.SetPointLights(NumLights, pPointLights, false);
}

void PhongRenderer::SetSpotLights(u_int NumLights, const SpotLight *pSpotLights)
{
    if (!pSpotLights || (NumLights == 0))
    {
        m_numSpotLights = 0;
        return;
    }

    if (NumLights > LightingTechnique::MAX_SPOT_LIGHTS)
    {
        printf("Number of spot lights (%d) exceeds max (%d)\n", NumLights, LightingTechnique::MAX_SPOT_LIGHTS);
        exit(0);
    }

    for (u_int i = 0; i < NumLights; i++)
    {
        m_spotLights[i] = pSpotLights[i];
    }

    m_numSpotLights = NumLights;

    SwitchToLightingTech();
    m_lightingTech.SetSpotLights(NumLights, pSpotLights, false);

    SwitchToSkinningTech();
    m_skinningTech.SetSpotLights(NumLights, pSpotLights, false);
}

void PhongRenderer::UpdateDirLightDir(const glm::vec3 &WorldDir)
{
    m_dirLight.WorldDirection = WorldDir;
}

void PhongRenderer::UpdatePointLightPos(u_int Index, const glm::vec3 &WorldPos)
{
    if (Index > m_numPointLights)
    {
        printf("Trying to update point light %d while total number of lights is %d\n", Index, m_numPointLights);
        exit(0);
    }

    m_pointLights[Index].WorldPosition = WorldPos;
}

void PhongRenderer::UpdateSpotLightPosAndDir(u_int Index, const glm::vec3 &WorldPos, const glm::vec3 &WorldDir)
{
    if (Index > m_numSpotLights)
    {
        printf("Trying to update spot light %d while total number of lights is %d\n", Index, m_numSpotLights);
        exit(0);
    }

    m_spotLights[Index].WorldPosition = WorldPos;
    m_spotLights[Index].WorldDirection = WorldDir;
}

void PhongRenderer::Render(BasicMesh *pMesh)
{
    if (!m_pCamera)
    {
        printf("PhongRenderer: camera not initialized\n");
        exit(0);
    }

    if ((m_numPointLights == 0) && (m_numSpotLights == 0) && m_dirLight.IsZero())
    {
        printf("Warning! Trying to render but all lights are zero\n");
    }

    SwitchToLightingTech();
    glm::mat4 WVP;
    GetWVP(pMesh, WVP);
    m_lightingTech.SetWVP(WVP);

    RefreshLightingPosAndDirs(pMesh);

    if (m_dirLight.DiffuseIntensity > 0.0f)
    {
        m_lightingTech.SetDirectionalLight(m_dirLight);
    }

    m_lightingTech.UpdatePointLightsPos(m_numPointLights, m_pointLights);
    m_lightingTech.UpdateSpotLightsPosAndDir(m_numSpotLights, m_spotLights);
    m_lightingTech.SetMaterial(pMesh->GetMaterial());

    if (m_isPBR)
    {
        m_lightingTech.SetPBR(true);
        m_lightingTech.SetPBRMaterial(pMesh->GetPBRMaterial());
    }
    else
    {
        m_lightingTech.SetPBR(false);
    }

    glm::vec3 CameraLocalPos3f = pMesh->GetWorldTransform().WorldPosToLocalPos(m_pCamera->GetPos());
    m_lightingTech.SetCameraLocalPos(CameraLocalPos3f);

    m_lightingTech.SetCameraWorldPos(m_pCamera->GetPos());

    glm::mat4 World = pMesh->GetWorldTransform().GetMatrix();
    m_lightingTech.SetWorldMatrix(World);

    if (m_subTech == LightingTechnique::SUBTECH_WIREFRAME_ON_MESH)
    {
        m_lightingTech.SetViewportMatrix(m_pCamera->GetViewportMatrix());
    }

    pMesh->Render();
}

void PhongRenderer::RenderAnimation(SkinnedMesh *pMesh, float AnimationTimeSec, int AnimationIndex)
{
    RenderAnimationCommon(pMesh);

    std::vector<glm::mat4> Transforms;
    pMesh->GetBoneTransforms(AnimationTimeSec, Transforms, AnimationIndex);

    for (u_int i = 0; i < Transforms.size(); i++)
    {
        m_skinningTech.SetBoneTransform(i, Transforms[i]);
    }

    pMesh->Render();
}

void PhongRenderer::RenderAnimationBlended(SkinnedMesh *pMesh, float AnimationTimeSec,
                                           int StartAnimIndex, int EndAnimIndex, float BlendFactor)
{
    RenderAnimationCommon(pMesh);

    std::vector<glm::mat4> Transforms;
    pMesh->GetBoneTransformsBlended(AnimationTimeSec, Transforms, StartAnimIndex, EndAnimIndex, BlendFactor);

    for (u_int i = 0; i < Transforms.size(); i++)
    {
        m_skinningTech.SetBoneTransform(i, Transforms[i]);
    }

    pMesh->Render();
}

void PhongRenderer::RenderAnimationCommon(SkinnedMesh *pMesh)
{
    if (!m_pCamera)
    {
        printf("PhongRenderer: camera not initialized\n");
        exit(0);
    }

    if ((m_numPointLights == 0) && (m_numSpotLights == 0) && m_dirLight.IsZero())
    {
        printf("Warning! Trying to render but all lights are zero\n");
    }

    SwitchToSkinningTech();

    glm::mat4 WVP;
    GetWVP(pMesh, WVP);
    m_skinningTech.SetWVP(WVP);

    RefreshLightingPosAndDirs(pMesh);

    if (m_dirLight.DiffuseIntensity > 0.0f)
    {
        m_skinningTech.UpdateDirLightDirection(m_dirLight);
    }

    m_skinningTech.UpdatePointLightsPos(m_numPointLights, m_pointLights);
    m_skinningTech.UpdateSpotLightsPosAndDir(m_numSpotLights, m_spotLights);
    m_skinningTech.SetMaterial(pMesh->GetMaterial());

    if (m_isPBR)
    {
        m_skinningTech.SetPBR(true);
        m_skinningTech.SetPBRMaterial(pMesh->GetPBRMaterial());
    }
    else
    {
        m_skinningTech.SetPBR(false);
    }

    glm::vec3 CameraLocalPos3f = pMesh->GetWorldTransform().WorldPosToLocalPos(m_pCamera->GetPos());
    m_skinningTech.SetCameraLocalPos(CameraLocalPos3f);
    m_skinningTech.SetCameraWorldPos(m_pCamera->GetPos());

    glm::mat4 World = pMesh->GetWorldTransform().GetMatrix();
    m_skinningTech.SetWorldMatrix(World);
}

void PhongRenderer::RenderToShadowMap(BasicMesh *pMesh, const SpotLight &SpotLight)
{
    glm::mat4 World = pMesh->GetWorldTransform().GetMatrix();
    glm::vec3 Up(0.0f, 1.0f, 0.0f);
    glm::mat4 View = glm::lookAt(
        SpotLight.WorldPosition,
        SpotLight.WorldPosition + SpotLight.WorldDirection,
        Up);

    float FOV = 45.0f;
    float zNear = 0.1f;
    float zFar = 100.0f;
    float ar = 1000.0f / 1000.0f;
    glm::mat4 Projection = glm::perspective(glm::radians(FOV), ar, zNear, zFar);

    glm::mat4 WVP = Projection * View * World;
    m_shadowMapTech.SetWVP(WVP);
    pMesh->Render();
}

void PhongRenderer::RefreshLightingPosAndDirs(BasicMesh *pMesh)
{
    WorldTrans &meshWorldTransform = pMesh->GetWorldTransform();

    if (m_dirLight.DiffuseIntensity > 0.0f)
    {
        m_dirLight.CalcLocalDirection(meshWorldTransform);
    }

    for (u_int i = 0; i < m_numPointLights; i++)
    {
        m_pointLights[i].CalcLocalPosition(meshWorldTransform);
    }

    for (u_int i = 0; i < m_numSpotLights; i++)
    {
        m_spotLights[i].CalcLocalDirectionAndPosition(meshWorldTransform);
    }
}

void PhongRenderer::GetWVP(BasicMesh *pMesh, glm::mat4 &WVP)
{
    WorldTrans &meshWorldTransform = pMesh->GetWorldTransform();

    glm::mat4 World = meshWorldTransform.GetMatrix();
    glm::mat4 View = m_pCamera->GetMatrix();
    glm::mat4 Projection = m_pCamera->GetProjectionMat();

    WVP = Projection * View * World;
}

void PhongRenderer::ControlRimLight(bool IsEnabled)
{
    SwitchToLightingTech();
    m_lightingTech.ControlRimLight(IsEnabled);

    SwitchToSkinningTech();
    m_skinningTech.ControlRimLight(IsEnabled);
}

void PhongRenderer::ControlCellShading(bool IsEnabled)
{
    SwitchToLightingTech();
    m_lightingTech.ControlCellShading(IsEnabled);

    SwitchToSkinningTech();
    m_skinningTech.ControlCellShading(IsEnabled);
}

void PhongRenderer::SetLinearFog(float FogStart, float FogEnd, const glm::vec3 &FogColor)
{
    SwitchToLightingTech();
    m_lightingTech.SetLinearFog(FogStart, FogEnd);
    m_lightingTech.SetFogColor(FogColor);

    SwitchToSkinningTech();
    m_skinningTech.SetLinearFog(FogStart, FogEnd);
    m_skinningTech.SetFogColor(FogColor);
}

void PhongRenderer::SetExpFog(float FogEnd, const glm::vec3 &FogColor, float FogDensity)
{
    SwitchToLightingTech();
    m_lightingTech.SetExpFog(FogEnd, FogDensity);
    m_lightingTech.SetFogColor(FogColor);

    SwitchToSkinningTech();
    m_skinningTech.SetExpFog(FogEnd, FogDensity);
    m_skinningTech.SetFogColor(FogColor);
}

void PhongRenderer::SetExpSquaredFog(float FogEnd, const glm::vec3 &FogColor, float FogDensity)
{
    SwitchToLightingTech();
    m_lightingTech.SetExpSquaredFog(FogEnd, FogDensity);
    m_lightingTech.SetFogColor(FogColor);

    SwitchToSkinningTech();
    m_skinningTech.SetExpSquaredFog(FogEnd, FogDensity);
    m_skinningTech.SetFogColor(FogColor);
}

void PhongRenderer::SetLayeredFog(float FogTop, float FogEnd, const glm::vec3 &FogColor)
{
    SwitchToLightingTech();
    m_lightingTech.SetLayeredFog(FogTop, FogEnd);
    m_lightingTech.SetFogColor(FogColor);

    SwitchToSkinningTech();
    m_skinningTech.SetLayeredFog(FogTop, FogEnd);
    m_skinningTech.SetFogColor(FogColor);
}

void PhongRenderer::SetAnimatedFog(float FogEnd, float FogDensity, const glm::vec3 &FogColor)
{
    SwitchToLightingTech();
    m_lightingTech.SetAnimatedFog(FogEnd, FogDensity);
    m_lightingTech.SetFogColor(FogColor);

    SwitchToSkinningTech();
    m_skinningTech.SetAnimatedFog(FogEnd, FogDensity);
    m_skinningTech.SetFogColor(FogColor);
}

void PhongRenderer::UpdateAnimatedFogTime(float FogTime)
{
    SwitchToLightingTech();
    m_lightingTech.SetFogTime(FogTime);

    SwitchToSkinningTech();
    m_skinningTech.SetFogTime(FogTime);
}

void PhongRenderer::DisableFog()
{
    SwitchToLightingTech();
    m_lightingTech.SetFogColor(glm::vec3(0.0f));

    SwitchToSkinningTech();
    m_skinningTech.SetFogColor(glm::vec3(0.0f));
}

void PhongRenderer::SetPBR(bool IsPBR)
{
    m_isPBR = IsPBR;

    SwitchToLightingTech();
    m_lightingTech.SetPBR(IsPBR);

    SwitchToSkinningTech();
    m_skinningTech.SetPBR(IsPBR);
}

void PhongRenderer::SetWireframeLineWidth(float Width)
{
    SwitchToLightingTech();
    m_lightingTech.SetWireframeWidth(Width);
}

void PhongRenderer::SetWireframeColor(const glm::vec4 &Color)
{
    SwitchToLightingTech();
    m_lightingTech.SetWireframeColor(Color);
}
