#include "gl_skybox_technique.h"

bool SkyboxTechnique::Init()
{
    if (!InitShaders({{GL_VERTEX_SHADER, "framework/shaders/GL/skybox.vs"},
                      {GL_FRAGMENT_SHADER, "framework/shaders/GL/skybox.fs"}}))
    {
        return false;
    }

    return InitUniforms({{&m_VP, "gVP"},
                         {&m_cubemapSampler, "gCubemapTexture"}});
}