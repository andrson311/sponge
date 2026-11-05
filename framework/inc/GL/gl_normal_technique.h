#pragma once

#include "technique.h"
#include "math_3d.h"

class NormalTechnique : public Technique
{
public:
    NormalTechnique() = default;

    virtual bool Init() override;

    void SetWVP(const glm::mat4 &WVP) { m_WVP.Set(WVP); }
    void SetNormalMatrix(const glm::mat3 &normalMatrix) { m_normalMatrix.Set(normalMatrix); }
    void SetVP(const glm::mat4 &VP) { m_VP.Set(VP); }
    void ControlIndirectRender(bool isRenderIndirect) { m_isIndirectRender.Set(isRenderIndirect); }
    void ControlPVP(bool isPVP) { m_isPVP.Set(isPVP); }

private:
    Uniform<glm::mat4> m_WVP;
    Uniform<glm::mat3> m_normalMatrix;
    Uniform<glm::mat4> m_VP;
    Uniform<bool> m_isIndirectRender;
    Uniform<bool> m_isPVP;
};
