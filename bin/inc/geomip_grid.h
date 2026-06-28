#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

class BaseTerrain;

class GeomipGrid
{
public:
    GeomipGrid() {};
    ~GeomipGrid();

    void CreateGeomipGrid(int Width, int Depth, int PatchSize, const BaseTerrain *pTerrain);
    void Destroy();
    void Render();

private:
    struct Vertex
    {
        glm::vec3 Pos;
        glm::vec2 Tex;
        glm::vec3 Normal = glm::vec3(0.0f);

        void InitVertex(const BaseTerrain* pTerrain, int x, int z);
    };

    void CreateGLState();

    void PopulateBuffers(const BaseTerrain* pTerrain);
    void InitVertices(const BaseTerrain* pTerrain, std::vector<Vertex>& Vertices);
    void InitIndices(std::vector<u_int>& Indices);
    void CalcNormals(std::vector<Vertex>& Vertices, std::vector<u_int>& Indices);

    u_int AddTriangle(u_int Index, std::vector<u_int>& Indices, u_int v1, u_int v2, u_int v3);

    int m_width = 0;
    int m_depth = 0;
    int m_patchSize = 0;
    GLuint m_vao = 0;
    GLuint m_vb = 0;
    GLuint m_ib = 0;
};
