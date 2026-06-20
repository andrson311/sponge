#pragma once

#include "camera_api.h"
#include "lighting_technique.h"
#include "skinning_technique.h"
#include "shadow_mapping_technique.h"
#include "basic_mesh.h"
#include "skinned_mesh.h"

class PhongRenderer
{
public:
    PhongRenderer() {};
    ~PhongRenderer() {};

    void InitPhongRenderer(int SubTech = LightingTechnique::SUBTECH_DEFAULT);
    void StartShadowPass();
    void SetCamera(const CameraAPI *pCamera) { m_pCamera = pCamera; }
    void SetPBR(bool IsPBR);

    // Lighting
    void SetDirLight(const DirectionalLight &DirLight);
    void SetPointLights(u_int NumLights, const PointLight *pPointLights);
    void SetSpotLights(u_int NumLights, const SpotLight *pSpotLights);
    void UpdateDirLightDir(const glm::vec3 &WorldDir);
    void UpdatePointLightPos(u_int Index, const glm::vec3 &WorldPos);
    void UpdateSpotLightPosAndDir(u_int Index, const glm::vec3 &WorldPos, const glm::vec3 &WorldDir);

    // Fog
    void SetLinearFog(float FogStart, float FogEnd, const glm::vec3 &FogColor);
    void SetExpFog(float FogEnd, const glm::vec3 &FogColor, float FogDensity);
    void SetExpSquaredFog(float FogEnd, const glm::vec3 &FogColor, float FogDensity);
    void SetLayeredFog(float FogTop, float FogEnd, const glm::vec3 &FogColor);
    void SetAnimatedFog(float FogEnd, float FogDensity, const glm::vec3 &FogColor);
    void UpdateAnimatedFogTime(float FogTime);
    void DisableFog();

    // Lighting Effects
    void ControlRimLight(bool IsEnabled);
    void ControlCellShading(bool IsEnabled);

    // Debugging
    void SetWireframeLineWidth(float Width);
    void SetWireframeColor(const glm::vec4 &Color);

    // Rendering objects
    void Render(BasicMesh* pMesh);
    void RenderAnimation(SkinnedMesh* pMesh, float AnimationTimeSec, int AnimationIndex = 0);
    void RenderAnimationBlended(SkinnedMesh* pMesh, 
        float AnimationTimeSec, int StartAnimIndex, int EndAnimIndex, float BlendFactor);
    void RenderToShadowMap(BasicMesh* pMesh, const SpotLight& SpotLight);

private:
    
    void GetWVP(BasicMesh* pMesh, glm::mat4& WVP);

    // Techniques
    void SwitchToLightingTech();
    void SwitchToSkinningTech();

    // Rendering
    void RefreshLightingPosAndDirs(BasicMesh* pMesh);
    void RenderAnimationCommon(SkinnedMesh* pMesh);

    const CameraAPI *m_pCamera = NULL;
    int m_subTech = LightingTechnique::SUBTECH_DEFAULT;

    // Rendering techniques
    LightingTechnique m_lightingTech;
    SkinningTechnique m_skinningTech;
    ShadowMappingTechnique m_shadowMapTech;
    
    // Lighting info
    DirectionalLight m_dirLight;
    u_int m_numPointLights = 0;
    PointLight m_pointLights[LightingTechnique::MAX_POINT_LIGHTS];
    u_int m_numSpotLights = 0;
    SpotLight m_spotLights[LightingTechnique::MAX_SPOT_LIGHTS];
    bool m_isPBR = false;
};
