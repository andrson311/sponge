#pragma once

#include "technique.h"
#include "math_3d.h"
#include "util.h"

class BezierCurveTechnique : public Technique
{
public:
    BezierCurveTechnique() {};

    virtual bool Init();
    void SetWVP(const glm::mat4 &WVP);
    void SetNumSegments(int NumSegments);
    void SetLineColor(float r, float g, float b, float a);

private:
    GLuint m_wvpLoc = INVALID_UNIFORM_LOCATION;
    GLuint m_numSegmentsLoc = INVALID_UNIFORM_LOCATION;
    GLuint m_lineColorLoc = INVALID_UNIFORM_LOCATION;
};
