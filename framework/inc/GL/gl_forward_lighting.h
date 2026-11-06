#pragma once

#include "technique.h"
#include "gl_base_lighting_technique.h"
#include "lighting.h"
#include "math_3d.h"
#include "world_transform.h"
#include "core_material.h"
#include "core_model.h"

class ForwardLightingTechnique : public BaseLightingTechnique
{
public:
    ForwardLightingTechnique() = default;

    virtual bool Init() override;

    virtual void SetMaterial(const CoreMaterial &material) override;

    virtual void ControlSpecularExponent(bool enable) override { m_enableSpecularExponent.Set(enable); }
    virtual void ControlRefRefract(bool enable) override { m_refRefractEnabled.Set(enable); }
    virtual void SetReflectionFactor(float factor) override { m_reflectionFactor.Set(factor); }

    virtual void SetColorMod(const glm::vec4 &colorMod) override { m_colorMod.Set(colorMod); }
    virtual void SetColorAdd(const glm::vec4 &colorAdd) override { m_colorAdd.Set(colorAdd); }

    virtual void SetMaterialToRefRefractFactor(float factor) override { m_materialToRefRefractFactor.Set(factor); }
    virtual void SetRefractETA(float f) override { m_ETA.Set(f); }
    virtual void SetFresnelPower(float f) override { m_fresnelPower.Set(f); }

    virtual void SetLightWVP(const glm::mat4 &lightWVP) override { m_lightWVP.Set(lightWVP); }

    virtual void ControlNormalMap(bool enable) override { m_hasNormalMap.Set(enable); }

    virtual void ControlCubemapping(bool enable) { m_gIsCubeMapping.Set(enable); }
    virtual void SetCubeMipmapLevel(int level) { m_gCubeMipmapLevel.Set((int)level); }
    virtual void SetProjectionMatrix(const glm::mat4 &m) { m_gProjectionMatrix.Set(m); }

    void SetTextureUnit(u_int textureUnit) { m_sampler.Set((int)textureUnit); }
    void SetShadowMapTextureUnit(u_int textureUnit) { m_shadowMap.Set((int)textureUnit); }
    void SetShadowCubeMapTextureUnit(u_int textureUnit) { m_shadowCubeMap.Set((int)textureUnit); }
    void SetShadowMapOffsetTextureUnit(u_int textureUnit) { m_shadowMapOffsetTexture.Set((int)textureUnit); }
    void SetSpecularExponentTextureUnit(u_int textureUnit) { m_samplerSpecularExponent.Set((int)textureUnit); }
    void SetNormalMapTextureUnit(u_int textureUnit) { m_normalMap.Set((int)textureUnit); }
    void SetHeightMapTextureUnit(u_int textureUnit) { m_heightMap.Set((int)textureUnit); }

    void SetShadowMapSize(u_int width, u_int height);
    void SetShadowMapFilterSize(u_int size) { m_shadowMapFilterSize.Set((int)size); }
    void SetShadowMapOffsetTextureParams(float textureSize, float filterSize, float radius);

    void SetCubeMapTextureUnit(u_int textureUnit) { m_gCubemapTexture.Set((int)textureUnit); }
    void SetProjectedTextureUnit(u_int textureUnit) { m_gProjectedTexture.Set((int)textureUnit); }

    void ControlRimLight(bool enable) { m_enableRimLight.Set(enable); }
    void ControlCellShading(bool enable) { m_enableCellShading.Set(enable); }

    void SetLinearFog(float fogStart, float fogEnd);
    void SetExpFog(float fogEnd, float fogDensity);
    void SetExpSquaredFog(float fogEnd, float fogDensity);
    void SetLayeredFog(float fogTop, float fogEnd);
    void SetFogColor(const glm::vec3 &fogColor) { m_fogColor.Set(fogColor); }
    void SetAnimatedFog(float fogEnd, float fogDensity);
    void SetFogTime(float time) { m_fogTime.Set(time); }

protected:
    bool ResolveUniforms();

private:
    void SetExpFogCommon(float fogEnd, float fogDensity);
    void ControlDiffuseTexture(bool enable) { m_hasSampler.Set(enable); }

    Uniform<glm::mat4> m_lightWVP;
    Uniform<int> m_sampler;
    Uniform<bool> m_hasSampler;

    Uniform<bool> m_hasNormalMap;
    Uniform<int> m_normalMap;

    Uniform<int> m_heightMap;

    Uniform<int> m_shadowMap;
    Uniform<int> m_shadowCubeMap;
    Uniform<int> m_shadowMapWidth;
    Uniform<int> m_shadowMapHeight;
    Uniform<int> m_shadowMapFilterSize;
    Uniform<float> m_shadowMapOffsetFilterSize;
    Uniform<int> m_shadowMapOffsetTexture;
    Uniform<float> m_shadowMapOffsetTextureSize;
    Uniform<float> m_shadowMapRandomRadius;

    Uniform<int> m_samplerSpecularExponent;
    Uniform<int> m_numPointLights;
    Uniform<int> m_numSpotLights;

    Uniform<glm::vec4> m_colorMod;
    Uniform<glm::vec4> m_colorAdd;

    Uniform<bool> m_enableRimLight;
    Uniform<bool> m_enableCellShading;
    Uniform<bool> m_enableSpecularExponent;

    Uniform<float> m_fogStart;
    Uniform<float> m_fogEnd;
    Uniform<glm::vec3> m_fogColor;
    Uniform<float> m_expFogDensity;
    Uniform<bool> m_expSquaredFogEnabled;
    Uniform<float> m_layeredFogTop;
    Uniform<float> m_fogTime;

    Uniform<bool> m_refRefractEnabled;
    Uniform<float> m_reflectionFactor;
    Uniform<float> m_materialToRefRefractFactor;
    Uniform<float> m_ETA;
    Uniform<float> m_fresnelPower;
    
    Uniform<int> m_gCubemapTexture;
    Uniform<bool> m_gIsCubeMapping;
    Uniform<int> m_gCubeMipmapLevel;
    Uniform<int> m_gProjectedTexture;
    Uniform<glm::mat4> m_gProjectionMatrix;

    struct
    {
        Uniform<glm::vec4> Ambient;
        Uniform<glm::vec4> Diffuse;
        Uniform<glm::vec4> Specular;
    } m_material;
};
