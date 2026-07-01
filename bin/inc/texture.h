#pragma once
#include <string>
#include <GL/glew.h>
#include <gli/gli.hpp>

class BaseTexture
{
protected:
    BaseTexture() {}

    GLenum m_textureTarget = 0;
    GLuint m_textureObj = 0;

    bool m_isKTX = false;
    gli::gl::format m_ktxFormat;
};

class Texture : public BaseTexture
{
public:
    Texture(GLenum TextureTarget, const std::string &FileName);
    Texture(GLenum TextureTarget);

    bool Load(bool IsSRGB = false);
    void Load(const std::string &Filename, bool IsSRGB = false);
    void Load(u_int BufferSize, const void *pData, bool isSRGB = false);
    void LoadRaw(int Width, int Height, int BPP, const u_char *pImageData, bool IsSRGB = false);
    void LoadF32(int Width, int Height, const float *pImageData);

    void Bind(GLenum TextureUnit);

    void GetImageSize(int &ImageWidth, int &ImageHeight)
    {
        ImageWidth = m_imageWidth;
        ImageHeight = m_imageHeight;
    }

    GLuint GetTexture() const { return m_textureObj; }
    GLuint64 GetBindlessHandle() const { return m_bindlessHandle; }

private:
    void LoadInternal(const void *pImageData, bool IsSRGB);
    void LoadInternalNonDSA(const void * pImageData, bool IsRGB);
    void LoadInternalDSA(const void * pImageData, bool IsRGB);

    void BindInternalNonDSA(GLenum TextureUnit);
    void BindInternalDSA(GLenum TextureUnit);

    std::string m_fileName;
    GLuint64 m_bindlessHandle = -1;
    int m_imageWidth = 0;
    int m_imageHeight = 0;
    int m_imageBPP = 0;
};
