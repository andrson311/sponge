#pragma once

#include <string>
#include <vector>
#include <GL/glew.h>

#include "bitmap.h"
#include "texture.h"

class BaseCubemapTexture : public BaseTexture
{
public:
    virtual void Load() = 0;
    virtual void Bind(GLenum TextureUnit) = 0;
};

class CubemapTexture : public BaseCubemapTexture
{
public:
    CubemapTexture(const std::string &Directory,
                   const std::string &PosXFilename,
                   const std::string &NegXFilename,
                   const std::string &PosYFilename,
                   const std::string &NegYFilename,
                   const std::string &PosZFilename,
                   const std::string &NegZFilename);
    CubemapTexture() {};
    ~CubemapTexture();

    virtual void Load();
    void LoadKTX(const std::string &Filename);
    virtual void Bind(GLenum TextureUnit);

private:
    std::string m_fileNames[6];
};

class CubemapEctTexture : public BaseCubemapTexture
{
public:
    CubemapEctTexture(const std::string &Filename);
    ~CubemapEctTexture() {};

    virtual void Load();
    virtual void Bind(GLenum TextureUnit);

private:
    void LoadCubemapData(const std::vector<Bitmap> &Cubemap);
    std::string m_filename;
};
