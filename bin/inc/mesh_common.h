#pragma once

#include "world_transform.h"
#include <glm/glm.hpp>

class MeshCommon
{
public:
    MeshCommon() {}

    // void SetScale(float scale)
    // {
    //     m_worldTransform.SetScale(scale);
    // }

    // void SetRotation(float x, float y, float z)
    // {
    //     m_worldTransform.SetRotation(x, y, z);
    // }

    // void SetPosition(float x, float y, float z)
    // {
    //     m_worldTransform.SetPosition(x, y, z);
    // }

    glm::vec3 GetPosition() const { return m_worldTransform.GetPos(); }
    glm::mat4 GetWorldMatrix() { return m_worldTransform.GetMatrix(); }
    WorldTrans& GetWorldTransform() { return m_worldTransform; }

    virtual void Render() = 0;

protected:
    WorldTrans m_worldTransform;
};