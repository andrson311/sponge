#pragma once

#include "gl_full_screen_technique.h"

class BlurFilter2Technique : public FullScreenTechnique
{
public:
    BlurFilter2Technique() = default;

    bool Init();

    void SetSigmaH(float sigma);
    void SetBlurScale(float scale) { m_blurScale.Set(scale); }

private:
    void SetWeight(u_int index, float weight) { m_weights.Set(index, weight); }

    static constexpr int NUM_WEIGHTS = 10;

    Uniform<int> m_sampler;
    Uniform<float> m_blurScale;
    UniformArray<float, NUM_WEIGHTS> m_weights;
};
