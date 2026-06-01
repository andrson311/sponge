#include "texture.h"
#include <stdio.h>

#include "../includes/stb_image.h"

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
    u_char* pImageData = stbi_load_from_memory(
        (const stbi_uc*)pData, BufferSize,
        &width, &height, &channels, 0
    );

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

void Texture::LoadInternal(const void *pImageData, bool IsSRGB)
{
    glGenTextures(1, &m_textureObj);
    glBindTexture(m_textureTarget, m_textureObj);

    if (m_textureTarget == GL_TEXTURE_2D)
    {
        switch (m_imageBPP)
        {
        case 1:
            glTexImage2D(m_textureTarget, 0, GL_RED, m_imageWidth, m_imageHeight, 0, GL_RED, GL_UNSIGNED_BYTE, pImageData);
            {
                GLint SwizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_RED};
                glTexParameteriv(m_textureTarget, GL_TEXTURE_SWIZZLE_RGBA, SwizzleMask);
            }
            break;
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

void Texture::Bind(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(m_textureTarget, m_textureObj);
}
