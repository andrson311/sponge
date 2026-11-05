#include <cstdio>
#include <cstdlib>

#include "shadow_cube_map_fbo.h"

ShadowCubeMapFBO::ShadowCubeMapFBO() = default;

ShadowCubeMapFBO::~ShadowCubeMapFBO()
{
    if (m_fbo)
        glDeleteFramebuffers(1, &m_fbo);
    if (m_shadowCubeMap)
        glDeleteTextures(1, &m_shadowCubeMap);
    if (m_depth)
        glDeleteRenderbuffers(1, &m_depth);
}

bool ShadowCubeMapFBO::Init(u_int size)
{
    if (!size)
        return false;

    m_size = size;
    glGenFramebuffers(1, &m_fbo);
    glGenTextures(1, &m_shadowCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_shadowCubeMap);
    for (int face = 0; face < 6; ++face)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_DEPTH_COMPONENT32F,
                     size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X, m_shadowCubeMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    const bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    if (!complete)
        fprintf(stderr, "Point shadow cubemap framebuffer is incomplete\n");
    return complete;
}

void ShadowCubeMapFBO::BindForWriting(GLenum CubeFace)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, CubeFace, m_shadowCubeMap, 0);
    glViewport(0, 0, m_size, m_size);
}

void ShadowCubeMapFBO::BindForReading(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_shadowCubeMap);
}
