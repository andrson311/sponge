#include "traingle_list.h"
#include "terrain.h"

TriangleList::~TriangleList()
{
    Destroy();
}

void TriangleList::Destroy()
{
    if (m_vao > 0)
    {
        glDeleteVertexArrays(1, &m_vao);
    }

    if (m_vb > 0)
    {
        glDeleteBuffers(1, &m_vb);
    }

    if (m_ib > 0)
    {
        glDeleteBuffers(1, &m_ib);
    }
}

void TriangleList::CreateTriangleList(int Width, int Depth, const BaseTerrain *pTerrain)
{
    m_width = Width;
    m_depth = Depth;

    CreateGLState();
    PopulateBuffers(pTerrain);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void TriangleList::CreateGLState()
{
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    glGenBuffers(1, &m_vb);
    glBindBuffer(GL_ARRAY_BUFFER, m_vb);
    glGenBuffers(1, &m_ib);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);

    int POS_LOC = 0;
    int TEX_LOC = 1;
    int NORMAL_LOC = 2;

    size_t NumFloats = 0;

    glEnableVertexAttribArray(POS_LOC);
    glVertexAttribPointer(POS_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)(NumFloats * sizeof(float)));
    NumFloats += 3;

    glEnableVertexAttribArray(TEX_LOC);
    glVertexAttribPointer(TEX_LOC, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)(NumFloats * sizeof(float)));
    NumFloats += 2;

    glEnableVertexAttribArray(NORMAL_LOC);
    glVertexAttribPointer(NORMAL_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)(NumFloats * sizeof(float)));
    NumFloats += 3;
}

void TriangleList::PopulateBuffers(const BaseTerrain *pTerrain)
{
    std::vector<Vertex> Vertices;
    Vertices.resize(m_width * m_depth);
    InitVertices(pTerrain, Vertices);

    std::vector<u_int> Indices;
    int NumQuads = (m_width - 1) * (m_depth - 1);
    Indices.resize(NumQuads * 6);
    InitIndices(Indices);
    CalcNormals(Vertices, Indices);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);
}

void TriangleList::Vertex::InitVertex(const BaseTerrain *pTerrain, int x, int z)
{
    float y = pTerrain->GetHeight(x, z);

    float WorldScale = pTerrain->GetWorldScale();
    Pos = glm::vec3(x * WorldScale, y, z * WorldScale);

    float Size = (float)pTerrain->GetSize();
    float TextureScale = pTerrain->GetTextureScale();
    Tex = glm::vec2(TextureScale * (float)x / Size, TextureScale * (float)z / Size);
}

void TriangleList::InitVertices(const BaseTerrain *pTerrain, std::vector<Vertex> &Vertices)
{
    int Index = 0;

    for (int z = 0; z < m_depth; z++)
    {
        for (int x = 0; x < m_width; x++)
        {
            assert(Index < Vertices.size());
            Vertices[Index].InitVertex(pTerrain, x, z);
            Index++;
        }
    }

    assert(Index == Vertices.size());
}

void TriangleList::InitIndices(std::vector<u_int> &Indices)
{
    int Index = 0;

    for (int z = 0; z < m_depth - 1; z++)
    {
        for (int x = 0; x < m_width - 1; x++)
        {
            u_int IndexBottomLeft = z * m_width + x;
            u_int IndexTopLeft = (z + 1) * m_width + x;
            u_int IndexBottomRight = z * m_width + x + 1;
            u_int IndexTopRight = (z + 1) * m_width + x + 1;

            // add top left triangle
            assert(Index < Indices.size());
            Indices[Index++] = IndexBottomLeft;
            assert(Index < Indices.size());
            Indices[Index++] = IndexTopLeft;
            assert(Index < Indices.size());
            Indices[Index++] = IndexTopRight;

            // add bottom right triangle
            assert(Index < Indices.size());
            Indices[Index++] = IndexBottomLeft;
            assert(Index < Indices.size());
            Indices[Index++] = IndexTopRight;
            assert(Index < Indices.size());
            Indices[Index++] = IndexBottomRight;
        }
    }

    assert(Index == Indices.size());
}

void TriangleList::CalcNormals(std::vector<Vertex> &Vertices, std::vector<u_int> &Indices)
{
    u_int Index = 0;

    for (u_int i = 0; i < Indices.size(); i += 3)
    {
        u_int Index0 = Indices[i];
        u_int Index1 = Indices[i + 1];
        u_int Index2 = Indices[i + 2];

        glm::vec3 v1 = Vertices[Index1].Pos - Vertices[Index0].Pos;
        glm::vec3 v2 = Vertices[Index2].Pos - Vertices[Index0].Pos;
        glm::vec3 Normal = glm::normalize(glm::cross(v1, v2));

        Vertices[Index0].Normal += Normal;
        Vertices[Index1].Normal += Normal;
        Vertices[Index2].Normal += Normal;
    }

    for (u_int i = 0; i < Vertices.size(); i++)
    {
        Vertices[i].Normal = glm::normalize(Vertices[i].Normal);
    }
}

void TriangleList::Render()
{
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, (m_depth - 1) * (m_width - 1) * 6, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);
}
