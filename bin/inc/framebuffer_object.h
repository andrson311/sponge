#pragma once

#include <GL/glew.h>
#include "util.h"

class FramebufferObject
{
public:
    FramebufferObject();
    ~FramebufferObject();

    bool Init(u_int Width, u_int Height, bool ForPCF = false);
    void BindForWriting();
    void BindForReading(GLenum TextureUnit);

private:
    bool InitNonDSA(u_int Width, u_int Height, bool ForPCF = false);
    // bool InitDSA(u_int Width, u_int Height, bool ForPCF = false);

    // void BindForReadingDSA(GLenum TextureUnit);
    void BindForReadingNonDSA(GLenum TextureUnit);

    u_int m_width = 0;
    u_int m_height = 0;
    GLuint m_fbo;
    GLuint m_depthBuffer;
};

class CascadedShadowMapFBO
{
public:
    CascadedShadowMapFBO();
    ~CascadedShadowMapFBO();

    bool Init(u_int WindowWidth, u_int WindowHeight);
    void BindForWriting(u_int CascadeIndex);
    void BindForReading();

private:
    GLuint m_fbo;
    GLuint m_shadowMap[3];
};