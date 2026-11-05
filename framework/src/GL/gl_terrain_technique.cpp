#include "scene.h"
#include "gl_terrain_technique.h"

bool TerrainTechnique::Init()
{
    if (!InitShaders({{GL_VERTEX_SHADER, "framework/shaders/GL/terrain.vs"},
                      {GL_FRAGMENT_SHADER, "framework/shaders/GL/terrain.fs"}}))
    {
        return false;
    }

    return InitUniforms({{&m_WVP, "gWVP"},
                         {&m_maxTerrainHeight, "gMaxTerrainHeight"},
                         {&m_horizontalScale, "gHorizontalScale"},
                         {&m_lowHeightPercent, "gLowHeightPercent"},
                         {&m_highHeightPercent, "gHighHeightPercent"},
                         {&m_sunlightDir, "gSunlightDir"},
                         {&m_ambientFactor, "gAmbientFactor"},
                         {&m_texCoordScale, "gTexCoordScale"},
                         {&m_renderMode, "gRenderMode"}});
}
