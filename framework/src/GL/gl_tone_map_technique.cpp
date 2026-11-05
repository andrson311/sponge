#include "gl_tone_map_technique.h"

bool ToneMapTechnique::Init()
{
    const char *pFragmentShader = m_withBloom
                                      ? "framework/shaders/GL/tone_map_with_bloom.fs"
                                      : "framework/shaders/GL/tone_map.fs";

    if (!InitShaders({{GL_VERTEX_SHADER, "framework/shaders/GL/full_screen_quad.vs"},
                      {GL_FRAGMENT_SHADER, pFragmentShader}}))
    {
        return false;
    }

    glGenVertexArrays(1, &m_dummyVAO);

    bool ok = InitUniforms({{&m_avgLum, "gAvgLum"},
                            {&m_hdrSampler, "gHDRSampler"},
                            {&m_exposure, "gExposure"},
                            {&m_methodType, "gMethodType"},
                            {&m_enableGammaCorrection, "gEnableGammaCorrection"}});

    if (m_withBloom)
    {
        ok = InitUniforms({{&m_blurSampler, "gBlurSampler"},
                           {&m_bloomStrength, "gBloomStrength"}}) &&
             ok;
    }

    return ok;
}

void ToneMapTechnique::Render()
{
    glBindVertexArray(m_dummyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}