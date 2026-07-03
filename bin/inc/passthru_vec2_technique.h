#pragma once

#include "technique.h"
#include "math_3d.h"
#include "util.h"

class PassthruVec2Technique : public Technique
{
public:
    PassthruVec2Technique() {};

    virtual bool Init();
    void SetColor(float r, float g, float b);

private:
    GLuint m_colorLoc = -1;
};
