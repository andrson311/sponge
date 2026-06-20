#include "skinning_technique.h"

bool SkinningTechnique::Init()
{
    if (!Technique::Init())
    {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "bin/shaders/skinning.vs"))
    {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "bin/shaders/lighting.fs"))
    {
        return false;
    }

    if (!Finalize())
    {
        return false;
    }

    if (!InitCommon())
    {
        return false;
    }

    for (u_int i = 0; i < std::size(m_boneLocation); i++)
    {
        char Name[128];
        memset(Name, 0, sizeof(Name));
        snprintf(Name, sizeof(Name), "gBones[%d]", i);
        m_boneLocation[i] = GetUniformLocation(Name);
    }

    return true;
}

void SkinningTechnique::SetBoneTransform(u_int Index, const glm::mat4 &Transform)
{
    if (Index >= MAX_BONES)
    {
        return;
    }

    glUniformMatrix4fv(m_boneLocation[Index], 1, GL_FALSE, glm::value_ptr(Transform));
}
