#include "single_tex_terrain_technique.h"

bool SingleTexTerrainTechnique::Init()
{
    if (!Technique::Init())
    {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "bin/shaders/single_tex_terrain.vs"))
    {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "bin/shaders/single_tex_terrain.fs"))
    {
        return false;
    }

    if (!Finalize())
    {
        return false;
    }

    m_VPLoc = GetUniformLocation("gVP");
    m_texUnitLoc = GetUniformLocation("gTerrainTexture");
    m_minHeightLoc = GetUniformLocation("gMinHeight");
    m_maxHeightLoc = GetUniformLocation("gMaxHeight");

    if (m_VPLoc == INVALID_UNIFORM_LOCATION ||
        m_texUnitLoc == INVALID_UNIFORM_LOCATION ||
        m_minHeightLoc == INVALID_UNIFORM_LOCATION ||
        m_maxHeightLoc == INVALID_UNIFORM_LOCATION)
    {
        return false;
    }

    Enable();

    glUniform1i(m_texUnitLoc, COLOR_TEXTURE_UNIT_INDEX);

    return true;
}

void SingleTexTerrainTechnique::SetVP(const glm::mat4 &VP)
{
    glUniformMatrix4fv(m_VPLoc, 1, GL_FALSE, glm::value_ptr(VP));
}

void SingleTexTerrainTechnique::SetMinMaxHeight(float Min, float Max)
{
    glUniform1f(m_minHeightLoc, Min);
    glUniform1f(m_maxHeightLoc, Max);
}
