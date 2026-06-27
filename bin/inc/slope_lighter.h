#pragma once

#include "array_2d.h"
#include "util.h"

class SlopeLighter
{
public:
    SlopeLighter(const Array2D<float>* pHeightmap) : m_pHeightmap(pHeightmap) {};

    void InitLighter(const glm::vec3& LightDir, int TerrainSize, float Softness);
    float GetLighting(int x, int z) const;

private:
    const Array2D<float>* m_pHeightmap = NULL;

    int m_terrainSize = 0;
    float m_softness = 0.0f;

    int m_dz0 = 0;
    int m_dx0 = 0;
    int m_dz1 = 0;
    int m_dx1 = 0;
    float m_factor = 0.0f;
};
