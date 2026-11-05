#pragma once

#include "gl_full_screen_technique.h"

class BlurFilter1Technique : public FullScreenTechnique
{
public:
    BlurFilter1Technique() = default;

    bool Init();

    void SetSigmaV(float sigma);

private:
    void SetWeight(u_int index, float weight) { m_weights.Set(index, weight); }

    static constexpr int NUM_WEIGHTS = 10;

    Uniform<int> m_sampler;
    UniformArray<float, NUM_WEIGHTS> m_weights;
};
