#include <stdio.h>
#include <vector>

#include "math_3d.h"
#include "quad_list.h"
#include "terrain.h"

QuadList::~QuadList()
{
    Destroy();
}

void QuadList::Destroy()
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

void QuadList::CreateQuadList(int Width, int Depth, const BaseTerrain *pTerrain)
{
    m_width = Width;
    m_depth = Depth;

    CreateGLState();
    PopulateBuffers(pTerrain);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void QuadList::CreateGLState()
{
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vb);
    glBindBuffer(GL_ARRAY_BUFFER, m_vb);

    glGenBuffers(1, &m_ib);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);

    int pos_loc = 0;
    int tex_loc = 1;

    size_t NumFloats = 0;

    glEnableVertexAttribArray(pos_loc);
    glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)(NumFloats * sizeof(float)));
    NumFloats += 3;

    glEnableVertexAttribArray(tex_loc);
    glVertexAttribPointer(tex_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)(NumFloats * sizeof(float)));
    NumFloats += 2;
}

void QuadList::PopulateBuffers(const BaseTerrain *pTerrain)
{
    std::vector<Vertex> Vertices;
    Vertices.resize(m_width * m_depth);

    InitVertices(pTerrain, Vertices);

    std::vector<u_int> Indices;
    int NumQuads = (m_width - 1) * (m_depth - 1);
    Indices.resize(NumQuads * 4);
    InitIndices(Indices);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * Indices.size(), &Indices[0], GL_STATIC_DRAW);
}

void QuadList::Vertex::InitVertex(const BaseTerrain *pTerrain, int Width, int Depth, int x, int z)
{
    // float y = pTerrain->GetHeight(x, z);
    float WorldScale = pTerrain->GetWorldScale();
    Pos = glm::vec3(x * WorldScale, 0.0f, z * WorldScale);

    // float Size = (float)pTerrain->GetSize();
    float TextureScale = pTerrain->GetTextureScale();
    Tex = glm::vec2(TextureScale * (float)x / (float)Width, TextureScale * (float)z / (float)Depth);
}

void QuadList::InitVertices(const BaseTerrain *pTerrain, std::vector<Vertex> &Vertices)
{
    int Index = 0;

    for (int z = 0; z < m_depth; z++)
    {
        for (int x = 0; x < m_width; x++)
        {
            assert(Index < Vertices.size());
            Vertices[Index].InitVertex(pTerrain, m_width, m_depth, x, z);
            Index++;
        }
    }

    assert(Index == Vertices.size());
}

void QuadList::InitIndices(std::vector<u_int> &Indices)
{
    int Index = 0;

    for (int z = 0; z < m_depth - 1; z++)
    {
        for (int x = 0; x < m_width - 1; x++)
        {
            assert(Index < Indices.size());
            u_int IndexBottomLeft = z * m_width + x;
            Indices[Index++] = IndexBottomLeft;

            assert(Index < Indices.size());
            u_int IndexBottomRight = z * m_width + x + 1;
            Indices[Index++] = IndexBottomRight;

            assert(Index < Indices.size());
            u_int IndexTopLeft = (z + 1) * m_width + x;
            Indices[Index++] = IndexTopLeft;

            assert(Index < Indices.size());
            u_int IndexTopRight = (z + 1) * m_width + x + 1;
            Indices[Index++] = IndexTopRight;
        }
    }

    assert(Index == Indices.size());
}

void QuadList::Render()
{
    glBindVertexArray(m_vao);

    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glDrawElements(GL_PATCHES, (m_depth - 1) * (m_width - 1) * 4, GL_UNSIGNED_INT, NULL);

    glBindVertexArray(0);
}
