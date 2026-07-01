#include "skydome_technique.h"

bool SkydomeTechnique::Init()
{
    if (!Technique::Init())
    {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "bin/shaders/skydome.vs"))
    {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "bin/shaders/skydome.fs"))
    {
        return false;
    }

    if (!Finalize())
    {
        return false;
    }

    m_WVPLoc = GetUniformLocation("gWVP");
    m_samplerLoc = GetUniformLocation("gSampler");

    if (m_WVPLoc == INVALID_UNIFORM_LOCATION ||
        m_samplerLoc == INVALID_UNIFORM_LOCATION)
    {
        return false;
    }

    return true;
}

void SkydomeTechnique::SetTextureUnit(u_int TextureUnit)
{
    glUniform1i(m_samplerLoc, TextureUnit);
}

void SkydomeTechnique::SetWVP(const glm::mat4 &WVP)
{
    glUniformMatrix4fv(m_WVPLoc, 1, GL_FALSE, glm::value_ptr(WVP));
}
