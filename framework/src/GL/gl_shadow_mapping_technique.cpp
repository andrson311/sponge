#include "gl_shadow_mapping_technique.h"

bool ShadowMappingTechnique::Init()
{
    if (!InitShaders({{GL_VERTEX_SHADER, "framework/shaders/GL/shadow_map.vs"},
                      {GL_FRAGMENT_SHADER, "framework/shaders/GL/shadow_map.fs"}}))
    {
        return false;
    }

    return InitUniforms({{&m_WVP, "gWVP"},
                         {&m_VP, "gVP"},
                         {&m_isIndirectRender, "gIsIndirectRender"},
                         {&m_isPVP, "gIsPVP"}});
}
