#include <vector>
#include "terrain_grid.h"

void CreateTerrainGridVectors(int width, int height, std::vector<TerrainVertex> &vertices, std::vector<u_int> &indices)
{
    float baseCol = -width / 2.0f;
    float baseRow = -height / 2.0f;

    for (int row = 0; row < height; ++row)
    {
        for (int col = 0; col < width; ++col)
        {
            TerrainVertex vertex;

            vertex.position.x = baseCol + (float)(col);
            vertex.position.y = baseRow + (float)(row);

            vertex.texCoords.x = (float)(col) / (float)(width - 1);
            vertex.texCoords.y = (float)(row) / (float)(height - 1);

            vertices.push_back(vertex);
        }
    }

    for (int row = 0; row < height - 1; ++row)
    {
        for (int col = 0; col < width - 1; ++col)
        {
            u_int topLeft = row * width + col;
            u_int topRight = topLeft + 1;
            u_int bottomLeft = (row + 1) * width + col;
            u_int bottomRight = bottomLeft + 1;

            // triangle 1
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // triangle 2
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
}
