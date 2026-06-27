#include "slope_lighter.h"

void SlopeLighter::InitLighter(const glm::vec3 &LightDir, int TerrainSize, float Softness)
{
    m_terrainSize = TerrainSize;
    m_softness = Softness;

    glm::vec3 ReversedLightDir = LightDir * -1.0f;
    ReversedLightDir.y = 0.0f;
    ReversedLightDir = glm::normalize(ReversedLightDir);

    glm::vec3 DirX(1.0f, 0.0f, 0.0f);
    float dpx = glm::dot(DirX, ReversedLightDir);
    float cosx = glm::degrees(acosf(dpx));

    glm::vec3 DirZ(0.0f, 0.0f, 1.0f);
    float dpz = glm::dot(DirZ, ReversedLightDir);
    float cosz = glm::degrees(acosf(dpz));

    float RadianOf45Degrees = cosf(glm::radians(45.0f));
    bool InterpolateOnX = false;

    if (dpz >= RadianOf45Degrees)
    {
        m_dz0 = m_dz1 = 1;
        InterpolateOnX = true;
    }
    else if (dpz <= -RadianOf45Degrees)
    {
        m_dz0 = m_dz1 = -1;
        InterpolateOnX = true;
    }
    else
    {
        if (dpz >= 0.0f)
        {
            m_dz0 = 0;
            m_dz1 = 1;
        }
        else
        {
            m_dz0 = 0;
            m_dz1 = -1;
        }

        m_factor = 1.0f - fabsf(dpz) / RadianOf45Degrees;

        if (dpx >= 0.0f)
        {
            m_dx0 = m_dx1 = 1;
        }
        else
        {
            m_dx0 = m_dx1 = -1;
        }
    }

    if (InterpolateOnX)
    {
        if (dpx >= 0.0f)
        {
            m_dx0 = 0;
            m_dx1 = 1;
        }
        else
        {
            m_dx0 = 0;
            m_dx1 = -1;
        }

        m_factor = 1.0f - fabsf(dpx) / RadianOf45Degrees;
    }
}

float SlopeLighter::GetLighting(int x, int z) const
{
    float Height = m_pHeightmap->Get(x, z);

    float f = 0.0f;

    int XBefore0 = x + m_dx0 * 5;
    int ZBefore0 = z + m_dz0 * 5;
    int XBefore1 = x + m_dx1 * 5;
    int ZBefore1 = z + m_dz1 * 5;

    bool V0InsideHeightmap = (XBefore0 >= 0) && (XBefore0 < m_terrainSize) && (ZBefore0 >= 0) && (ZBefore0 < m_terrainSize);
    bool V1InsideHeightmap = (XBefore1 >= 0) && (XBefore1 < m_terrainSize) && (ZBefore1 >= 0) && (ZBefore1 < m_terrainSize);

    float min_brightness = 0.4f;

    if (V0InsideHeightmap && V1InsideHeightmap)
    {
        float HeightBefore0 = m_pHeightmap->Get(XBefore0, ZBefore0);
        float HeightBefore1 = m_pHeightmap->Get(XBefore1, ZBefore1);

        float HeightBefore = HeightBefore0 * m_factor + (1.0f - m_factor) * HeightBefore1;
        f = (Height - HeightBefore) / m_softness;
    }
    else if (V0InsideHeightmap)
    {
        float HeightBefore = m_pHeightmap->Get(XBefore0, ZBefore0);
        f = (Height - HeightBefore) / m_softness;
    }
    else if (V1InsideHeightmap)
    {
        float HeightBefore = m_pHeightmap->Get(XBefore1, ZBefore1);
        f = (Height - HeightBefore) / m_softness;
    }
    else
    {
        f = 1.0f;
    }

    f = std::min(1.0f, std::max(f, min_brightness));

    return f;
}
