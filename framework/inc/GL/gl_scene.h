#pragma once

#include "cubemap_texture.h"
#include "core_scene.h"

class RenderingSystemGL;

class GLScene : public CoreScene
{
public:
    GLScene(RenderingSystemGL *pRenderingSystem) : CoreScene((CoreRenderingSystem *)pRenderingSystem) {}
    ~GLScene();

    void LoadSkybox(const char *pFilename);
    BaseCubemapTexture *GetSkyboxTex() const { return m_pSkyboxTex; }
    virtual int GetSkyboxMipmapLevels() const;

private:
    BaseCubemapTexture* m_pSkyboxTex = NULL;
};
