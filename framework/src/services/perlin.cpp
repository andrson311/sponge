#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>

#include "stb_image_write.h"
#include "util.h"
#include "perlin.h"

void CreatePerlinMap(const PerlinConfig &config, std::vector<float> &heightMap)
{
    heightMap.resize(config.width * config.height);

    std::vector<glm::vec2> octaveOffsets(config.octaves);
    srand(config.seed);

    for (int i = 0; i < config.octaves; i++)
    {
        float offsetX = (float)(rand() % 200000 - 100000);
        float offsetY = (float)(rand() % 200000 - 100000);

        octaveOffsets[i] = glm::vec2(offsetX, offsetY);
    }

    float minValue = std::numeric_limits<float>::max();
    float maxValue = std::numeric_limits<float>::lowest();

    float safeScale = config.scale;
    if (safeScale <= 0.0f)
    {
        safeScale = 0.0001f;
    }

    for (int row = 0; row < config.height; row++)
    {
        for (int col = 0; col < config.width; col++)
        {
            float sum = 0.0f;
            float freq = 1.0f;
            float amplitude = 1.0f;

            for (int oct = 0; oct < config.octaves; oct++)
            {
                float sampleX = col / safeScale * freq + octaveOffsets[oct].x;
                float sampleY = row / safeScale * freq + octaveOffsets[oct].y;

                float p = glm::perlin(glm::vec2(sampleX, sampleY)) * amplitude;
                sum += p;
                freq *= config.lacunarity;
                amplitude *= config.persistence;
            }

            heightMap[row * config.width + col] = sum;

            minValue = std::min(minValue, sum);
            maxValue = std::min(maxValue, sum);
        }
    }

    if (maxValue != minValue)
    {
        float range = maxValue - minValue;
        for (int i = 0; i < heightMap.size(); i++)
        {
            float normalizedHeight = (heightMap[i] - minValue) / range;
            heightMap[i] = normalizedHeight;
        }
    }
}
