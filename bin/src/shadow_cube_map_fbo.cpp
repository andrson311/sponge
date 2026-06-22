#include <stdio.h>

#include "shadow_cube_map_fbo.h"
#include "util.h"

ShadowCubeMapFBO::ShadowCubeMapFBO()
{
    m_fbo = 0;
    m_shadowCubeMap = 0;
    m_depth = 0;
}

ShadowCubeMapFBO::~ShadowCubeMapFBO()
{
    if (m_fbo != 0)
    {
        glDeleteFramebuffers(1, &m_fbo);
    }

    if (m_shadowCubeMap != 0)
    {
        glDeleteTextures(1, &m_shadowCubeMap);
    }

    if (m_depth != 0)
    {
        glDeleteTextures(1, &m_depth);
    }
}

bool ShadowCubeMapFBO::Init(u_int size)
{
    m_size = size;

    // create the depth buffer
    glGenTextures(1, &m_depth);
    glBindTexture(GL_TEXTURE_2D, m_depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_size, m_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    // create the cube map
    glGenTextures(1, &m_shadowCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_shadowCubeMap);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    for (u_int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_R32F, m_size, m_size, 0, GL_RED, GL_FLOAT, NULL);
    }

    // create the FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth, 0);

    // disable writes to the color buffer
    glDrawBuffer(GL_NONE);

    // disable reads from the color buffer
    glReadBuffer(GL_NONE);

    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("FB error, status 0x%x\n", Status);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return true;
}

void ShadowCubeMapFBO::BindForWriting(GLenum CubeFace)
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_size, m_size);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, CubeFace, m_shadowCubeMap, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
}

void ShadowCubeMapFBO::BindForReading(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_shadowCubeMap);
}
