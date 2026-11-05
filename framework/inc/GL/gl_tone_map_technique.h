#pragma once

#include "technique.h"
#include "math_3d.h"
#include "scene.h"

class ToneMapTechnique : public Technique
{
public:
    ToneMapTechnique(bool withBloom) { m_withBloom = withBloom; }

    virtual bool Init() override;

    void Render();

    void SetAverageLuminance(float avgLum) { m_avgLum.Set(avgLum); }
    void SetHDRSampler(u_int textureUnit) { m_hdrSampler.Set((int)textureUnit); }
    void SetBlurSampler(u_int textureUnit) { m_blurSampler.Set((int)textureUnit); }
    void SetExposure(float exposure) { m_exposure.Set(exposure); }
    void SetToneMapMethod(TONE_MAP_METHOD method) { m_methodType.Set(method); }
    void ControlGammaCorrection(bool enable) { m_enableGammaCorrection.Set(enable); }
    void SetBloomStrength(float strength) { m_bloomStrength.Set(strength); }

private:
    bool m_withBloom = false;

    GLuint m_dummyVAO = -1;

    Uniform<float> m_avgLum;
    Uniform<int> m_hdrSampler;
    Uniform<float> m_exposure;
    Uniform<TONE_MAP_METHOD> m_methodType;
    Uniform<bool> m_enableGammaCorrection;
    Uniform<int> m_blurSampler;
    Uniform<float> m_bloomStrength;
};
