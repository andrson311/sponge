#pragma once

#include "technique.h"
#include "math_3d.h"
#include "gl_forward_lighting.h"
#include "gl_pbr_forward_lighting.h"

class ForwardSkinningTechnique : public ForwardLightingTechnique
{
public:
    ForwardSkinningTechnique() = default;

    virtual bool Init() override;
    void SetBoneTransform(u_int index, const glm::mat4 &transform) { m_boneTransforms.Set(index, transform); }

private:
    UniformArray<glm::mat4, MAX_BONES> m_boneTransforms;
};

class PBRSkinningTechnique : public PBRForwardLightingTechnique
{
public:
    PBRSkinningTechnique() = default;

    virtual bool Init() override;
    void SetBoneTransform(u_int index, const glm::mat4 &transform) { m_boneTransform.Set(index, transform); }

private:
    UniformArray<glm::mat4, MAX_BONES> m_boneTransform;
};
