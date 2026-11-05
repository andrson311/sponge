#pragma once

#include "technique.h"
#include "math_3d.h"

class PickingTechnique : public Technique
{
public:
    PickingTechnique() = default;

    virtual bool Init() override;

    void SetWVP(const glm::mat4 &WVP) { m_WVP.Set(WVP); }
    void SetObjectIndex(u_int objectIndex) { m_objectIndex.Set((int)objectIndex); }
    void DrawStartCB(u_int drawIndex) { m_drawIndex.Set((int)drawIndex); }

private:
    Uniform<glm::mat4> m_WVP;
    Uniform<int> m_drawIndex;
    Uniform<int> m_objectIndex;
};
