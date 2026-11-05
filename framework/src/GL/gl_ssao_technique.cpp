#include "gl_ssao_technique.h"

bool SSAOTechnique::Init()
{
    if (!InitShaders({{GL_VERTEX_SHADER, "framework/shaders/GL/full_screen_quad.vs"},
                      {GL_FRAGMENT_SHADER, "framework/shaders/GL/ssao.fs"}}))
    {
        return false;
    }

    glGenVertexArrays(1, &m_dummyVAO);

    return true;
}

void SSAOTechnique::Render()
{
    glBindVertexArray(m_dummyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
