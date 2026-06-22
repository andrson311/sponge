#include "shadow_mapping_technique_point_light.h"

bool ShadowMappingPointLightTechnique::Init()
{
    if (!Technique::Init())
    {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "bin/shaders/shadow_map_point_light.vs"))
    {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "bin/shaders/shadow_map_point_light.fs"))
    {
        return false;
    }

    if (!Finalize())
    {
        return false;
    }

    m_WVPLoc = GetUniformLocation("gWVP");
    m_worldMatrixLoc = GetUniformLocation("gWorld");
    m_lightWorldPosLoc = GetUniformLocation("gLightWorldPos");

    if (m_WVPLoc == INVALID_UNIFORM_LOCATION ||
        m_worldMatrixLoc == INVALID_UNIFORM_LOCATION ||
        m_lightWorldPosLoc == INVALID_UNIFORM_LOCATION)
    {
        return false;
    }

    return true;
}

void ShadowMappingPointLightTechnique::SetWVP(const glm::mat4 &WVP)
{
    glUniformMatrix4fv(m_WVPLoc, 1, GL_FALSE, glm::value_ptr(WVP));
}

void ShadowMappingPointLightTechnique::SetWorld(const glm::mat4 &World)
{
    glUniformMatrix4fv(m_worldMatrixLoc, 1, GL_FALSE, glm::value_ptr(World));
}

void ShadowMappingPointLightTechnique::SetLightWorldPos(const glm::vec3 &Pos)
{
    glUniform3f(m_lightWorldPosLoc, Pos.x, Pos.y, Pos.z);
}
