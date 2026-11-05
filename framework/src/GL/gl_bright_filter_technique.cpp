#include "gl_bright_filter_technique.h"

bool BrightFilterTechnique::Init()
{
    if (!FullScreenTechnique::Init("framework/shaders/GL/bright_filter.fs"))
    {
        return false;
    }

    if (!InitUniforms({{&m_luminanceThreshold, "gLuminanceTreshold"},
                       {&m_sampler, "gSampler"}}))
    {
        return false;
    }

    Enable();
    m_sampler.Set(0);

    return true;
}
