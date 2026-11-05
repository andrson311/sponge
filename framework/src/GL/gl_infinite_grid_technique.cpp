#include "gl_infinite_grid_technique.h"

bool InfiniteGridTechnique::Init()
{
    if (!InitShaders({{GL_VERTEX_SHADER, "framework/shaders/GL/infinite_grid.vs"},
                      {GL_FRAGMENT_SHADER, "framework/shaders/GL/infinite_grid.fs"}}))
    {
        return false;
    }

    return InitUniforms({{&m_VP, "gVP"},
                         {&m_cameraWorldPos, "gCameraWorldPos"},
                         {&m_gridCellSize, "gGridCellSize"},
                         {&m_lightVP, "gLightVP"},
                         {&m_shadowMap, "gShadowMap"},
                         {&m_lightDir, "gLightDir"},
                         {&m_shadowsEnabled, "gShadowsEnabled"}});
}
