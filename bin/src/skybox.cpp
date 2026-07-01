#include "skybox.h"
#include "pipeline.h"
#include "util.h"
#include "basic_mesh.h"

SkyBox::SkyBox()
{
    m_pSkyboxTechnique = NULL;
    m_pCubemapTex = NULL;
    m_pMesh = NULL;
}

SkyBox::~SkyBox()
{
    if (m_pSkyboxTechnique)
    {
        delete m_pSkyboxTechnique;
        m_pSkyboxTechnique = NULL;
    }

    if (m_pCubemapTex)
    {
        delete m_pCubemapTex;
        m_pCubemapTex = NULL;
    }

    if (m_pMesh)
    {
        delete m_pMesh;
        m_pMesh = NULL;
    }
}

void SkyBox::Init(const std::string &Directory,
                  const std::string &PosXFilename,
                  const std::string &NegXFilename,
                  const std::string &PosYFilename,
                  const std::string &NegYFilename,
                  const std::string &PosZFilename,
                  const std::string &NegZFilename)
{
    InitTechnique();

    m_pCubemapTex = new CubemapTexture(Directory,
                                       PosXFilename,
                                       NegXFilename,
                                       PosYFilename,
                                       NegYFilename,
                                       PosZFilename,
                                       NegZFilename);

    LoadTextureAndMesh();
}

void SkyBox::Init(const std::string &EctTextureFilename)
{
    InitTechnique();

    m_pCubemapTex = new CubemapEctTexture(EctTextureFilename);

    LoadTextureAndMesh();
}

void SkyBox::InitTechnique()
{
    m_pSkyboxTechnique = new SkyboxTechnique();

    if (!m_pSkyboxTechnique->Init())
    {
        printf("Error initializing the skybox technique\n");
        exit(1);
    }

    m_pSkyboxTechnique->Enable();
    m_pSkyboxTechnique->SetTextureUnit(0);
}

void SkyBox::LoadTextureAndMesh()
{
    m_pCubemapTex->Load();

    m_pMesh = new BasicMesh();

    m_pMesh->LoadMesh("assets/skybox/box.obj");
}

void SkyBox::Render(const Camera &Camera)
{
    m_pSkyboxTechnique->Enable();

    GLint OldCullFaceMode;
    glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);

    GLint OldDepthFuncMode;
    glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);

    glCullFace(GL_FRONT);
    glDepthFunc(GL_LEQUAL);

    static float r = 0.0f;
    glm::mat4 Rotation = glm::eulerAngleXYZ(0.0f, glm::radians(r), 0.0f);

    r += 0.01f;

    glm::mat4 View = glm::lookAt(glm::vec3(0.0f), Camera.GetTarget(), Camera.GetUp());
    glm::mat4 Proj = glm::perspectiveFov(glm::radians(Camera.GetPersProjInfo().FOV),
                                         Camera.GetPersProjInfo().Width, Camera.GetPersProjInfo().Height,
                                         Camera.GetPersProjInfo().zNear, Camera.GetPersProjInfo().zFar);
    glm::mat4 WVP = Proj * View * Rotation;
    m_pSkyboxTechnique->SetWVP(WVP);
    m_pCubemapTex->Bind(GL_TEXTURE0);
    m_pMesh->Render();

    glCullFace(OldCullFaceMode);
    glDepthFunc(OldDepthFuncMode);
}

void SkyBox::Render(const glm::mat4 &VP)
{
    m_pSkyboxTechnique->Enable();

    GLint OldCullFaceMode;
    glGetIntegerv(GL_CULL_FACE_MODE, &OldCullFaceMode);

    GLint OldDepthFuncMode;
    glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);

    glCullFace(GL_FRONT);

    glDepthFunc(GL_LEQUAL);

    static float r = 0.0f;
    glm::mat4 Rotation = glm::eulerAngleXYZ(0.0f, glm::radians(r), 0.0f);
    r += 0.01f;

    glm::mat4 WVP = VP * Rotation;
    m_pSkyboxTechnique->SetWVP(WVP);
    m_pCubemapTex->Bind(GL_TEXTURE0);
    m_pMesh->Render();

    glCullFace(OldCullFaceMode);

    glDepthFunc(OldDepthFuncMode);
}
