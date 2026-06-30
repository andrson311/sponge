#pragma once

#include <string>
#include <vector>

#include "camera.h"
#include "array_2d.h"
#include "texture.h"
#include "geomip_grid.h"
#include "terrain_technique.h"

class BaseTerrain
{
public:
    BaseTerrain() {}
    ~BaseTerrain();

    void Destroy();
    void InitTerrain(float WorldScale, float TextureScale, const std::vector<std::string> &TextureFilenames);
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
    float GetMaxHeight() const { return m_maxHeight; }
    float GetWorldSize() const { return m_terrainSize * m_worldScale; }
    glm::vec3 ConstrainCameraPosToTerrain(const glm::vec3 &CameraPos);

protected:
    void LoadHeightMapFile(const char *pFilename);
    void SetMinMaxHeight(float MinHeight, float MaxHeight);
    void Finalize();
    float GetWorldHeight(float x, float z) const;

    int m_terrainSize = 0;
    int m_patchSize = 0;
    float m_worldScale = 1.0f;
    Array2D<float> m_heightMap;
    Texture *m_pTextures[4] = {0};
    float m_textureScale = 1.0f;

private:
    GeomipGrid m_geomipGrid;
    float m_minHeight = 0.0f;
    float m_maxHeight = 0.0f;
    TerrainTechnique m_terrainTech;
    glm::vec3 m_lightDir;
    float m_cameraHeight = 2.0f;
};