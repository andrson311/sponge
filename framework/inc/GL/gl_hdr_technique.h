#pragma once

#include "technique.h"
#include "math_3d.h"

class HDRTechnique : public Technique
{
public:
    HDRTechnique() = default;

    virtual bool Init() override;
};