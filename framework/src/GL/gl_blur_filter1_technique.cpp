#include "gl_blur_filter1_technique.h"

#define SIGMA_V 25.0f

float Gauss(float x, float sigma)
{
    double coeff = 1.0 / (glm::two_pi<double>() * sigma);
    double expon = -(x * x) / (2.0 * sigma);
    return (float)(coeff * exp(expon));
}

bool BlurFilter1Technique::Init()
{
    if (!FullScreenTechnique::Init("framework/shaders/GL/blur_filter1.fs"))
    {
        return false;
    }

    if (!InitUniforms({{&m_sampler, "gSampler"},
                       {&m_weights, "gWeights"}}))
    {
        return false;
    }

    Enable();
    m_sampler.Set(0);

    SetSigmaV(SIGMA_V);

    return true;
}

void BlurFilter1Technique::SetSigmaV(float sigma)
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
