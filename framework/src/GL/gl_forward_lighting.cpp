#include "gl_forward_lighting.h"

bool ForwardLightingTechnique::Init()
{
    if (!InitShaders({{GL_VERTEX_SHADER, "framework/shaders/GL/forward_lighting.vs"},
                      {GL_FRAGMENT_SHADER, "framework/shaders/GL/forward_lighting.fs"}}))
    {
        return false;
    }

    return ResolveUniforms();
}

bool ForwardLightingTechnique::ResolveUniforms()
{
    if (!BaseLightingTechnique::Init())
    {
        return false;
    }

    return InitUniforms({{&m_lightWVP, "gLightWVP"},
                         {&m_hasNormalMap, "gHasNormalMap"},
                         {&m_sampler, "gSampler"},
                         {&m_hasSampler, "gHasSampler"},
                         {&m_shadowMap, "gShadowMap"},
                         {&m_shadowCubeMap, "gShadowCubeMap"},
                         {&m_normalMap, "gNormalMap"},
                         //{&m_heightMap, "gHeightMap"},
                         {&m_shadowMapWidth, "gShadowMapWidth"},
                         {&m_shadowMapHeight, "gShadowMapHeight"},
                         {&m_shadowMapFilterSize, "gShadowMapFilterSize"},
                         {&m_shadowMapOffsetTexture, "gShadowMapOffsetTexture"},
                         {&m_shadowMapOffsetTextureSize, "gShadowMapOffsetTextureSize"},
                         {&m_shadowMapOffsetFilterSize, "gShadowMapOffsetFilterSize"},
                         {&m_shadowMapRandomRadius, "gShadowMapRandomRadius"},
                         {&m_samplerSpecularExponent, "gSamplerSpecularExponent"},
                         {&m_colorMod, "gColorMod"},
                         {&m_colorAdd, "gColorAdd"},
                         {&m_enableRimLight, "gRimLightEnabled"},
                         {&m_enableCellShading, "gCellShadingEnabled"},
                         {&m_enableSpecularExponent, "gEnableSpecularExponent"},
                         {&m_fogStart, "gFogStart"},
                         {&m_fogEnd, "gFogEnd"},
                         {&m_fogColor, "gFogColor"},
                         {&m_expFogDensity, "gExpFogDensity"},
                         {&m_expSquaredFogEnabled, "gExpSquaredFogEnabled"},
                         {&m_layeredFogTop, "gLayeredFogTop"},
                         {&m_fogTime, "gFogTime"},
                         {&m_refRefractEnabled, "gRefRefractEnabled"},
                         //{&m_reflectionFactor, "gReflectionFactor"},
                         {&m_materialToRefRefractFactor, "gMatToRefRefractFactor"},
                         {&m_ETA, "gETA"},
                         {&m_fresnelPower, "gFresnelPower"},
                         {&m_material.Ambient, "gMaterial.AmbientColor"},
                         {&m_material.Diffuse, "gMaterial.DiffuseColor"},
                         {&m_material.Specular, "gMaterial.SpecularColor"},
                         {&m_gCubemapTexture, "gCubemapTexture"},
                         {&m_gIsCubeMapping, "gIsCubemapping"},
                         {&m_gCubeMipmapLevel, "gCubeMipmapLevel"},
                         {&m_gProjectedTexture, "gProjectedTexture"},
                         {&m_gProjectionMatrix, "gProjectionMatrix"}});
}

void ForwardLightingTechnique::SetShadowMapSize(u_int width, u_int height)
{
    m_shadowMapWidth.Set((int)width);
    m_shadowMapHeight.Set((int)height);
}

void ForwardLightingTechnique::SetShadowMapOffsetTextureParams(float textureSize,
                                                               float filterSize,
                                                               float radius)
{
    m_shadowMapOffsetTextureSize.Set(textureSize);
    m_shadowMapOffsetFilterSize.Set(filterSize);
    m_shadowMapRandomRadius.Set(radius);
}

void ForwardLightingTechnique::SetMaterial(const CoreMaterial &material)
{
    m_material.Ambient.Set(material.AmbientColor);
    m_material.Diffuse.Set(material.DiffuseColor);
    m_material.Specular.Set(material.SpecularColor);

    bool hasDiffuseTexture = (material.pTextures[TEX_TYPE_BASE] != NULL);
    ControlDiffuseTexture(hasDiffuseTexture);
}

void ForwardLightingTechnique::SetLinearFog(float start, float end)
{
    if (start < 0.0f)
    {
        printf("Fog start must be positive %f\n", start);
        exit(1);
    }

    if (end < 0.0f)
    {
        printf("Fog end must be positive %f\n", end);
        exit(1);
    }

    if (start >= end)
    {
        printf("Fog start %f must be smaller than fog end %f\n", start, end);
        exit(1);
    }

    m_layeredFogTop.Set(-1.0f);
    m_fogTime.Set(-1.0f);
    m_fogStart.Set(start);
    m_fogEnd.Set(end);
}

void ForwardLightingTechnique::SetExpFog(float end, float density)
{
    SetExpFogCommon(end, density);
    m_expSquaredFogEnabled.Set(false);
}

void ForwardLightingTechnique::SetExpSquaredFog(float end, float density)
{
    SetExpFogCommon(end, density);
    m_expSquaredFogEnabled.Set(true);
}

void ForwardLightingTechnique::SetExpFogCommon(float end, float density)
{
    if (end < 0.0f)
    {
        printf("Fog end must be positive: %f\n", end);
        exit(1);
    }

    if (density < 0.0f)
    {
        printf("Fog density must be positive: %f\n", density);
        exit(1);
    }

    m_fogStart.Set(-1.0f);
    m_layeredFogTop.Set(-1.0f);
    m_fogTime.Set(-1.0f);
    m_fogEnd.Set(end);
    m_expFogDensity.Set(density);
}

void ForwardLightingTechnique::SetLayeredFog(float top, float end)
{
    if (top < 0.0f)
    {
        printf("Fog top must be positive: %f\n", top);
        exit(1);
    }

    if (end < 0.0f)
    {
        printf("Fog end must be positive: %f\n", end);
        exit(1);
    }

    m_fogStart.Set(-1.0f);
    m_fogTime.Set(-1.0f);
    m_layeredFogTop.Set(top);
    m_fogEnd.Set(end);
}

void ForwardLightingTechnique::SetAnimatedFog(float end, float density)
{
    m_fogStart.Set(-1.0f);
    m_layeredFogTop.Set(-1.0f);
    m_fogEnd.Set(end);
    m_expFogDensity.Set(density);
}
