#pragma once

#include <string>
#include "gl_skybox_technique.h"
#include "cubemap_texture.h"

class Skybox
{
public:
    Skybox() = default;
    ~Skybox() = default;

    void Init(GLenum textureUnit, int textureUnitIndex);
    void Render(BaseCubemapTexture *pSkyboxTex, const glm::mat4 &VP);

private:
    SkyboxTechnique m_skyboxTech;
    GLenum m_textureUnit = 0;
    GLuint m_dummyVAO = -1;
};
