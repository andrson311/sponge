#pragma once

#include <glm/glm.hpp>
#include "world_transform.h"
#include "material.h"

class IRenderCallbacks
{
public:
    virtual void DrawStartCB(u_int DrawIndex) {}
    virtual void ControlSpecularExponent(bool IsEnabled) {}
    virtual void SetMaterial(const Material &material) {}
    virtual void DisableDiffuseTexture() {}
};

class MeshCommon
{
public:
    MeshCommon() {}

    void SetScale(float scale);
    void SetRotation(float x, float y, float z);
    void SetPosition(float x, float y, float z);
    void SetPosition(const glm::vec3 &pos) { SetPosition(pos.x, pos.y, pos.z); }

    glm::vec3 GetPosition() const;
    virtual void Render(IRenderCallbacks *pRenderCallbacks = NULL) = 0;
    WorldTrans &GetWorldTransform() { return m_worldTransform; }
    glm::mat4 GetWorldMatrix() { return m_worldTransform.GetMatrix(); }

protected:
    WorldTrans m_worldTransform;
};

inline void MeshCommon::SetScale(float Scale)
{
    m_worldTransform.SetScale(Scale);
}

inline void MeshCommon::SetRotation(float x, float y, float z)
{
    m_worldTransform.SetRotation(x, y, z);
}

inline void MeshCommon::SetPosition(float x, float y, float z)
{
    m_worldTransform.SetPosition(x, y, z);
}

inline glm::vec3 MeshCommon::GetPosition() const
{
    return m_worldTransform.GetPos();
}