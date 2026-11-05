#pragma once

#include "object.h"
#include "core_material.h"

class Model : public Object
{
public:
    virtual void SetColorTexture(int textureHandle) {}
    virtual void SetNormalMap(int textureHandle) {}
    virtual void SetHeightMap(int textureHandle) {}
    virtual void SetAmbientOcclusionMap(int textureHandle) {}
    virtual void SetTextureScale(float scale) {}
    virtual void ConvertToMesh(const char *pFilename) { assert(0); }
    // virtual int GetMeshIndex(const std::string &name) const { return -1; }
    virtual size_t GetNumMeshes() const { return 0; }
};

class Grid : public Model
{
};
