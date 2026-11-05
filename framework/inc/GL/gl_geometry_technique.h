#pragma once

#include "technique.h"
#include "math_3d.h"

class GeometryTechnique : public Technique
{
public:
    GeometryTechnique() = default;

    virtual bool Init() override;

    void SetWVP(const glm::mat4 &WVP) { m_WVP.Set(WVP); }
    void SetNormalMatrix(const glm::mat3 &normalMatrix) { m_normalMatrix.Set(normalMatrix); }
    void SetVP(const glm::mat4 &VP) { m_VP.Set(VP); }
    void SetAlbedoTextureUnit(u_int textureUnit) { m_albedoSampler.Set((int)textureUnit); }
    void ControlIndirectRender(bool enable) { m_isIndirectRender.Set(enable); }
    void ControlPVP(bool enable) { m_isPVP.Set(enable); }

private:
    Uniform<glm::mat4> m_WVP;
    Uniform<glm::mat3> m_normalMatrix;
    Uniform<glm::mat4> m_VP;
    Uniform<bool> m_isIndirectRender;
    Uniform<bool> m_isPVP;
    Uniform<int> m_albedoSampler;
};
