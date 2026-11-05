#pragma once

#include <glm/glm.hpp>
#include "types.h"

struct TerrainVertex
{
    glm::vec2 position;
    glm::vec2 texCoords;
};

void CreateTerrainGridVectors(int width, int height, std::vector<TerrainVertex> &vertices, std::vector<u_int> &indices);
