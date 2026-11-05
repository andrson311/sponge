#include <cstdio>
#include <cstdlib>

#include "framebuffer.h"

Framebuffer::~Framebuffer()
{
    if (m_fbo)
        glDeleteFramebuffers(1, &m_fbo);
    if (m_colorBuffer)
        glDeleteTextures(1, &m_colorBuffer);
    if (m_normalBuffer)
        glDeleteTextures(1, &m_normalBuffer);
    if (m_depthBuffer)
        glDeleteTextures(1, &m_depthBuffer);
}

void Framebuffer::Init(int Width, int Height, int NumFormatComponents,
                       bool IsFloat, bool DepthEnabled, bool NormalEnabled)
{
    if (Width <= 0 || Height <= 0 || NumFormatComponents < 0 || NumFormatComponents > 4)
    {
        fprintf(stderr, "Invalid framebuffer dimensions or color format\n");
        exit(1);
    }

    m_width = Width;
    m_height = Height;
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    if (NumFormatComponents)
        GenerateBuffer(m_colorBuffer, Width, Height, NumFormatComponents, IsFloat);
    if (NormalEnabled)
        GenerateBuffer(m_normalBuffer, Width, Height, 3, IsFloat);
    if (DepthEnabled)
        GenerateDepthBuffer(Width, Height);

    GLenum attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    if (NumFormatComponents && NormalEnabled)
        glDrawBuffers(2, attachments);
    else if (NumFormatComponents)
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
    else
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        fprintf(stderr, "Framebuffer is incomplete: 0x%x\n", status);
        exit(1);
    }
}

void Framebuffer::GenerateBuffer(GLuint &Buffer, int Width, int Height,
                                 int NumFormatComponents, bool IsFloat)
{
    glGenTextures(1, &Buffer);
    glBindTexture(GL_TEXTURE_2D, Buffer);

    GLenum internalFormat = GL_R8;
    GLenum format = GL_RED;
    if (NumFormatComponents == 2)
    {
        internalFormat = IsFloat ? GL_RG16F : GL_RG8;
        format = GL_RG;
    }
    else if (NumFormatComponents == 3)
    {
        internalFormat = IsFloat ? GL_RGB16F : GL_RGB8;
        format = GL_RGB;
    }
    else if (NumFormatComponents == 4)
    {
        internalFormat = IsFloat ? GL_RGBA16F : GL_RGBA8;
        format = GL_RGBA;
    }
    else if (IsFloat)
    {
        internalFormat = GL_R32F;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, Width, Height, 0,
                 format, IsFloat ? GL_FLOAT : GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    const GLenum attachment = Buffer == m_colorBuffer ? GL_COLOR_ATTACHMENT0 : GL_COLOR_ATTACHMENT1;
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, Buffer, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Framebuffer::GenerateDepthBuffer(int Width, int Height)
{
    glGenTextures(1, &m_depthBuffer);
    glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, Width, Height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBuffer, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Framebuffer::BindForWriting()
{
    m_saveViewport.Save();
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
}

void Framebuffer::UnbindWriting()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_saveViewport.Restore();
}

void Framebuffer::BindColorForReading(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_2D, m_colorBuffer);
}

void Framebuffer::BindNormalForReading(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_2D, m_normalBuffer);
}

void Framebuffer::BindDepthForReading(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
}

void Framebuffer::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Framebuffer::ClearColorBuffer(const glm::vec4 &Color)
{
    glClearBufferfv(GL_COLOR, 0, &Color[0]);
}

void Framebuffer::BlitToWindow()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
