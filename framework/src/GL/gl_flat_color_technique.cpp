#include "gl_flat_color_technique.h"

bool FlatColorTechnique::Init()
{
    if (!InitShaders({{GL_VERTEX_SHADER, "framework/shaders/GL/flat_color.vs"},
                      {GL_FRAGMENT_SHADER, "framework/shaders/GL/flat_color.fs"}}))
    {
        return false;
    }

    return InitUniforms({{&m_WVP, "gWVP"},
                         {&m_color, "gColor"}});
}