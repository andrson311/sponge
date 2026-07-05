#pragma once

#include <GL/glew.h>
#include <vector>

#include "math_3d.h"

class BaseTerrain;

class QuadList
{
public:
    QuadList() {}
    ~QuadList();

    void CreateQuadList(int Width, int Depth, const BaseTerrain *pTerrain);
    void Destroy();
    void Render();

private:
    struct Vertex
    {
        glm::vec3 Pos;
        glm::vec2 Tex;

        void InitVertex(const BaseTerrain *pTerrain, int Width, int Depth, int x, int z);
    };

    void CreateGLState();
    void PopulateBuffers(const BaseTerrain *pTerrain);
    void InitVertices(const BaseTerrain *pTerrain, std::vector<Vertex> &Vertices);
    void InitIndices(std::vector<u_int> &Indices);

    int m_width = 0;
    int m_depth = 0;
    GLuint m_vao = 0;
    GLuint m_vb = 0;
    GLuint m_ib = 0;
};
