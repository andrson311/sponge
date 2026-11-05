#include "gl_infinite_grid.h"

void InfiniteGrid::Init(int shadowMapTextureUnit)
{
    if (!m_infiniteGridTech.Init())
    {
        printf("Error initializing the infinite grid technique\n");
        exit(1);
    }

    m_infiniteGridTech.Enable();
    m_infiniteGridTech.SetShadowMapTextureUnit(shadowMapTextureUnit);

    glGenVertexArrays(1, &m_dummyVAO);
}

void InfiniteGrid::Render(const InfiniteGridConfig &config, const glm::mat4 &VP,
                          const glm::vec3 &cameraPos, const glm::mat4 &lightVP,
                          const glm::vec3 &lightDir)
{
    GLint curProg = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &curProg);

    m_infiniteGridTech.Enable();
    m_infiniteGridTech.SetVP(VP);
    m_infiniteGridTech.SetLightVP(lightVP);
    m_infiniteGridTech.SetCameraWorldPos(cameraPos);
    m_infiniteGridTech.SetCellSize(config.CellSize);
    m_infiniteGridTech.SetLightDirection(lightDir);
    m_infiniteGridTech.ControlShadows(config.ShadowsEnabled);

    glBindVertexArray(m_dummyVAO);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, 1, 0);
    glDisable(GL_BLEND);

    glBindVertexArray(0);
    glUseProgram(curProg);
}
