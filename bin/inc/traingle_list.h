#pragma once

#include <GL/glew.h>
#include <vector>
#include "util.h"

class BaseTerrain;

class TriangleList
{
public:
    TriangleList() {};
    ~TriangleList();

    void Destroy();
    void CreateTriangleList(int Width, int Depth, const BaseTerrain *pTerrain);
    void Render();

private:
    struct Vertex
    {
        glm::vec3 Pos;
        glm::vec2 Tex;
        glm::vec3 Normal = glm::vec3(0.0f);

        void InitVertex(const BaseTerrain *pTerrain, int x, int z);
    };

    void CreateGLState();

    void PopulateBuffers(const BaseTerrain *pTerrain);
    void InitVertices(const BaseTerrain *pTerrain, std::vector<Vertex> &Vertices);
    void InitIndices(std::vector<u_int> &Indices);
    void CalcNormals(std::vector<Vertex>& Vertices, std::vector<u_int>& Indices);

    int m_width = 0;
    int m_depth = 0;
    GLuint m_vao;
    GLuint m_vb;
    GLuint m_ib;
};
