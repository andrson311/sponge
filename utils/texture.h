#pragma once
#include <string>
#include <GL/glew.h>

class Texture
{
public:
    Texture(GLenum TextureTarget, const std::string &FileName);
    Texture(GLenum TextureTarget);

    bool Load(bool IsSRGB = false);
    void Load(const std::string &Filename, bool IsSRGB = false);
    void LoadRaw(int Width, int Height, int BPP, const u_char *pImageData, bool IsSRGB = false);

    void Bind(GLenum TextureUnit);

    void GetImageSize(int &ImageWidth, int &ImageHeight)
    {
        ImageWidth = m_imageWidth;
        ImageHeight = m_imageHeight;
    }

    GLuint GetTexture() const { return m_textureObj; }

private:
    void LoadInternal(const void* pImageData, bool IsSRGB);

    GLenum m_textureTarget = 0;
    GLuint m_textureObj = 0;
    std::string m_fileName;
    int m_imageWidth = 0;
    int m_imageHeight = 0;
    int m_imageBPP = 0;
};
