#pragma once

#include "scene.h"
#include "gl_infinite_grid_technique.h"

class InfiniteGrid
{
public:
    InfiniteGrid() = default;

    void Init(int shadowMapTextureUnit);

    void Render(const InfiniteGridConfig &config, const glm::mat4 &VP,
                const glm::vec3 &cameraPos, const glm::mat4 &lightVP,
                const glm::vec3 &lightDir);

private:
    GLuint m_dummyVAO = 0;
    InfiniteGridTechnique m_infiniteGridTech;
};
