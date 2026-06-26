#pragma once

#include "camera.h"
#include "array_2d.h"
#include "texture.h"
#include "traingle_list.h"
#include "terrain_technique.h"
#include "single_tex_terrain_technique.h"

class BaseTerrain
{
public:
    BaseTerrain() {}
    ~BaseTerrain();

    void Destroy();
    void InitTerrain(float WorldScale, float TextureScale, const std::vector<std::string>& TextureFilenames);
    void InitTerrain(float WorldScale, float TextureScale);
    void Render(const Camera &Camera);
    void LoadFromFile(const char *pFilename);
    void SaveToFile(const char *pFilename);
    float GetHeight(int x, int z) const { return m_heightMap.Get(x, z); }
    float GetHeightInterpolated(float x, float z) const;
    float GetWorldScale() const { return m_worldScale; }
    float GetTextureScale() const { return m_textureScale; }
    int GetSize() const { return m_terrainSize; }
    void SetTexture(Texture* pTexture) { m_pTextures[0] = pTexture; }
    void SetTextureHeights(float Tex0Height, float Tex1Height, float Tex2Height, float Tex3Height);

protected:
    void LoadHeightMapFile(const char *pFilename);
    void SetMinMaxHeight(float MinHeight, float MaxHeight);

    int m_terrainSize = 0;
    float m_worldScale = 1.0f;
    Array2D<float> m_heightMap;
    TriangleList m_triangleList;
    Texture* m_pTextures[4] = { 0 };
    float m_textureScale = 1.0f;
    bool m_isSingleTexTerrain = false;

private:
    float m_minHeight = 0.0f;
    float m_maxHeight = 0.0f;
    TerrainTechnique m_terrainTech;
    SingleTexTerrainTechnique m_singleTexTerrainTech;
};