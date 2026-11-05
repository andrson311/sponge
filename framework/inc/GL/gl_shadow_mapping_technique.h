#pragma once

#include "technique.h"
#include "math_3d.h"

class ShadowMappingTechnique : public Technique
{
public:
    ShadowMappingTechnique() = default;

    virtual bool Init() override;

    void SetWVP(const glm::mat4 &WVP) { m_WVP.Set(WVP); }
    void SetVP(const glm::mat4 &VP) { m_VP.Set(VP); }
    void ControlIndirectRender(bool enable) { m_isIndirectRender.Set(enable); }
    void ControlPVP(bool enable) { m_isPVP.Set(enable); }

private:
    Uniform<glm::mat4> m_WVP;
    Uniform<glm::mat4> m_VP;
    Uniform<bool> m_isIndirectRender;
    Uniform<bool> m_isPVP;
};
