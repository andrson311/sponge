#include "bezier_curve_technique.h"

bool BezierCurveTechnique::Init()
{
    if (!Technique::Init())
    {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "bin/shaders/passthru_vec2.vs"))
    {
        return false;
    }

    if (!AddShader(GL_TESS_EVALUATION_SHADER, "bin/shaders/bezier_curve.tes"))
    {
        return false;
    }

    if (!AddShader(GL_TESS_CONTROL_SHADER, "bin/shaders/bezier_curve.tcs"))
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
    m_numSegmentsLoc = GetUniformLocation("gNumSegments");
    m_lineColorLoc = GetUniformLocation("gColor");

    return ((m_wvpLoc != INVALID_UNIFORM_LOCATION) &&
            (m_numSegmentsLoc != INVALID_UNIFORM_LOCATION) &&
            (m_lineColorLoc != INVALID_UNIFORM_LOCATION));
}

void BezierCurveTechnique::SetWVP(const glm::mat4 &WVP)
{
    glUniformMatrix4fv(m_wvpLoc, 1, GL_FALSE, glm::value_ptr(WVP));
}

void BezierCurveTechnique::SetNumSegments(int NumSegments)
{
    glUniform1i(m_numSegmentsLoc, NumSegments);
}

void BezierCurveTechnique::SetLineColor(float r, float g, float b, float a)
{
    glUniform4f(m_lineColorLoc, r, g, b, a);
}
