#include "shadow_mapping_technique.h"

bool ShadowMappingTechnique::Init()
{
    if (!Technique::Init())
    {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "bin/shaders/shadow_map.vs"))
    {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "bin/shaders/empty.fs"))
    {
        return false;
    }

    if (!Finalize())
    {
        return false;
    }

    m_WVPLoc = GetUniformLocation("gWVP");

    if (m_WVPLoc == INVALID_UNIFORM_LOCATION)
    {
        return false;
    }

    return true;
}

void ShadowMappingTechnique::SetWVP(const glm::mat4 &WVP)
{
    glUniformMatrix4fv(m_WVPLoc, 1, GL_FALSE, glm::value_ptr(WVP));
}
