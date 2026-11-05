#include "gl_blur_filter2_technique.h"

#define SIGMA_H 25.0f

float Gauss(float x, float sigma);

bool BlurFilter2Technique::Init()
{
    if (!FullScreenTechnique::Init("framework/shaders/GL/blur_filter2.fs"))
    {
        return false;
    }

    if (!InitUniforms({{&m_sampler, "gSampler"},
                       {&m_blurScale, "gBlurScale"},
                       {&m_weights, "gWeights"}}))
    {
        return false;
    }

    Enable();
    m_sampler.Set(0);

    SetSigmaH(SIGMA_H);

    return true;
}

void BlurFilter2Technique::SetSigmaH(float sigma)
{
    float weights[NUM_WEIGHTS] = {0.0f}, sum = 0.0f;

    weights[0] = Gauss(0, sigma);
    sum = weights[0];

    for (int i = 1; i < NUM_WEIGHTS; i++)
    {
        weights[i] = Gauss(float(i), sigma);
        sum += 2 * weights[i];
    }

    for (int i = 0; i < NUM_WEIGHTS; i++)
    {
        float val = weights[i] / sum;
        SetWeight(i, val);
    }
}