#include <stdio.h>
#include "geomip_grid.h"
#include "terrain.h"

GeomipGrid::~GeomipGrid()
{
    Destroy();
}

void GeomipGrid::Destroy()
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

void GeomipGrid::CreateGeomipGrid(int Width, int Depth, int PatchSize, const BaseTerrain *pTerrain)
{
    if ((Width - 1) % (PatchSize - 1) != 0)
    {
        int RecommendedWidth = ((Width - 1 + PatchSize - 1) / (PatchSize - 1)) * (PatchSize - 1) + 1;
        printf("Width minus 1 (%d) must be divisible by PatchSize minus 1 (%d)\n", Width, PatchSize);
        printf("Try using Width = %d\n", RecommendedWidth);
        exit(0);
    }

    if ((Depth - 1) % (PatchSize - 1) != 0)
    {
        int RecommendedDepth = ((Depth - 1 + PatchSize - 1) / (PatchSize - 1)) * (PatchSize - 1) + 1;
        printf("Depth minus 1 (%d) must be divisible by PatchSize minus 1 (%d)\n", Depth, PatchSize);
        printf("Try using Depth = %d\n", RecommendedDepth);
        exit(0);
    }

    if (PatchSize < 3)
    {
        printf("The minimum patch size is 3\n");
        exit(0);
    }

    if (PatchSize % 2 == 0)
    {
        printf("Patch size must be an odd number\n");
    }

    m_width = Width;
    m_depth = Depth;
    m_patchSize = PatchSize;
    m_pTerrain = pTerrain;

    m_numPatchesX = (Width - 1) / (PatchSize - 1);
    m_numPatchesZ = (Depth - 1) / (PatchSize - 1);

    m_worldScale = pTerrain->GetWorldScale();
    m_maxLOD = m_lodManager.InitLODManager(PatchSize, m_numPatchesX, m_numPatchesZ, m_worldScale);
    m_lodInfo.resize(m_maxLOD + 1);

    CreateGLState();
    PopulateBuffers(pTerrain);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GeomipGrid::CreateGLState()
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

void GeomipGrid::PopulateBuffers(const BaseTerrain *pTerrain)
{
    std::vector<Vertex> Vertices;
    Vertices.resize(m_width * m_depth);
    InitVertices(pTerrain, Vertices);

    int NumIndices = CalcNumIndices();
    std::vector<u_int> Indices;
    Indices.resize(NumIndices);
    NumIndices = InitIndices(Indices);

    CalcNormals(Vertices, Indices);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * NumIndices, &Indices[0], GL_STATIC_DRAW);
}

int GeomipGrid::CalcNumIndices()
{
    int NumQuads = (m_patchSize - 1) * (m_patchSize - 1);
    int NumIndices = 0;
    int MaxPermutationsPerLevel = 16;
    const int IndicesPerQuad = 6;
    for (int lod = 0; lod <= m_maxLOD; lod++)
    {
        printf("LOD %d: num quads %d\n", lod, NumQuads);
        NumIndices += NumQuads * IndicesPerQuad * MaxPermutationsPerLevel;
        NumQuads /= 4;
    }

    printf("Initial number of indices %d\n", NumIndices);
    return NumIndices;
}

void GeomipGrid::Vertex::InitVertex(const BaseTerrain *pTerrain, int x, int z)
{
    float y = pTerrain->GetHeight(x, z);

    float WorldScale = pTerrain->GetWorldScale();
    Pos = glm::vec3(x * WorldScale, y, z * WorldScale);

    float Size = (float)pTerrain->GetSize();
    float TextureScale = pTerrain->GetTextureScale();
    Tex = glm::vec2(TextureScale * (float)x / Size, TextureScale * (float)z / Size);
}

void GeomipGrid::InitVertices(const BaseTerrain *pTerrain, std::vector<Vertex> &Vertices)
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

int GeomipGrid::InitIndices(std::vector<u_int> &Indices)
{
    int Index = 0;

    for (int lod = 0; lod <= m_maxLOD; lod++)
    {
        Index = InitIndicesLOD(Index, Indices, lod);
    }

    return Index;
}

int GeomipGrid::InitIndicesLOD(int Index, std::vector<u_int> &Indices, int lod)
{
    int TotalIndicesForLOD = 0;

    for (int l = 0; l < LEFT; l++)
    {
        for (int r = 0; r < RIGHT; r++)
        {
            for (int t = 0; t < TOP; t++)
            {
                for (int b = 0; b < BOTTOM; b++)
                {
                    m_lodInfo[lod].info[l][r][t][b].Start = Index;
                    Index = InitIndicesLODSingle(Index, Indices, lod, lod + l, lod + r, lod + t, lod + b);

                    m_lodInfo[lod].info[l][r][t][b].Count = Index - m_lodInfo[lod].info[l][r][t][b].Start;
                    TotalIndicesForLOD += m_lodInfo[lod].info[l][r][t][b].Count;
                }
            }
        }
    }

    return Index;
}

int GeomipGrid::InitIndicesLODSingle(int Index, std::vector<u_int> &Indices,
                                     int lodCore, int lodLeft, int lodRight, int lodTop, int lodBottom)
{
    int FanStep = (int)powf(2.0f, (float)(lodCore + 1));
    int EndPos = m_patchSize - 1 - FanStep;

    for (int z = 0; z <= EndPos; z += FanStep)
    {
        for (int x = 0; x <= EndPos; x += FanStep)
        {
            int lLeft = x == 0 ? lodLeft : lodCore;
            int lRight = x == EndPos ? lodRight : lodCore;
            int lBottom = z == 0 ? lodBottom : lodCore;
            int lTop = z == EndPos ? lodTop : lodCore;

            Index = CreateTriangleFan(Index, Indices, lodCore, lLeft, lRight, lTop, lBottom, x, z);
        }
    }

    return Index;
}

u_int GeomipGrid::CreateTriangleFan(int Index, std::vector<u_int> &Indices,
                                    int lodCore, int lodLeft, int lodRight, int lodTop, int lodBottom, int x, int z)
{
    int StepLeft = (int)powf(2.0f, (float)lodLeft);
    int StepRight = (int)powf(2.0f, (float)lodRight);
    int StepTop = (int)powf(2.0f, (float)lodTop);
    int StepBottom = (int)powf(2.0f, (float)lodBottom);
    int StepCenter = (int)powf(2.0f, (float)lodCore);

    u_int IndexCenter = (z + StepCenter) * m_width + x + StepCenter;

    // first up
    u_int IndexTemp1 = z * m_width + x;
    u_int IndexTemp2 = (z + StepLeft) * m_width + x;

    Index = AddTriangle(Index, Indices, IndexCenter, IndexTemp1, IndexTemp2);

    // second up
    if (lodLeft == lodCore)
    {
        IndexTemp1 = IndexTemp2;
        IndexTemp2 += StepLeft * m_width;

        Index = AddTriangle(Index, Indices, IndexCenter, IndexTemp1, IndexTemp2);
    }

    // first right
    IndexTemp1 = IndexTemp2;
    IndexTemp2 += StepTop;

    Index = AddTriangle(Index, Indices, IndexCenter, IndexTemp1, IndexTemp2);

    // second right
    if (lodTop == lodCore)
    {
        IndexTemp1 = IndexTemp2;
        IndexTemp2 += StepTop;

        Index = AddTriangle(Index, Indices, IndexCenter, IndexTemp1, IndexTemp2);
    }

    // first down
    IndexTemp1 = IndexTemp2;
    IndexTemp2 -= StepRight * m_width;

    Index = AddTriangle(Index, Indices, IndexCenter, IndexTemp1, IndexTemp2);

    // second down
    if (lodRight == lodCore)
    {
        IndexTemp1 = IndexTemp2;
        IndexTemp2 -= StepRight * m_width;

        Index = AddTriangle(Index, Indices, IndexCenter, IndexTemp1, IndexTemp2);
    }

    // first left
    IndexTemp1 = IndexTemp2;
    IndexTemp2 -= StepBottom;

    Index = AddTriangle(Index, Indices, IndexCenter, IndexTemp1, IndexTemp2);

    // second left
    if (lodBottom == lodCore)
    {
        IndexTemp1 = IndexTemp2;
        IndexTemp2 -= StepBottom;

        Index = AddTriangle(Index, Indices, IndexCenter, IndexTemp1, IndexTemp2);
    }

    return Index;
}

u_int GeomipGrid::AddTriangle(u_int Index, std::vector<u_int> &Indices, u_int v1, u_int v2, u_int v3)
{
    assert(Index < Indices.size());
    Indices[Index++] = v1;
    assert(Index < Indices.size());
    Indices[Index++] = v2;
    assert(Index < Indices.size());
    Indices[Index++] = v3;

    return Index;
}

void GeomipGrid::CalcNormals(std::vector<Vertex> &Vertices, std::vector<u_int> &Indices)
{
    u_int Index = 0;

    for (int z = 0; z < m_depth - 1; z += (m_patchSize - 1))
    {
        for (int x = 0; x < m_width - 1; x += (m_patchSize - 1))
        {
            int BaseVertex = z * m_width + x;
            int NumIndices = m_lodInfo[0].info[0][0][0][0].Count;

            for (u_int i = 0; i < NumIndices; i += 3)
            {
                u_int Index0 = BaseVertex + Indices[i];
                u_int Index1 = BaseVertex + Indices[i + 1];
                u_int Index2 = BaseVertex + Indices[i + 2];

                glm::vec3 v1 = Vertices[Index1].Pos - Vertices[Index0].Pos;
                glm::vec3 v2 = Vertices[Index2].Pos - Vertices[Index0].Pos;

                glm::vec3 Normal = glm::normalize(glm::cross(v1, v2));

                Vertices[Index0].Normal += Normal;
                Vertices[Index1].Normal += Normal;
                Vertices[Index2].Normal += Normal;
            }
        }
    }

    for (u_int i = 0; i < Vertices.size(); i++)
    {
        Vertices[i].Normal = glm::normalize(Vertices[i].Normal);
    }
}

void GeomipGrid::Render(const glm::vec3 &CameraPos, const glm::mat4 &ViewProj)
{
    m_lodManager.Update(CameraPos);
    FrustumCulling fc(ViewProj);
    glBindVertexArray(m_vao);

    float PatchSize = ((float)m_patchSize - 1.0f) * m_worldScale;
    float HalfPatchSize = PatchSize / 2.0f;

    for (int PatchZ = 0; PatchZ < m_numPatchesZ; PatchZ++)
    {
        for (int PatchX = 0; PatchX < m_numPatchesX; PatchX++)
        {
            int x = PatchX * (m_patchSize - 1);
            int z = PatchZ * (m_patchSize - 1);

            // if (!IsPatchInsideViewFrustum_ViewSpace(x, z, ViewProj))
            // if (!IsPatchInsideViewFrustum_WorldSpace(x, z, fc))
            if (!IsPatchInsideViewFrustum_AABB(x, z, fc))
            {
                continue;
            }

            const LODManager::PatchLOD &plod = m_lodManager.GetPatchLOD(PatchX, PatchZ);
            int C = plod.Core;
            int L = plod.Left;
            int R = plod.Right;
            int T = plod.Top;
            int B = plod.Bottom;

            size_t BaseIndex = sizeof(u_int) * m_lodInfo[C].info[L][R][T][B].Start;

            int BaseVertex = z * m_width + x;

            glDrawElementsBaseVertex(GL_TRIANGLES, m_lodInfo[C].info[L][R][T][B].Count,
                                     GL_UNSIGNED_INT, (void *)BaseIndex, BaseVertex);
        }
    }

    glBindVertexArray(0);
}

bool GeomipGrid::IsPatchInsideViewFrustum_ViewSpace(int x, int z, const glm::mat4 &ViewProj)
{
    int x0 = x;
    int x1 = x + m_patchSize - 1;
    int z0 = z;
    int z1 = z + m_patchSize - 1;

    glm::vec3 p00((float)x0 * m_worldScale, m_pTerrain->GetHeight(x0, z0), (float)z0 * m_worldScale);
    glm::vec3 p01((float)x0 * m_worldScale, m_pTerrain->GetHeight(x0, z1), (float)z1 * m_worldScale);
    glm::vec3 p10((float)x1 * m_worldScale, m_pTerrain->GetHeight(x1, z0), (float)z0 * m_worldScale);
    glm::vec3 p11((float)x1 * m_worldScale, m_pTerrain->GetHeight(x1, z1), (float)z1 * m_worldScale);

    bool InsideViewFrustum =
        IsPointInsideViewFrustum(p00, ViewProj) ||
        IsPointInsideViewFrustum(p01, ViewProj) ||
        IsPointInsideViewFrustum(p10, ViewProj) ||
        IsPointInsideViewFrustum(p11, ViewProj);

    return InsideViewFrustum;
}

bool GeomipGrid::IsPatchInsideViewFrustum_WorldSpace(int x, int z, const FrustumCulling &fc)
{
    int x0 = x;
    int x1 = x + m_patchSize - 1;
    int z0 = z;
    int z1 = z + m_patchSize - 1;

    float h00 = m_pTerrain->GetHeight(x0, z0);
    float h01 = m_pTerrain->GetHeight(x0, z1);
    float h10 = m_pTerrain->GetHeight(x1, z0);
    float h11 = m_pTerrain->GetHeight(x1, z1);

    float MinHeight = std::min(h00, std::min(h01, std::min(h10, h11)));
    float MaxHeight = std::max(h00, std::max(h01, std::max(h10, h11)));

    glm::vec3 p00_low((float)x0 * m_worldScale, MinHeight, (float)z0 * m_worldScale);
    glm::vec3 p01_low((float)x0 * m_worldScale, MinHeight, (float)z1 * m_worldScale);
    glm::vec3 p10_low((float)x1 * m_worldScale, MinHeight, (float)z0 * m_worldScale);
    glm::vec3 p11_low((float)x1 * m_worldScale, MinHeight, (float)z1 * m_worldScale);

    glm::vec3 p00_high((float)x0 * m_worldScale, MaxHeight, (float)z0 * m_worldScale);
    glm::vec3 p01_high((float)x0 * m_worldScale, MaxHeight, (float)z1 * m_worldScale);
    glm::vec3 p10_high((float)x1 * m_worldScale, MaxHeight, (float)z0 * m_worldScale);
    glm::vec3 p11_high((float)x1 * m_worldScale, MaxHeight, (float)z1 * m_worldScale);

    bool InsideViewFrustum =
        fc.IsPointInsideViewFrustum(p00_low) ||
        fc.IsPointInsideViewFrustum(p01_low) ||
        fc.IsPointInsideViewFrustum(p10_low) ||
        fc.IsPointInsideViewFrustum(p11_low) ||
        fc.IsPointInsideViewFrustum(p00_high) ||
        fc.IsPointInsideViewFrustum(p01_high) ||
        fc.IsPointInsideViewFrustum(p10_high) ||
        fc.IsPointInsideViewFrustum(p11_high);

    return InsideViewFrustum;
}

bool GeomipGrid::IsCameraInPatch(const glm::vec3 &CameraPos, int x, int z)
{
    float x0 = (float)(x - 2 * m_patchSize) * m_worldScale;
    float x1 = (float)(x + 2 * m_patchSize) * m_worldScale;
    float z0 = (float)(z - 2 * m_patchSize) * m_worldScale;
    float z1 = (float)(z + 2 * m_patchSize) * m_worldScale;

    bool CameraInPatch = (CameraPos.x >= x0) &&
                         (CameraPos.x <= x1) &&
                         (CameraPos.z >= z0) &&
                         (CameraPos.z <= z1);

    return CameraInPatch;
}

bool GeomipGrid::IsPatchInsideViewFrustum_AABB(int x, int z, FrustumCulling &fc)
{
    int x0 = x;
    int x1 = x + m_patchSize - 1;
    int z0 = z;
    int z1 = z + m_patchSize - 1;

    float h00 = m_pTerrain->GetHeight(x0, z0);
    float h01 = m_pTerrain->GetHeight(x0, z1);
    float h10 = m_pTerrain->GetHeight(x1, z0);
    float h11 = m_pTerrain->GetHeight(x1, z1);

    float MinHeight = std::min(h00, std::min(h01, std::min(h10, h11)));
    float MaxHeight = std::max(h00, std::max(h01, std::max(h10, h11)));

    glm::vec3 MinCorner((float)x0 * m_worldScale, MinHeight, (float)z0 * m_worldScale);
    glm::vec3 MaxCorner((float)x1 * m_worldScale, MaxHeight, (float)z1 * m_worldScale);

    return fc.IsAABBInsideViewFrustum(MinCorner, MaxCorner);
}
