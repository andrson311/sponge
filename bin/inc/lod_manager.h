#pragma once

#include "math_3d.h"
#include "array_2d.h"

class LODManager
{
public:
    int InitLODManager(int PatchSize, int NumPatchesX, int NumPatchesZ, float WorldScale);
    void Update(const glm::vec3& CameraPos);

    struct PatchLOD {
        int Core = 0;
        int Left = 0;
        int Right = 0;
        int Top = 0;
        int Bottom = 0;
    };

    const PatchLOD& GetPatchLOD(int PatchX, int PatchZ) const;

private:
    void CalcLODRegions();
    void CalcMaxLOD();
    void UpdateLODMapPass1(const glm::vec3& CameraPos);
    void UpdateLODMapPass2(const glm::vec3& CameraPos);

    int DistanceToLOD(float Distance);

    int m_maxLOD = 0;
    int m_patchSize = 0;
    int m_numPatchesX = 0;
    int m_numPatchesZ = 0;
    float m_worldScale = 0.0f;

    Array2D<PatchLOD> m_map;
    std::vector<int> m_regions;
};
