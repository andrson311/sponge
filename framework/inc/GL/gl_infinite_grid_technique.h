#pragma once

#include "technique.h"
#include "math_3d.h"

class InfiniteGridTechnique : public Technique
{
public:
    InfiniteGridTechnique() = default;

    virtual bool Init() override;

    void SetVP(const glm::mat4 &VP) { m_VP.Set(VP); }
    void SetLightVP(const glm::mat4 &lightVP) { m_lightVP.Set(lightVP); }
    void SetCameraWorldPos(const glm::vec3 &pos) { m_cameraWorldPos.Set(pos); }
    void SetCellSize(float size) { m_gridCellSize.Set(size); }
    void SetShadowMapTextureUnit(u_int textureUnit) { m_shadowMap.Set((int)textureUnit); }
    void SetLightDirection(const glm::vec3 &dir) { m_lightDir.Set(glm::normalize(dir)); }
    void ControlShadows(bool enable) { m_shadowsEnabled.Set(enable); }

private:
    Uniform<glm::mat4> m_VP;
    Uniform<glm::vec3> m_cameraWorldPos;
    Uniform<float> m_gridCellSize;
    Uniform<glm::mat4> m_lightVP;
    Uniform<int> m_shadowMap;
    Uniform<glm::vec3> m_lightDir;
    Uniform<bool> m_shadowsEnabled;
};
