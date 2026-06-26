#pragma once

#include "technique.h"
#include "util.h"

class SingleTexTerrainTechnique : public Technique
{
public:
    SingleTexTerrainTechnique() {};

    virtual bool Init();
    void SetVP(const glm::mat4& VP);
    void SetMinMaxHeight(float Min, float Max);

private:
    GLuint m_VPLoc = -1;
    GLuint m_texUnitLoc = -1;
    GLuint m_minHeightLoc = -1;
    GLuint m_maxHeightLoc = -1;
};