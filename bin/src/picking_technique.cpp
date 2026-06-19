#include "../inc/picking_technique.h"

bool PickingTechnique::Init()
{
    if (!Technique::Init())
    {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "bin/shaders/picking.vs"))
    {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "bin/shaders/picking.fs"))
    {
        return false;
    }

    if (!Finalize())
    {
        return false;
    }

    m_WVPLocation = GetUniformLocation("gWVP");
    m_objectIndexLocation = GetUniformLocation("gObjectIndex");
    m_drawIndexLocation = GetUniformLocation("gDrawIndex");

    if (m_WVPLocation == INVALID_UNIFORM_LOCATION ||
        m_objectIndexLocation == INVALID_UNIFORM_LOCATION ||
        m_drawIndexLocation == INVALID_UNIFORM_LOCATION)
    {
        return false;
    }

    return true;
}

void PickingTechnique::SetWVP(const glm::mat4 &WVP)
{
    glUniformMatrix4fv(m_WVPLocation, 1, GL_FALSE, glm::value_ptr(WVP));
}

void PickingTechnique::DrawStartCB(u_int DrawIndex)
{
    glUniform1ui(m_drawIndexLocation, DrawIndex);
}

void PickingTechnique::SetObjectIndex(u_int ObjectIndex)
{
    glUniform1ui(m_objectIndexLocation, ObjectIndex);
}
