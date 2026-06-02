#pragma once

#include <glm/glm.hpp>
#include "texture.h"

class Material
{
public:
    glm::vec3 AmbientColor = glm::vec3(0.0f);
    glm::vec3 DiffuseColor = glm::vec3(0.0f);
    glm::vec3 SpecularColor = glm::vec3(0.0f);

    Texture *pDiffuse = NULL;
    Texture *pSpecular = NULL;
};
