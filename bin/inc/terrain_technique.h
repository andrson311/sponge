#pragma once

#include "util.h"
#include "technique.h"

class TerrainTechnique : public Technique
{
public:
    TerrainTechnique() {};

    virtual bool Init();
    void SetVP(const glm::mat4 &VP);
    void SetMinMaxHeight(float Min, float Max);
    void SetTextureHeights(float Tex0Height, float Tex1Height, float Tex2Height, float Tex3Height);
    void SetLightDir(const glm::vec3& Dir);
    void SetTime(float Time);

private:
    GLuint m_VPLoc = -1;
    GLuint m_texUnitLoc = -1;
    GLuint m_reversedLightDirLoc = -1;
    GLuint m_timeLoc = -1;
};
