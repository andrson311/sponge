#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "save_viewport.h"

class Framebuffer
{
public:
    Framebuffer() {};
    ~Framebuffer();

    void Init(int Width, int Height, int NumFormatComponents,
              bool IsFloat, bool DepthEnabled, bool NormalEnabled);

    void BindForWriting();
    void UnbindWriting();

    void BindColorForReading(GLenum TextureUnit);
    void BindNormalForReading(GLenum TextureUnit);
    void BindDepthForReading(GLenum TextureUnit);

    void Clear();
    void ClearColorBuffer(const glm::vec4 &Color);

    void BlitToWindow();

    GLuint GetTexture() const { return m_colorBuffer; }
    GLuint GetDepthTexture() const { return m_depthBuffer; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    // void InitDSA(int Width, int Height, int NumFormatComponents,
    //              bool IsFloat, bool DepthEnabled, bool NormalEnabled);
    void InitNonDSA(int Width, int Height, int NumFormatComponents,
                    bool IsFloat, bool DepthEnabled, bool NormalEnabled);

    void GenerateBuffer(GLuint &Buffer, int Width, int Height,
                        int NumFormatComponents, bool IsFloat);
    // void GenerateBufferDSA(GLuint &Buffer, int Width, int Height,
    //                     int NumFormatComponents, bool IsFloat);
    void GenerateBufferNonDSA(GLuint &Buffer, int Width, int Height,
                        int NumFormatComponents, bool IsFloat);
    void GenerateDepthBuffer(int Width, int Height);

    int m_width = 0;
    int m_height = 0;
    GLuint m_fbo = -1;
    GLuint m_colorBuffer = -1;
    GLuint m_depthBuffer = -1;
    GLuint m_normalBuffer = -1;
    SaveViewport m_saveViewport;
};
