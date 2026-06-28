#include <stdio.h>

#include "lod_manager.h"

int LODManager::InitLODManager(int PatchSize, int NumPatchesX, int NumPatchesZ, float WorldScale)
{
    m_patchSize = PatchSize;
    m_numPatchesX = NumPatchesX;
    m_numPatchesZ = NumPatchesZ;
    m_worldScale = WorldScale;

    CalcMaxLOD();

    PatchLOD Zero;
    m_map.InitArray2D(NumPatchesX, NumPatchesZ, Zero);
    m_regions.resize(m_maxLOD + 1);

    CalcLODRegions();

    return m_maxLOD;
}

void LODManager::CalcMaxLOD()
{
    int NumSegments = m_patchSize - 1;
    if (ceilf(log2f((float)NumSegments)) != floorf(log2f((float)NumSegments)))
    {
        printf("The number of vertices in the patch minus one must be a power of two\n");
        printf("%f %f\n", ceilf(log2f((float)NumSegments)), floorf(log2f((float)NumSegments)));
        exit(0);
    }

    int patchSizeLog2 = (int)log2f((float)NumSegments);
    printf("log2 of patch size %d is %d\n", m_patchSize, patchSizeLog2);
    m_maxLOD = patchSizeLog2 - 1;
}

void LODManager::Update(const glm::vec3 &CameraPos)
{
    UpdateLODMapPass1(CameraPos);
    UpdateLODMapPass2(CameraPos);
}

void LODManager::UpdateLODMapPass1(const glm::vec3 &CameraPos)
{
    int CenterStep = m_patchSize / 2;

    for (int LODMapZ = 0; LODMapZ < m_numPatchesZ; LODMapZ++)
    {
        for (int LODMapX = 0; LODMapX < m_numPatchesX; LODMapX++)
        {
            int x = LODMapX * (m_patchSize - 1) + CenterStep;
            int z = LODMapZ * (m_patchSize - 1) + CenterStep;

            glm::vec3 PatchCenter = glm::vec3(x * (float)m_worldScale, 0.0f, z * (float)m_worldScale);

            float DistanceToCamera = glm::distance(CameraPos, PatchCenter);
            int CoreLOD = DistanceToLOD(DistanceToCamera);

            PatchLOD *pPatchLOD = m_map.GetAddr(LODMapX, LODMapZ);
            pPatchLOD->Core = CoreLOD;
        }
    }
}

void LODManager::UpdateLODMapPass2(const glm::vec3 &CameraPos)
{
    int Step = m_patchSize / 2;

    for (int LODMapZ = 0; LODMapZ < m_numPatchesZ; LODMapZ++)
    {
        for (int LODMapX = 0; LODMapX < m_numPatchesX; LODMapX++)
        {
            int CoreLOD = m_map.Get(LODMapX, LODMapZ).Core;

            int IndexLeft = LODMapX;
            int IndexRight = LODMapX;
            int IndexTop = LODMapZ;
            int IndexBottom = LODMapZ;

            if (LODMapX > 0)
            {
                IndexLeft--;

                if (m_map.Get(IndexLeft, LODMapZ).Core > CoreLOD)
                {
                    m_map.At(LODMapX, LODMapZ).Left = 1;
                }
                else
                {
                    m_map.At(LODMapX, LODMapZ).Left = 0;
                }
            }

            if (LODMapX < m_numPatchesX - 1)
            {
                IndexRight++;

                if (m_map.Get(IndexRight, LODMapZ).Core > CoreLOD)
                {
                    m_map.At(LODMapX, LODMapZ).Right = 1;
                }
                else
                {
                    m_map.At(LODMapX, LODMapZ).Right = 0;
                }
            }

            if (LODMapZ > 0)
            {
                IndexBottom--;

                if (m_map.Get(LODMapX, IndexBottom).Core > CoreLOD)
                {
                    m_map.At(LODMapX, LODMapZ).Bottom = 1;
                }
                else
                {
                    m_map.At(LODMapX, LODMapZ).Bottom = 0;
                }
            }

            if (LODMapZ < m_numPatchesZ - 1)
            {
                IndexTop++;

                if (m_map.Get(LODMapX, IndexTop).Core > CoreLOD)
                {
                    m_map.At(LODMapX, LODMapZ).Top = 1;
                }
                else
                {
                    m_map.At(LODMapX, LODMapZ).Top = 0;
                }
            }
        }
    }
}

int LODManager::DistanceToLOD(float Distance)
{
    int LOD = m_maxLOD;

    for (int i = 0; i <= m_maxLOD; i++)
    {
        if (Distance < m_regions[i])
        {
            LOD = i;
            break;
        }
    }

    return LOD;
}

const LODManager::PatchLOD &LODManager::GetPatchLOD(int PatchX, int PatchZ) const
{
    return m_map.Get(PatchX, PatchZ);
}

void LODManager::CalcLODRegions()
{
    int Sum = 0;

    for (int i = 0; i <= m_maxLOD; i++)
    {
        Sum += (i + 1);
    }

    printf("Sum %d\n", Sum);

    float X = 5000.0f / (float)Sum;

    int Temp = 0;

    for (int i = 0; i <= m_maxLOD; i++)
    {
        int CurRange = (int)(X * (i + 1));
        m_regions[i] = Temp + CurRange;
        Temp += CurRange;
        printf("%d %d\n", i, m_regions[i]);
    }
}
