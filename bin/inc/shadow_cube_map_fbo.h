#pragma once

#include <GL/glew.h>
#include "util.h"

class ShadowCubeMapFBO
{
public:
    ShadowCubeMapFBO();
    ~ShadowCubeMapFBO();

    bool Init(u_int size);
    void BindForWriting(GLenum CubeFace);
    void BindForReading(GLenum TextureUnit);

private:
    u_int m_size = 0;
    GLuint m_fbo;
    GLuint m_shadowCubeMap;
    GLuint m_depth;
};
