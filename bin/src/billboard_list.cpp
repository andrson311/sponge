#include "util.h"
#include "billboard_list.h"

BillboardList::BillboardList()
{
    m_pTexture = NULL;
    m_vb = INVALID_OGL_VALUE;
}

BillboardList::~BillboardList()
{
    if (m_pTexture)
    {
        delete m_pTexture;
        m_pTexture = NULL;
    }

    if (m_vb != INVALID_OGL_VALUE)
    {
        glDeleteBuffers(1, &m_vb);
    }

    if (m_vao != INVALID_OGL_VALUE)
    {
        glDeleteVertexArrays(1, &m_vao);
    }
}

bool BillboardList::Init(const std::string &TexFilename, const std::vector<glm::vec3> &Positions)
{
    m_pTexture = new Texture(GL_TEXTURE_2D, TexFilename.c_str());

    if (!m_pTexture->Load())
    {
        return false;
    }

    CreatePositionBuffer(Positions);

    if (!m_technique.Init())
    {
        return false;
    }

    m_technique.SetColorTextureUnit(COLOR_TEXTURE_UNIT_INDEX);

    return true;
}

void BillboardList::CreatePositionBuffer(const std::vector<glm::vec3> &Positions)
{
    m_numPoints = (int)Positions.size();

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vb);
    glBindBuffer(GL_ARRAY_BUFFER, m_vb);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Positions[0]) * Positions.size(), &Positions[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void BillboardList::Render(const glm::mat4 &VP, const glm::vec3 &CameraPos)
{
    m_technique.Enable();
    m_technique.SetVP(VP);
    m_technique.SetCameraPosition(CameraPos);
    m_pTexture->Bind(COLOR_TEXTURE_UNIT);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_POINTS, 0, m_numPoints);
    glBindVertexArray(0);
}
