#pragma once

#include "math_3d.h"
#include "util.h"
#include "technique.h"

class SkyboxTechnique : public Technique
{
public:
    SkyboxTechnique() = default;

    virtual bool Init() override;

    void SetVP(const glm::mat4 &VP) { m_VP.Set(VP); }
    void SetTextureUnitIndex(int textureUnitIndex) { m_cubemapSampler.Set(textureUnitIndex); }

private:
    Uniform<glm::mat4> m_VP;
    Uniform<int> m_cubemapSampler;
};
