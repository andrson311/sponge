#pragma once

#include "technique.h"
#include "math_3d.h"
#include "core_material.h"
#include "world_transform.h"
#include "lighting.h"
#include "core_model.h"
#include "gl_base_lighting_technique.h"

class PBRForwardLightingTechnique : public BaseLightingTechnique
{
public:
    PBRForwardLightingTechnique() = default;

    virtual bool Init() override;

    virtual void SetMaterial(const CoreMaterial &material) override;

    void SetAmbientOcclusionTextureUnit(int textureUnit) { m_aoUnit.Set(textureUnit); }
    void SetEmissiveTextureUnit(int textureUnit) { m_emissiveUnit.Set(textureUnit); }
    void SetAlbedoTextureUnit(int textureUnit) { m_albedoUnit.Set(textureUnit); }
    void SetRoughnessTextureUnit(int textureUnit) { m_roughnessUnit.Set(textureUnit); }
    void SetNormalTextureUnit(int textureUnit) { m_normalUnit.Set(textureUnit); }
    void SetEnvmapTextureUnit(int textureUnit) { m_envmapUnit.Set(textureUnit); }
    void SetBRDF_LUTTextureUnit(int textureUnit) { m_brdfLUTUnit.Set(textureUnit); }
    void SetIrradianceTextureUnit(int textureUnit) { m_irradianceUnit.Set(textureUnit); }
    void SetClearCoatTextureUnit(int textureUnit) { m_clearCoat.Set(textureUnit); }
    void SetClearCoatRoughnessTextureUnit(int textureUnit) { m_clearCoatRoughness.Set(textureUnit); }
    void SetClearCoatNormalTextureUnit(int textureUnit) { m_clearCoatNormal.Set(textureUnit); }

protected:
    bool ResolveUniforms();

private:
    Uniform<int> m_aoUnit;
    Uniform<int> m_emissiveUnit;
    Uniform<int> m_albedoUnit;
    Uniform<int> m_roughnessUnit;
    Uniform<int> m_normalUnit;
    Uniform<int> m_envmapUnit;
    Uniform<int> m_brdfLUTUnit;
    Uniform<int> m_irradianceUnit;

    Uniform<int> m_clearCoat;
    Uniform<int> m_clearCoatRoughness;
    Uniform<int> m_clearCoatNormal;
    Uniform<glm::vec4> m_clearCoatTransmissionThickness;

    Uniform<glm::vec4> m_baseColor;
    Uniform<glm::vec4> m_emissiveColor;
    Uniform<glm::vec4> m_metallicRoughnessNormalOcclusion;
    Uniform<float> m_ior;
    Uniform<int> m_materialType;
};
