#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include "util.h"
#include "framebuffer.h"

Framebuffer::~Framebuffer()
{
    if (m_fbo != 0)
    {
        glDeleteFramebuffers(1, &m_fbo);
    }

    if (m_colorBuffer != 0)
    {
        glDeleteTextures(1, &m_colorBuffer);
    }

    if (m_depthBuffer != 0)
    {
        glDeleteTextures(1, &m_depthBuffer);
    }
}

void Framebuffer::Init(int Width, int Height, int NumFormatComponents,
                       bool IsFloat, bool DepthEnabled, bool NormalEnabled)
{
    InitNonDSA(Width, Height, NumFormatComponents, IsFloat, DepthEnabled, NormalEnabled);
}

void Framebuffer::InitNonDSA(int Width, int Height, int NumFormatComponents,
                             bool IsFloat, bool DepthEnabled, bool NormalEnabled)
{
    m_width = Width;
    m_height = Height;

    std::vector<GLenum> DrawBuffers;
    DrawBuffers.push_back(GL_COLOR_ATTACHMENT0);

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    GenerateBuffer(m_colorBuffer, Width, Height, NumFormatComponents, IsFloat);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorBuffer, 0);

    if (DepthEnabled)
    {
        GenerateDepthBuffer(Width, Height);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBuffer, 0);
    }

    if (NormalEnabled)
    {
        GenerateBuffer(m_normalBuffer, Width, Height, NumFormatComponents, IsFloat);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_normalBuffer, 0);
        DrawBuffers.push_back(GL_COLOR_ATTACHMENT1);
    }

    glDrawBuffers((GLsizei)DrawBuffers.size(), DrawBuffers.data());
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("FB error, status: 0x%x\n", Status);
        exit(0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::GenerateDepthBuffer(int Width, int Height)
{
    glGenTextures(1, &m_depthBuffer);
    glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, Width, Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Framebuffer::GenerateBuffer(GLuint &Buffer, int Width, int Height, int NumFormatComponents, bool IsFloat)
{
    GenerateBufferNonDSA(Buffer, Width, Height, NumFormatComponents, IsFloat);
}

void Framebuffer::GenerateBufferNonDSA(GLuint &Buffer, int Width, int Height, int NumFormatComponents, bool IsFloat)
{
    glGenTextures(1, &Buffer);
    glBindTexture(GL_TEXTURE_2D, Buffer);

    switch (NumFormatComponents)
    {
    case 4:
        assert(!IsFloat);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        break;
    case 3:
        assert(!IsFloat);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        break;
    case 1:
        assert(IsFloat);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, Width, Height, 0, GL_RED, GL_FLOAT, NULL);
        break;

    default:
        printf("Unsupported num components %d\n", NumFormatComponents);
        exit(1);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Framebuffer::BindForWriting()
{
    m_saveViewport.Save();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
}

void Framebuffer::UnbindWriting()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
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
    if (m_depthBuffer == -1)
    {
        printf("Trying to bind depth for reading in a FBO without depth buffer\n");
        exit(1);
    }

    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
}

void Framebuffer::Clear()
{
    glm::vec4 Color(0.0f);
    float Depth = 1.0f;

    if (m_colorBuffer != -1)
    {
        glClearBufferfv(GL_COLOR, 0, glm::value_ptr(Color));
    }

    if (m_depthBuffer != -1)
    {
        glClearBufferfv(GL_DEPTH, 0, &Depth);
    }

    if (m_normalBuffer != -1)
    {
        glClearBufferfv(GL_COLOR, 1, glm::value_ptr(Color));
    }
}

void Framebuffer::ClearColorBuffer(const glm::vec4 &Color)
{
    glClearBufferfv(GL_COLOR, 0, glm::value_ptr(Color));
}

void Framebuffer::BlitToWindow()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, m_width, m_height,
                      0, 0, m_width, m_height,
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
