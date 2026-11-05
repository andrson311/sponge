#include "gl_ssao_combine_technique.h"

bool SSAOCombineTechnique::Init()
{
    if (!InitShaders({{GL_VERTEX_SHADER, "framework/shaders/GL/full_screen_quad.vs"},
                      {GL_FRAGMENT_SHADER, "framework/shaders/GL/ssao_combine.fs"}}))
    {
        return false;
    }

    glGenVertexArrays(1, &m_dummyVAO);

    return true;
}

void SSAOCombineTechnique::Render()
{
    glBindVertexArray(m_dummyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
