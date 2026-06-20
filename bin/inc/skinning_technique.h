#pragma once

#include "technique.h"
#include "lighting_technique.h"

class SkinningTechnique : public LightingTechnique
{
public:
    SkinningTechnique() {};

    virtual bool Init();
    void SetBoneTransform(u_int Index, const glm::mat4 &Transform);

private:
    GLuint m_boneLocation[MAX_BONES];
};