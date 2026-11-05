#include "gl_shadow_mapping_technique_point_light.h"

bool ShadowMappingPointLightTechnique::Init()
{
    if (!InitShaders({{GL_VERTEX_SHADER, "framework/shaders/GL/point_shadow_map.vs"},
                      {GL_FRAGMENT_SHADER, "framework/shaders/GL/point_shadow_map.fs"}}))
        return false;

    return InitUniforms({{&m_WVP, "gWVP"},
                         {&m_worldMatrix, "gWorld"},
                         {&m_lightWorldPos, "gLightWorldPos"}});
}
