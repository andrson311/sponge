#include "terrain_technique.h"
#include "texture_config.h"

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

    if (!AddShader(GL_GEOMETRY_SHADER, "bin/shaders/terrain.gs"))
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
    m_texUnitLoc = GetUniformLocation("gTextureHeight0");
    m_reversedLightDirLoc = GetUniformLocation("gReversedLightDir");
    m_timeLoc = GetUniformLocation("gTime");

    if (m_VPLoc == INVALID_UNIFORM_LOCATION ||
        m_texUnitLoc == INVALID_UNIFORM_LOCATION ||
        m_timeLoc == INVALID_UNIFORM_LOCATION ||
        m_reversedLightDirLoc == INVALID_UNIFORM_LOCATION)
    {
        return false;
    }

    Enable();

    glUniform1i(m_texUnitLoc, COLOR_TEXTURE_UNIT_INDEX_0);

    glUseProgram(0);

    return true;
}

void TerrainTechnique::SetVP(const glm::mat4 &VP)
{
    glUniformMatrix4fv(m_VPLoc, 1, GL_FALSE, glm::value_ptr(VP));
}

void TerrainTechnique::SetMinMaxHeight(float Min, float Max)
{
    // glUniform1f(m_minHeightLoc, Min);
    // glUniform1f(m_maxHeightLoc, Max);
}

void TerrainTechnique::SetTextureHeights(float Tex0Height, float Tex1Height, float Tex2Height, float Tex3Height)
{
    // glUniform1f(m_tex0HeightLoc, Tex0Height);
    // glUniform1f(m_tex1HeightLoc, Tex1Height);
    // glUniform1f(m_tex2HeightLoc, Tex2Height);
    // glUniform1f(m_tex3HeightLoc, Tex3Height);
}

void TerrainTechnique::SetLightDir(const glm::vec3 &Dir)
{
    glm::vec3 ReversedLightDir = glm::normalize(Dir * -1.0f);
    glUniform3f(m_reversedLightDirLoc, ReversedLightDir.x, ReversedLightDir.y, ReversedLightDir.z);
}

void TerrainTechnique::SetTime(float Time)
{
    glUniform1f(m_timeLoc, Time);

}
