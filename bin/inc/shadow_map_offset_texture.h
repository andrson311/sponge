#pragma once

#include <GL/glew.h>
#include <vector>
#include "util.h"

class ShadowMapOffsetTexture
{
public:
    ShadowMapOffsetTexture(int WindowSize, int FilterSize);

    void Bind(GLenum TextureUnit);

private:
    GLuint m_textureObj;

    void CreateTexture(int WindowSize, int FilterSize, const std::vector<float> &Data);
};