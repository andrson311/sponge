#include "gl_geometry_technique.h"

bool GeometryTechnique::Init()
{
    if (!InitShaders({{GL_VERTEX_SHADER, "framework/shaders/GL/geometry.vs"},
                      {GL_FRAGMENT_SHADER, "framework/shaders/GL/geometry.fs"}}))
    {
        return false;
    }

    return InitUniforms({{&m_WVP, "gWVP"},
                         {&m_VP, "gVP"},
                         {&m_normalMatrix, "gNormalMatrix"},
                         {&m_isIndirectRender, "gIsIndirectRender"},
                         {&m_isPVP, "gIsPVP"},
                         {&m_albedoSampler, "gAlbedoSampler"}});
}
