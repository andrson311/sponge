#include "util.h"
#include "billboard_technique.h"

bool BillboardTechnique::Init()
{
    if (!Technique::Init())
    {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "bin/shaders/billboard.vs"))
    {
        return false;
    }

    if (!AddShader(GL_GEOMETRY_SHADER, "bin/shaders/billboard.gs"))
    {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "bin/shaders/billboard.fs"))
    {
        return false;
    }

    if (!Finalize())
    {
        return false;
    }

    m_VPLocation = GetUniformLocation("gVP");
    m_cameraPosLocation = GetUniformLocation("gCameraPos");
    m_colorMapLocation = GetUniformLocation("gColorMap");

    if (m_VPLocation == INVALID_UNIFORM_LOCATION ||
        m_cameraPosLocation == INVALID_UNIFORM_LOCATION ||
        m_colorMapLocation == INVALID_UNIFORM_LOCATION)
    {
        return false;
    }

    return true;
}

void BillboardTechnique::SetVP(const glm::mat4 &VP)
{
    glUniformMatrix4fv(m_VPLocation, 1, GL_FALSE, glm::value_ptr(VP));
}

void BillboardTechnique::SetCameraPosition(const glm::vec3 &Pos)
{
    glUniform3f(m_cameraPosLocation, Pos.x, Pos.y, Pos.z);
}

void BillboardTechnique::SetColorTextureUnit(u_int TextureUnit)
{
    glUniform1i(m_colorMapLocation, TextureUnit);
}
