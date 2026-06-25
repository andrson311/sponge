#pragma once

#include "util.h"
#include "technique.h"

class TerrainTechnique : public Technique
{
public:
    TerrainTechnique() {};

    virtual bool Init();
    void SetVP(const glm::mat4 &VP);

private:
    GLuint m_VPLoc = -1;
};
