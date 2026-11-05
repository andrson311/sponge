#include "gl_ssgi_technique.h"

bool SSGITechnique::Init()
{
    if (!InitShaders({{GL_VERTEX_SHADER, "framework/shaders/GL/full_screen_tri.vs"},
                      {GL_FRAGMENT_SHADER, "framework/shaders/GL/ssgi.fs"}}))
    {
        return false;
    }

    if (!InitUniforms({{&m_albedoSampler, "gAlbedoTex"},
                       {&m_normalSampler, "gNormalTex"},
                       {&m_depthSampler, "gDepthTex"},
                       {&m_proj, "gProj"},
                       {&m_invProj, "gInvProj"},
                       {&m_view, "gView"}}))
    {
        return false;
    }

    glGenVertexArrays(1, &m_dummyVAO);

    return true;
}

void SSGITechnique::Render()
{
    glBindVertexArray(m_dummyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void SSGITechnique::SetProj(const glm::mat4 &proj)
{
    m_proj.Set(proj);
    m_invProj.Set(glm::inverse(proj));
}
