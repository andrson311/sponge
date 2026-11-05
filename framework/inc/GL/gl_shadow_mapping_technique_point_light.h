#pragma once

#include "util.h"
#include "technique.h"

class ShadowMappingPointLightTechnique : public Technique
{
public:
    ShadowMappingPointLightTechnique() = default;

    virtual bool Init() override;

    void SetWVP(const glm::mat4 &WVP) { m_WVP.Set(WVP); };
    void SetWorld(const glm::mat4 &World) { m_worldMatrix.Set(World); };
    void SetLightWorldPos(const glm::vec3 &Pos) { m_lightWorldPos.Set(Pos); };

private:
    Uniform<glm::mat4> m_WVP;
    Uniform<glm::mat4> m_worldMatrix;
    Uniform<glm::vec3> m_lightWorldPos;
};
