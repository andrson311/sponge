#pragma once

#include <glm/glm.hpp>
#include "texture.h"

#define INVALID_MATERIAL 0xFFFFFFFF

enum TEXTURE_TYPE {
    TEX_TYPE_BASE = 0,
    TEX_TYPE_SPECULAR = 1,
    TEX_TYPE_NORMAL = 2,
    TEX_TYPE_METALNESS = 3,
    TEX_TYPE_EMISSIVE = 4,
    TEX_TYPE_NORMAL_CAMERA = 5,
    TEX_TYPE_EMISSION_COLOR = 6,
    TEX_TYPE_ROUGHNESS = 7,
    TEX_TYPE_AMBIENT_OCCLUSION = 8,
    TEX_TYPE_CLEARCOAT = 9,
    TEX_TYPE_CLEARCOAT_ROUGHNESS = 10,
    TEX_TYPE_CLEARCOAT_NORMAL = 11,
    TEX_TYPE_NUM = 12
};

class Material
{
public:
    glm::vec4 AmbientColor = glm::vec4(0.0f);
    glm::vec4 DiffuseColor = glm::vec4(0.0f);
    glm::vec4 SpecularColor = glm::vec4(0.0f);
    glm::vec4 BaseColor = glm::vec4(0.0f);
    glm::vec4 EmissiveColor = glm::vec4(0.0f);
    glm::vec4 MetallicRoughnessNormalOcclusion = glm::vec4(0.0f);
    glm::vec4 ClearCoatTransmissionThickness = glm::vec4(0.0f);

    Texture* pTextures[TEX_TYPE_NUM] = { 0 };

    ~Material()
    {
        for (Texture* pTex : pTextures) {
            delete pTex;
        }
    }
};
