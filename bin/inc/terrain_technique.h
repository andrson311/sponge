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

private:
    GLuint m_VPLoc = -1;
    GLuint m_minHeightLoc = -1;
    GLuint m_maxHeightLoc = -1;
};
