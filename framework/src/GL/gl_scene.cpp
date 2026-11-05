#include "cubemap_texture.h"
#include "gl_scene.h"

GLScene::~GLScene()
{
    delete m_pSkyboxTex;
}

void GLScene::LoadSkybox(const char *pFilename)
{
    if (m_pSkyboxTex)
    {
        delete m_pSkyboxTex;
    }

    m_pSkyboxTex = new CubemapEctTexture(pFilename);
    m_pSkyboxTex->Load();
}

int GLScene::GetSkyboxMipmapLevels() const
{
    int MipmapLevels = 1;

    if (m_pSkyboxTex)
    {
        MipmapLevels = m_pSkyboxTex->GetMipmapLevels();
    }

    return MipmapLevels;
}