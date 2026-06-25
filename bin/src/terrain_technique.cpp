#include "terrain_technique.h"

bool TerrainTechnique::Init()
{
    if (!Technique::Init())
    {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "bin/shaders/terrain.vs"))
    {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "bin/shaders/terrain.fs"))
    {
        return false;
    }

    if (!Finalize())
    {
        return false;
    }

    m_VPLoc = GetUniformLocation("gVP");

    if (m_VPLoc == INVALID_UNIFORM_LOCATION)
    {
        return false;
    }

    return true;
}

void TerrainTechnique::SetVP(const glm::mat4 &VP)
{
    glUniformMatrix4fv(m_VPLoc, 1, GL_FALSE, glm::value_ptr(VP));
}
