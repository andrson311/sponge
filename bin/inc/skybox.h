#pragma once

#include "camera.h"
#include "skybox_technique.h"
#include "cubemap_texture.h"

class BasicMesh;

class SkyBox
{
public:
    SkyBox();
    ~SkyBox();

    void Init(const std::string &Directory,
              const std::string &PosXFilename,
              const std::string &NegXFilename,
              const std::string &PosYFilename,
              const std::string &NegYFilename,
              const std::string &PosZFilename,
              const std::string &NegZFilename);
    void Init(const std::string &ExtTextureFilename);

    void Render(const Camera& pCamera);
    void Render(const glm::mat4& VP);

private:
    void InitTechnique();
    void LoadTextureAndMesh();

    SkyboxTechnique* m_pSkyboxTechnique = NULL;
    BaseCubemapTexture* m_pCubemapTex = NULL;
    BasicMesh* m_pMesh = NULL;
};
