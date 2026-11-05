#pragma once

#include "math_3d.h"
#include "texture.h"

enum TEXTURE_TYPE
{
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

enum MaterialType: u_int
{
    MaterialType_Invalid = 0,
    MaterialType_MetallicRoughness = 0x1,
    MaterialType_SpecularGlossiness = 0x2,
    MaterialType_Sheen = 0x4,
    MaterialType_ClearCoat = 0x8,
    MaterialType_Specular = 0x10,
    MaterialType_Transmission = 0x20,
    MaterialType_Volume = 0x40,
    MaterialType_Unlit = 0x80
};

class CoreMaterial
{
public:
    std::string m_name;
    bool m_isPBR = false;
    int m_materialType = MaterialType_Invalid;

    glm::vec4 AmbientColor = glm::vec4(0.0f);
    glm::vec4 DiffuseColor = glm::vec4(0.0f);
    glm::vec4 SpecularColor = glm::vec4(0.0f);
    glm::vec4 BaseColor = glm::vec4(0.0f);
    glm::vec4 EmissiveColor = glm::vec4(0.0f);
    glm::vec4 MetallicRoughnessNormalOcclusion = glm::vec4(1.0f);
    glm::vec4 ClearCoatTransmissionThickness = glm::vec4(1.0f);

    Texture *pTextures[TEX_TYPE_NUM] = {0};

    float m_transparencyFactor = 1.0f;
    float m_alphaTest = 0.0f;
    float m_ior = 1.5f;

    ~CoreMaterial()
    {
        for (Texture *pTex : pTextures)
        {
            delete pTex;
        }
    }
};
