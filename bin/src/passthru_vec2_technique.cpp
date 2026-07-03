#include "passthru_vec2_technique.h"

bool PassthruVec2Technique::Init()
{
    if (!Technique::Init())
    {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "bin/shaders/passthru_vec2.vs"))
    {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "bin/shaders/color.fs"))
    {
        return false;
    }

    if (!Finalize())
    {
        return false;
    }

    m_colorLoc = GetUniformLocation("gColor");

    if (m_colorLoc == INVALID_UNIFORM_LOCATION)
    {
        return false;
    }

    Enable();

    SetColor(1.0f, 1.0f, 1.0f);

    return true;
}

void PassthruVec2Technique::SetColor(float r, float g, float b)
{
    glUniform3f(m_colorLoc, r, g, b);
}
