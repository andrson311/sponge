#pragma once

#include "math_3d.h"
#include "util.h"
#include "technique.h"

class FlatColorTechnique : public Technique
{
public:
    FlatColorTechnique() {}

    virtual bool Init();

    void SetWVP(const glm::mat4 &WVP) { m_WVP.Set(WVP); }
    void SetColor(const glm::vec4 &Col) { m_color.Set(Col); }

private:
    Uniform<glm::mat4> m_WVP;
    Uniform<glm::vec4> m_color;
};
