#pragma once

#include "technique.h"
#include "math_3d.h"

class SSAOTechnique : public Technique
{
public:
    SSAOTechnique() = default;

    virtual bool Init() override;
    void Render();

private:
    GLuint m_dummyVAO = -1;
};
