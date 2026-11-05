#pragma once

#include "gl_full_screen_technique.h"

class BrightFilterTechnique : public FullScreenTechnique
{
public:
    BrightFilterTechnique() = default;

    bool Init();
    void SetLuminanceThreshold(float threshold) { m_luminanceThreshold.Set(threshold); }

private:
    Uniform<int> m_sampler;
    Uniform<float> m_luminanceThreshold;
};
