#include <stdio.h>
#include "texture.h"
#include "util.h"
#include "stb_image.h"
#include "stb_image_write.h"

Texture::Texture(GLenum TextureTarget, const std::string &FileName)
{
    m_textureTarget = TextureTarget;
    m_fileName = FileName;
}

Texture::Texture(GLenum TextureTarget)
{
    m_textureTarget = TextureTarget;
}

bool Texture::Load(bool IsSRGB)
{
    stbi_set_flip_vertically_on_load(1);

    u_char *pImageData = stbi_load(m_fileName.c_str(), &m_imageWidth, &m_imageHeight, &m_imageBPP, 0);

    if (!pImageData)
    {
        fprintf(stderr, "Can't load texture from '%s' - %s\n", m_fileName.c_str(), stbi_failure_reason());
        return false;
    }

    printf("Loaded texture '%s' width %d, height %d, bpp %d\n",
           m_fileName.c_str(), m_imageWidth, m_imageHeight, m_imageBPP);

    LoadInternal(pImageData, IsSRGB);

    stbi_image_free(pImageData);
    return true;
}

void Texture::Load(const std::string &Filename, bool IsSRGB)
{
    m_fileName = Filename;
    if (!Load(IsSRGB))
    {
        exit(1);
    }
}

void Texture::Load(u_int BufferSize, const void *pData, bool IsRGB)
{
    int width, height, channels;
    u_char *pImageData = stbi_load_from_memory(
        (const stbi_uc *)pData, BufferSize,
        &width, &height, &channels, 0);

    if (!pImageData)
    {
        fprintf(stderr, "Failed to load embedded texture: %s\n", stbi_failure_reason());
        return;
    }

    m_imageWidth = width;
    m_imageHeight = height;
    m_imageBPP = channels;

    LoadInternal(pImageData, IsRGB);
    stbi_image_free(pImageData);
}

void Texture::LoadRaw(int Width, int Height, int BPP, const u_char *pImageData, bool IsSRGB)
{
    m_imageWidth = Width;
    m_imageHeight = Height;
    m_imageBPP = BPP;
    LoadInternal(pImageData, IsSRGB);
}

void Texture::LoadF32(int Width, int Height, const float *pImageData)
{
    if (!IsGLVersionHigher(4, 5))
    {
        printf("Non DSA version is not implemented");
        exit(0);
    }

    m_imageWidth = Width;
    m_imageHeight = Height;

    glCreateTextures(m_textureTarget, 1, &m_textureObj);
    glTextureStorage2D(m_textureObj, 1, GL_R32F, m_imageWidth, m_imageHeight);
    glTextureSubImage2D(m_textureObj, 0, 0, 0, m_imageWidth, m_imageHeight, GL_RED, GL_FLOAT, pImageData);

    glTextureParameteri(m_textureObj, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_textureObj, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameterf(m_textureObj, GL_TEXTURE_BASE_LEVEL, 0);
    glTextureParameteri(m_textureObj, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_textureObj, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Texture::LoadInternal(const void *pImageData, bool IsSRGB)
{
    if (IsGLVersionHigher(4, 5))
    {
        LoadInternalDSA(pImageData, IsSRGB);
    }
    else
    {
        LoadInternalNonDSA(pImageData, IsSRGB);
    }
}

void Texture::LoadInternalNonDSA(const void *pImageData, bool IsSRGB)
{
    glGenTextures(1, &m_textureObj);
    glBindTexture(m_textureTarget, m_textureObj);

    if (m_textureTarget == GL_TEXTURE_2D)
    {
        switch (m_imageBPP)
        {
        case 1:
        {
            glTexImage2D(m_textureTarget, 0, GL_RED, m_imageWidth, m_imageHeight, 0, GL_RED, GL_UNSIGNED_BYTE, pImageData);
            GLint SwizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_RED};
            glTexParameteriv(m_textureTarget, GL_TEXTURE_SWIZZLE_RGBA, SwizzleMask);
            break;
        }
        case 2:
            glTexImage2D(m_textureTarget, 0, GL_RG, m_imageWidth, m_imageHeight, 0, GL_RG, GL_UNSIGNED_BYTE, pImageData);
            break;
        case 3:
        {
            GLenum InternalFormat = IsSRGB ? GL_SRGB8 : GL_RGB8;
            glTexImage2D(m_textureTarget, 0, InternalFormat, m_imageWidth, m_imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, pImageData);
            break;
        }
        case 4:
        {
            GLenum InternalFormat = IsSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
            glTexImage2D(m_textureTarget, 0, InternalFormat, m_imageWidth, m_imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImageData);
            break;
        }
        default:
            fprintf(stderr, "Unsupported BPP: %d\n", m_imageBPP);
            exit(1);
        }
    }
    else
    {
        fprintf(stderr, "Support for texture target %x is not implemented\n", m_textureTarget);
        exit(1);
    }

    glTexParameteri(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(m_textureTarget, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glGenerateMipmap(m_textureTarget);

    glBindTexture(m_textureTarget, 0);
}

void Texture::LoadInternalDSA(const void *pImageData, bool IsSRGB)
{
    glCreateTextures(m_textureTarget, 1, &m_textureObj);

    int levels = std::max(
        1,
        std::min(
            5,
            (int)log2f((float)std::max(m_imageWidth, m_imageHeight))));

    GLenum InternalFormat = GL_NONE;

    if (m_textureTarget == GL_TEXTURE_2D)
    {
        if (m_isKTX)
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTextureStorage2D(m_textureObj, levels, m_ktxFormat.Internal, m_imageWidth, m_imageHeight);
            glTextureSubImage2D(m_textureObj, 0, 0, 0,
                                m_imageWidth, m_imageHeight,
                                m_ktxFormat.External, m_ktxFormat.Type, pImageData);
        }
        else
        {
            switch (m_imageBPP)
            {
            case 1:
            {
                glTextureStorage2D(m_textureObj, levels, GL_R8, m_imageWidth, m_imageHeight);
                glTextureSubImage2D(m_textureObj, 0, 0, 0, m_imageWidth, m_imageHeight, GL_RED, GL_UNSIGNED_BYTE, pImageData);
                GLint SwizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_RED};
                glTextureParameteriv(m_textureObj, GL_TEXTURE_SWIZZLE_RGBA, SwizzleMask);
            }
            break;

            case 2:
                glTextureStorage2D(m_textureObj, levels, GL_RG8, m_imageWidth, m_imageHeight);
                glTextureSubImage2D(m_textureObj, 0, 0, 0, m_imageWidth, m_imageHeight, GL_RG, GL_UNSIGNED_BYTE, pImageData);
                break;

            case 3:
                InternalFormat = IsSRGB ? GL_SRGB8 : GL_RGB8;
                glTextureStorage2D(m_textureObj, levels, InternalFormat, m_imageWidth, m_imageHeight);
                glTextureSubImage2D(m_textureObj, 0, 0, 0, m_imageWidth, m_imageHeight, GL_RGB, GL_UNSIGNED_BYTE, pImageData);
                break;

            case 4:
                InternalFormat = IsSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
                glTextureStorage2D(m_textureObj, levels, InternalFormat, m_imageWidth, m_imageHeight);
                glTextureSubImage2D(m_textureObj, 0, 0, 0, m_imageWidth, m_imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, pImageData);
                break;

            default:
                break;
            }
        }
    }

    glTextureParameteri(m_textureObj, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(m_textureObj, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_textureObj, GL_TEXTURE_BASE_LEVEL, 0);
    glTextureParameteri(m_textureObj, GL_TEXTURE_MAX_LEVEL, levels - 1);
    glTextureParameteri(m_textureObj, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_textureObj, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(m_textureObj, GL_TEXTURE_MAX_ANISOTROPY, 16);

    glGenerateTextureMipmap(m_textureObj);

    m_bindlessHandle = glGetTextureHandleARB(m_textureObj);
    glMakeTextureHandleResidentARB(m_bindlessHandle);
}

void Texture::Bind(GLenum TextureUnit)
{
    if (IsGLVersionHigher(4, 5))
    {
        BindInternalDSA(TextureUnit);
    }
    else
    {
        BindInternalNonDSA(TextureUnit);
    }
}

void Texture::BindInternalNonDSA(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(m_textureTarget, m_textureObj);
}

void Texture::BindInternalDSA(GLenum TextureUnit)
{
    glBindTextureUnit(TextureUnit - GL_TEXTURE0, m_textureObj);
}
