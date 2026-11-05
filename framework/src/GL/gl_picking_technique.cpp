#include "gl_picking_technique.h"

bool PickingTechnique::Init()
{
    if (!InitShaders({{GL_VERTEX_SHADER, "framework/shaders/GL/picking.vs"},
                      {GL_FRAGMENT_SHADER, "framework/shaders/GL/picking.fs"}}))
    {
        return false;
    }

    return InitUniforms({{&m_WVP, "gWVP"},
                         {&m_drawIndex, "gDrawIndex"},
                         {&m_objectIndex, "gObjectIndex"}});
}
