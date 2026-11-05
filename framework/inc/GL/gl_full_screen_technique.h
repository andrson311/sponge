#pragma once

#include "technique.h"
#include "math_3d.h"

class FullScreenTechnique : public Technique
{
public:
    FullScreenTechnique() = default;

    bool Init(const char* pFragmentShaderFile);
    void Render();

private:
    GLuint m_dummyVAO = -1; 
};
