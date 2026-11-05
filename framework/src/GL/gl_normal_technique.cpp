#include "gl_normal_technique.h"

bool NormalTechnique::Init()
{
    if (!InitShaders({{GL_VERTEX_SHADER, "framework/shaders/GL/normal.vs"},
                      {GL_FRAGMENT_SHADER, "framework/shaders/GL/normal.fs"}}))
    {
        return false;
    }

    return InitUniforms({{&m_WVP, "gWVP"},
                         {&m_VP, "gVP"},
                         {&m_normalMatrix, "gNormalMatrix"},
                         {&m_isIndirectRender, "gIsIndirectRender"},
                         {&m_isPVP, "gIsPVP"}});
}
