#pragma once

#include <vector>

struct PerlinConfig
{
    int width = 256;
    int height = 256;
    float scale = 50.0f;
    int octaves = 4;
    float lacunarity = 2.0f;
    float persistence = 0.5f;
    int seed = 0;
};

void CreatePerlinMap(const PerlinConfig &config, std::vector<float> &heightMap);
