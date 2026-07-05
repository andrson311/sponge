#pragma once

#include "util.h"
#include "technique.h"

class TerrainTechnique : public Technique
{
public:
    TerrainTechnique() {};

    virtual bool Init();
    void SetVP(const glm::mat4 &VP);
    void SetViewMatrix(const glm::mat4 &View);
    void SetMinMaxHeight(float Min, float Max);
    void SetTextureHeights(float Tex0Height, float Tex1Height, float Tex2Height, float Tex3Height);
    void SetLightDir(const glm::vec3 &Dir);
    void SetTime(float Time);

private:
    GLuint m_VPLoc = -1;
    GLuint m_ViewLoc = -1;
    GLuint m_tex0HeightLoc = -1;
    GLuint m_tex1HeightLoc = -1;
    GLuint m_tex2HeightLoc = -1;
    GLuint m_tex3HeightLoc = -1;
    GLuint m_tex0UnitLoc = -1;
    GLuint m_tex1UnitLoc = -1;
    GLuint m_tex2UnitLoc = -1;
    GLuint m_tex3UnitLoc = -1;
    GLuint m_reversedLightDirLoc = -1;
    GLuint m_heightMapLoc = -1;
};
