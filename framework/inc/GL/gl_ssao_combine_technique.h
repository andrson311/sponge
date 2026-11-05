#pragma once

#include "technique.h"
#include "math_3d.h"

class SSAOCombineTechnique : public Technique
{
public:
    SSAOCombineTechnique() = default;

    virtual bool Init() override;

    void Render();

private:    
    GLuint m_dummyVAO = -1;
};
