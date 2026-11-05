#include "gl_forward_skinning.h"

bool ForwardSkinningTechnique::Init()
{
    if (!InitShaders({{GL_VERTEX_SHADER, "framework/shaders/GL/forward_skinning.vs"},
                      {GL_FRAGMENT_SHADER, "framework/shaders/GL/forward_lighting.fs"}}))
    {
        return false;
    }

    if (!ForwardLightingTechnique::ResolveUniforms())
    {
        return false;
    }

    return InitUniforms({{&m_boneTransforms, "gBones"}});
}

bool PBRSkinningTechnique::Init()
{
    if (!InitShaders({{GL_VERTEX_SHADER, "framework/shaders/GL/forward_skinning.vs"},
                      {GL_FRAGMENT_SHADER, "framework/shaders/GL/pbr_forward_lighting.fs"}}))
    {
        return false;
    }

    if (!PBRForwardLightingTechnique::ResolveUniforms())
    {
        return false;
    }

    return InitUniforms({{&m_boneTransform, "gBones"}});
}
