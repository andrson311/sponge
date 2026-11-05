#include "gl_skybox.h"

void Skybox::Init(GLenum textureUnit, int textureUnitIndex)
{
    m_textureUnit = textureUnit;

    if (!m_skyboxTech.Init())
    {
        printf("Error initializing skybox technique\n");
        exit(1);
    }

    m_skyboxTech.Enable();
    m_skyboxTech.SetTextureUnitIndex(textureUnitIndex);

    glGenVertexArrays(1, &m_dummyVAO);
}

void Skybox::Render(BaseCubemapTexture *pSkyboxTex, const glm::mat4 &VP)
{
    if (!pSkyboxTex)
    {
        printf("Skybox texture was not loaded\n");
        exit(1);
    }

    m_skyboxTech.Enable();

    GLint oldCullFaceMode;
    glGetIntegerv(GL_CULL_FACE_MODE, &oldCullFaceMode);

    GLint oldDepthFuncMode;
    glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFuncMode);

    glCullFace(GL_FRONT);
    glDepthFunc(GL_LEQUAL);

    m_skyboxTech.SetVP(VP);

    pSkyboxTex->Bind(m_textureUnit);

    glBindVertexArray(m_dummyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glCullFace(oldCullFaceMode);
    glDepthFunc(oldDepthFuncMode);
}
