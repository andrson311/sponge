#include "gl_full_screen_technique.h"

bool FullScreenTechnique::Init(const char *pFragmentShaderFile)
{
    if (!InitShaders({{GL_VERTEX_SHADER, "framework/shaders/GL/full_screen_tri.vs"},
                      {GL_FRAGMENT_SHADER, pFragmentShaderFile}}))
    {
        return false;
    }

    glGenVertexArrays(1, &m_dummyVAO);

    return true;
}

void FullScreenTechnique::Render()
{
    Enable();
    glBindVertexArray(m_dummyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}
