#pragma once

#include "technique.h"
#include "math_3d.h"
#include "util.h"

class QuadTessTechnique : public Technique
{
public:
    QuadTessTechnique() {}

    virtual bool Init();
    void SetWVP(const glm::mat4 &WVP);
    void SetLevels(
        float OuterLevelLeft,
        float OuterLevelBottom,
        float OuterLevelRight,
        float OuterLevelTop,
        float InnerLevelLeftRight,
        float InnerLevelTopBottom);
    void SetColor(const glm::vec4 &Color);

private:
    GLuint m_wvpLoc = INVALID_UNIFORM_LOCATION;

    GLuint m_outerLevelLeftLoc = INVALID_UNIFORM_LOCATION;
    GLuint m_outerLevelBottomLoc = INVALID_UNIFORM_LOCATION;
    GLuint m_outerLevelRightLoc = INVALID_UNIFORM_LOCATION;
    GLuint m_outerLevelTopLoc = INVALID_UNIFORM_LOCATION;
    
    GLuint m_innerLevelLeftRightLoc = INVALID_UNIFORM_LOCATION;
    GLuint m_innerLevelTopBottomLoc = INVALID_UNIFORM_LOCATION;
    
    GLuint m_colorLoc = INVALID_UNIFORM_LOCATION;
};
