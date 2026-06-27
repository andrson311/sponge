#pragma once

#include "camera.h"
#include "array_2d.h"
#include "texture.h"
#include "traingle_list.h"
#include "terrain_technique.h"
#include "single_tex_terrain_technique.h"
#include "slope_lighter.h"

class BaseTerrain
{
public:
    BaseTerrain() : m_slopeLighter(&m_heightMap) {}
    ~BaseTerrain();

    void Destroy();
    void InitTerrain(
        float WorldScale,
        float TextureScale,
        const std::vector<std::string> &TextureFilenames,
        const glm::vec3 &LightDir,
        float LightSoftness);
    void InitTerrain(float WorldScale, float TextureScale);
    void Render(const Camera &Camera);
    void LoadFromFile(const char *pFilename);
    void SaveToFile(const char *pFilename);
    float GetHeight(int x, int z) const { return m_heightMap.Get(x, z); }
    float GetHeightInterpolated(float x, float z) const;
    float GetWorldScale() const { return m_worldScale; }
    float GetTextureScale() const { return m_textureScale; }
    int GetSize() const { return m_terrainSize; }
    void SetTexture(Texture *pTexture) { m_pTextures[0] = pTexture; }
    void SetTextureHeights(float Tex0Height, float Tex1Height, float Tex2Height, float Tex3Height);
    void SetLightDir(const glm::vec3 &Dir) { m_lightDir = Dir; }
    float GetSlopeLighting(int x, int z) const;
    void SetLight(const glm::vec3 &LightDir, float Softness);

protected:
    void LoadHeightMapFile(const char *pFilename);
    void SetMinMaxHeight(float MinHeight, float MaxHeight);
    void FinalizeTerrain();

    int m_terrainSize = 0;
    float m_worldScale = 1.0f;
    Array2D<float> m_heightMap;
    Texture *m_pTextures[4] = {0};
    float m_textureScale = 1.0f;
    bool m_isSingleTexTerrain = false;

private:
    float m_minHeight = 0.0f;
    float m_maxHeight = 0.0f;
    TerrainTechnique m_terrainTech;
    TriangleList m_triangleList;
    SlopeLighter m_slopeLighter;
    SingleTexTerrainTechnique m_singleTexTerrainTech;
    glm::vec3 m_lightDir;
    float m_lightSoftness = 0.0f;
};