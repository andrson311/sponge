#pragma once

#include <GL/glew.h>
#include "math_3d.h"
#include "lod_manager.h"

class BaseTerrain;

class GeomipGrid
{
public:
    GeomipGrid() {};
    ~GeomipGrid();

    void CreateGeomipGrid(int Width, int Depth, int PatchSize, const BaseTerrain *pTerrain);
    void Destroy();
    void Render(const glm::vec3 &CameraPos);

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
    int InitIndices(std::vector<u_int> &Indices);
    int InitIndicesLOD(int Index, std::vector<u_int> &Indices, int lod);
    int InitIndicesLODSingle(int Index, std::vector<u_int> &Indices,
                             int lodCore, int lodLeft, int lodRight, int lodTop, int lodBottom);
    void CalcNormals(std::vector<Vertex> &Vertices, std::vector<u_int> &Indices);

    u_int AddTriangle(u_int Index, std::vector<u_int> &Indices, u_int v1, u_int v2, u_int v3);
    u_int CreateTriangleFan(int Index, std::vector<u_int> &Indices,
                            int lodCore, int lodLeft, int lodRight, int lodTop, int lodBottom, int x, int z);

    int CalcNumIndices();

    int m_width = 0;
    int m_depth = 0;
    int m_patchSize = 0;
    int m_maxLOD = 0;
    GLuint m_vao = 0;
    GLuint m_vb = 0;
    GLuint m_ib = 0;

    struct SingleLODInfo
    {
        int Start = 0;
        int Count = 0;
    };

#define LEFT 2
#define RIGHT 2
#define TOP 2
#define BOTTOM 2

    struct LODInfo
    {
        SingleLODInfo info[LEFT][RIGHT][TOP][BOTTOM];
    };

    std::vector<LODInfo> m_lodInfo;
    int m_numPatchesX = 0;
    int m_numPatchesZ = 0;
    LODManager m_lodManager;
};
