#pragma once

#include "math_3d.h"
#include "util.h"
#include "technique.h"

class TerrainTechnique : public Technique
{
public:
    TerrainTechnique() = default;

    virtual bool Init();

    void SetWVP(const glm::mat4 &WVP) { m_WVP.Set(WVP); }
    void SetMaxTerrainHeight(float maxHeight) { m_maxTerrainHeight.Set(maxHeight); }
    void SetHorizontalScale(float horizontalScale) { m_horizontalScale.Set(horizontalScale); }
    void SetHeightPercents(float lowPercent, float highPercent)
    {
        m_lowHeightPercent.Set(lowPercent);
        m_highHeightPercent.Set(highPercent);
    };
    void SetSunlightDir(const glm::vec3 &sunlightDir) { m_sunlightDir.Set(sunlightDir); }
    void SetAmbientLightFactor(float ambientLightFactor) { m_ambientFactor.Set(ambientLightFactor); }
    void SetTexTileSizeInWorldUnits(float texTileSizeInWorldUnits) { m_texCoordScale.Set(texTileSizeInWorldUnits); }
    void SetRenderMode(TERRAIN_RENDER_MODE renderMode) { m_renderMode.Set(renderMode); };

private:
    Uniform<glm::mat4> m_WVP;
    Uniform<float> m_maxTerrainHeight;
    Uniform<float> m_horizontalScale;
    Uniform<float> m_lowHeightPercent;
    Uniform<float> m_highHeightPercent;
    Uniform<glm::vec3> m_sunlightDir;
    Uniform<float> m_ambientFactor;
    Uniform<float> m_texCoordScale;
    Uniform<TERRAIN_RENDER_MODE> m_renderMode;
};