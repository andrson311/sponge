#include <vector>
#include <math.h>
#include <random>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "shadow_map_offset_texture.h"

float jitter()
{
    static std::default_random_engine generator;
    static std::uniform_real_distribution<float> distrib(-0.5f, 0.5f);
    return distrib(generator);
}

static void GenOffsetTextureData(int WindowSize, int FilterSize, std::vector<float> &Data)
{
    int BufferSize = WindowSize * WindowSize * FilterSize * FilterSize * 2;
    Data.resize(BufferSize);

    int Index = 0;

    for (int TexY = 0; TexY < WindowSize; TexY++)
    {
        for (int TexX = 0; TexX < WindowSize; TexX++)
        {
            for (int v = FilterSize - 1; v >= 0; v--)
            {
                for (int u = 0; u < FilterSize; u++)
                {
                    float x = ((float)u + 0.5f + jitter()) / (float)FilterSize;
                    float y = ((float)v + 0.5f + jitter()) / (float)FilterSize;

                    assert(Index + 1 < (int)Data.size());
                    Data[Index] = sqrtf(y) * cosf(2.0f * glm::pi<float>() * x);
                    Data[Index + 1] = sqrtf(y) * sinf(2.0f * glm::pi<float>() * x);

                    Index += 2;
                }
            }
        }
    }
}

ShadowMapOffsetTexture::ShadowMapOffsetTexture(int WindowSize, int FilterSize)
{
    std::vector<float> Data;
    GenOffsetTextureData(WindowSize, FilterSize, Data);
    CreateTexture(WindowSize, FilterSize, Data);
}

void ShadowMapOffsetTexture::CreateTexture(int WindowSize, int FilterSize, const std::vector<float> &Data)
{
    int NumFilterSamples = FilterSize * FilterSize;
    glActiveTexture(SHADOW_TEXTURE_UNIT);

    glGenTextures(1, &m_textureObj);
    glBindTexture(GL_TEXTURE_3D, m_textureObj);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA32F, NumFilterSamples / 2, WindowSize, WindowSize);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, NumFilterSamples / 2, WindowSize, WindowSize, GL_RGBA, GL_FLOAT, &Data[0]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_3D, 0);
}

void ShadowMapOffsetTexture::Bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_3D, m_textureObj);
}
