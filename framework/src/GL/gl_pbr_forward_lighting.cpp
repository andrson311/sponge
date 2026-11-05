#include "gl_pbr_forward_lighting.h"

bool PBRForwardLightingTechnique::Init()
{
    if (!InitShaders({
            {GL_VERTEX_SHADER, "framework/shaders/GL/forward_lighting.vs"},
            {GL_FRAGMENT_SHADER, "framework/shaders/GL/pbr_forward_lighting.fs"},
        }))
    {
        return false;
    }

    return ResolveUniforms();
}

bool PBRForwardLightingTechnique::ResolveUniforms()
{
    if (!BaseLightingTechnique::Init())
    {
        return false;
    }

    return InitUniforms({{&m_aoUnit, "gAmbientOcclusion"},
                         {&m_emissiveUnit, "gEmissive"},
                         {&m_albedoUnit, "gAlbedo"},
                         {&m_roughnessUnit, "gRoughness"},
                         {&m_normalUnit, "gNormal"},
                         {&m_envmapUnit, "gEnvMap"},
                         {&m_brdfLUTUnit, "gBRDF_LUT"},
                         {&m_irradianceUnit, "gIrradiance"},
                         {&m_clearCoat, "gClearCoat"},
                         {&m_clearCoatRoughness, "gClearCoatRoughness"},
                         {&m_clearCoatNormal, "gClearCoatNormal"},
                         {&m_baseColor, "gBaseColor"},
                         {&m_emissiveColor, "gEmissiveColor"},
                         {&m_metallicRoughnessNormalOcclusion, "gMetallicRoughnessNormalOcclusion"},
                         {&m_clearCoatTransmissionThickness, "gClearCoatTransmissionThickness"},
                         {&m_ior, "gIOR"},
                         {&m_materialType, "gMaterialType"}});
}

void PBRForwardLightingTechnique::SetMaterial(const CoreMaterial &mat)
{
    m_baseColor.Set(mat.BaseColor);
    m_emissiveColor.Set(mat.EmissiveColor);
    m_metallicRoughnessNormalOcclusion.Set(mat.MetallicRoughnessNormalOcclusion);
    m_clearCoatTransmissionThickness.Set(mat.ClearCoatTransmissionThickness);
    m_ior.Set(mat.m_ior);
    m_materialType.Set(mat.m_materialType);
}
