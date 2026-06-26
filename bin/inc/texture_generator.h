#pragma once

#include <stdio.h>
#include "texture.h"
#include "class_stb_image.h"

struct TextureHeightDesc
{
    float Low = 0.0f;
    float Optimal = 0.0f;
    float High = 0.0f;
};

struct TextureTile
{
    STBImage Image;
    TextureHeightDesc HeightDesc;
};

class BaseTerrain;

class TextureGenerator
{
public:
    TextureGenerator() {}

    void LoadTile(const char* Filename);
    Texture* GenerateTexture(int TextureSize, BaseTerrain* pTerrain, float MinHeight, float MaxHeight);

private:
    void CalculateTextureRegions(float MinHeight, float MaxHeight);
    float RegionPercent(int Tile, float Height);

    #define MAX_TEXTURE_TILES 4

    TextureTile m_textureTiles[MAX_TEXTURE_TILES] = {};
    int m_numTextureTiles = 0;
};
