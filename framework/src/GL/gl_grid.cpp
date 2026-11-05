#include "gl_grid.h"

GLGrid::GLGrid(int width, int depth)
{
    CreateTriangleList(width, depth);
}

GLGrid::~GLGrid()
{
    Destroy();
}

void GLGrid::Destroy()
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

void GLGrid::CreateTriangleList(int width, int depth)
{
    m_width = width;
    m_depth = depth;

    CreateGLState();
    PopulateBuffers();

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLGrid::CreateGLState()
{
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vb);
    glBindBuffer(GL_ARRAY_BUFFER, m_vb);

    glGenBuffers(1, &m_ib);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ib);

    int POS_LOC = 0;
    int TEX_LOC = 1;

    size_t numFloats = 0;

    glEnableVertexAttribArray(POS_LOC);
    glVertexAttribPointer(POS_LOC, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (const void *)(numFloats * sizeof(float)));
    numFloats += 3;

    glEnableVertexAttribArray(TEX_LOC);
    glVertexAttribPointer(TEX_LOC, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (const void *)(numFloats * sizeof(float)));
    numFloats += 2;
}

void GLGrid::PopulateBuffers()
{
    std::vector<Vertex> vertices;
    vertices.resize(m_width * m_depth);
    InitVertices(vertices);

    std::vector<u_int> indices;
    int numQuads = (m_width - 1) * (m_depth - 1);
    indices.resize(numQuads * 6);
    InitIndices(indices);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);
}

void GLGrid::Vertex::InitVertex(int x, int z)
{
    float y = 0.0f;
    float worldScale = 1.0f;
    Pos = glm::vec3(x * worldScale, y, z * worldScale);

    float size = 1.0f; // (float)pTerrain->GetSize()
    float textureScale = 1.0f;
    Tex = glm::vec2(textureScale * (float)x / size, textureScale * (float)z / size);
}

void GLGrid::InitVertices(std::vector<Vertex> &vertices)
{
    int index = 0;

    for (int z = 0; z < m_depth; z++)
    {
        for (int x = 0; x < m_width; x++)
        {
            assert(index < vertices.size());
            vertices[index].InitVertex(x, z);
            index++;
        }
    }

    assert(index == vertices.size());
}

void GLGrid::InitIndices(std::vector<u_int> &indices)
{
    int index = 0;

    for (int z = 0; z < m_depth - 1; z++)
    {
        for (int x = 0; x < m_width - 1; x++)
        {
            u_int indexBottomLeft = z * m_width + x;
            u_int indexTopLeft = (z + 1) * m_width + x;
            u_int indexTopRight = (z + 1) * m_width + x + 1;
            u_int indexBottomRight = z * m_width + x + 1;

            // add top left triangle
            assert(index < indices.size());
            indices[index++] = indexBottomLeft;
            assert(index < indices.size());
            indices[index++] = indexTopLeft;
            assert(index < indices.size());
            indices[index++] = indexTopRight;

            // add bottom right triangle
            assert(index < indices.size());
            indices[index++] = indexBottomLeft;
            assert(index < indices.size());
            indices[index++] = indexTopRight;
            assert(index < indices.size());
            indices[index++] = indexBottomRight;
        }
    }

    assert(index == indices.size());
}

void GLGrid::Render()
{
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, (m_depth - 1) * (m_width - 1) * 6, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);
}
