#include <limits.h>
#include <string.h>
#include "skybox_technique.h"
#include "util.h"

bool SkyboxTechnique::Init()
{
    if (!Technique::Init())
    {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "bin/shaders/skybox.vs"))
    {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "bin/shaders/skybox.fs"))
    {
        return false;
    }

    if (!Finalize())
    {
        return false;
    }

    m_WVPLoc = GetUniformLocation("gWVP");
    m_textureLoc = GetUniformLocation("gCubemapTexture");

    if (m_WVPLoc == INVALID_UNIFORM_LOCATION ||
        m_textureLoc == INVALID_UNIFORM_LOCATION)
    {
        return false;
    }

    return true;
}

void SkyboxTechnique::SetWVP(const glm::mat4 &WVP)
{
    glUniformMatrix4fv(m_WVPLoc, 1, GL_FALSE, glm::value_ptr(WVP));
}

void SkyboxTechnique::SetTextureUnit(u_int TextureUnit)
{
    glUniform1i(m_textureLoc, TextureUnit);
}
