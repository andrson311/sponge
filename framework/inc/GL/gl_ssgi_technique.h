#pragma once

#include "technique.h"
#include "math_3d.h"

class SSGITechnique : public Technique
{
public:
    SSGITechnique() = default;

    virtual bool Init() override;

    void SetAlbedoTextureUnit(u_int textureUnit) { m_albedoSampler.Set((int)textureUnit); }
    void SetNormalTextureUnit(u_int textureUnit) { m_normalSampler.Set((int)textureUnit); }
    void SetDepthTextureUnit(u_int textureUnit) { m_depthSampler.Set((int)textureUnit); }
    void SetProj(const glm::mat4 &proj);
    void SetViewTransform(const glm::mat4 &view) { m_view.Set(view); }

    void Render();

private:
    GLuint m_dummyVAO = 0;

    Uniform<int> m_albedoSampler;
    Uniform<int> m_normalSampler;
    Uniform<int> m_depthSampler;
    Uniform<glm::mat4> m_proj;
    Uniform<glm::mat4> m_invProj;
    Uniform<glm::mat4> m_view;
};
