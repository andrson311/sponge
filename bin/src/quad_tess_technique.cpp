#include "quad_tess_technique.h"

bool QuadTessTechnique::Init()
{
    if (!Technique::Init())
    {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "bin/shaders/passthru_vec2.vs"))
    {
        return false;
    }

    if (!AddShader(GL_TESS_CONTROL_SHADER, "bin/shaders/quad_tess.tcs"))
    {
        return false;
    }

    if (!AddShader(GL_TESS_EVALUATION_SHADER, "bin/shaders/quad_tess.tes"))
    {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "bin/shaders/color.fs"))
    {
        return false;
    }

    if (!Finalize())
    {
        return false;
    }

    m_wvpLoc = GetUniformLocation("gWVP");

    m_outerLevelLeftLoc = GetUniformLocation("gOuterLevelLeft");
    m_outerLevelBottomLoc = GetUniformLocation("gOuterLevelBottom");
    m_outerLevelRightLoc = GetUniformLocation("gOuterLevelRight");
    m_outerLevelTopLoc = GetUniformLocation("gOuterLevelTop");
    
    m_innerLevelLeftRightLoc = GetUniformLocation("gInnerLevelLeftRight");
    m_innerLevelTopBottomLoc = GetUniformLocation("gInnerLevelTopBottom");

    m_colorLoc = GetUniformLocation("gColor");

    return ((m_wvpLoc != INVALID_UNIFORM_LOCATION) &&
            (m_colorLoc != INVALID_UNIFORM_LOCATION) &&
            (m_outerLevelLeftLoc != INVALID_UNIFORM_LOCATION) &&
            (m_outerLevelRightLoc != INVALID_UNIFORM_LOCATION) &&
            (m_outerLevelBottomLoc != INVALID_UNIFORM_LOCATION) &&
            (m_outerLevelTopLoc != INVALID_UNIFORM_LOCATION) &&
            (m_innerLevelLeftRightLoc != INVALID_UNIFORM_LOCATION) &&
            (m_innerLevelTopBottomLoc != INVALID_UNIFORM_LOCATION));
}

void QuadTessTechnique::SetWVP(const glm::mat4 &WVP)
{
    glUniformMatrix4fv(m_wvpLoc, 1, GL_FALSE, glm::value_ptr(WVP));
}

void QuadTessTechnique::SetLevels(
    float OuterLevelLeft,
    float OuterLevelBottom,
    float OuterLevelRight,
    float OuterLevelTop,
    float InnerLevelLeftRight,
    float InnerLevelTopBottom)
{
    glUniform1f(m_outerLevelLeftLoc, OuterLevelLeft);
    glUniform1f(m_outerLevelBottomLoc, OuterLevelBottom);
    glUniform1f(m_outerLevelRightLoc, OuterLevelRight);
    glUniform1f(m_outerLevelTopLoc, OuterLevelTop);

    glUniform1f(m_innerLevelLeftRightLoc, InnerLevelLeftRight);
    glUniform1f(m_innerLevelTopBottomLoc, InnerLevelTopBottom);
}

void QuadTessTechnique::SetColor(const glm::vec4 &Color)
{
    glUniform4f(m_colorLoc, Color.x, Color.y, Color.z, Color.w);
}
