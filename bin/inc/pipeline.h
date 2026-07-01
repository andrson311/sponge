#pragma once

#include "util.h"
#include "camera.h"
#include "world_transform.h"

struct Orientation
{
    glm::vec3 m_scale;
    glm::vec3 m_rotation;
    glm::vec3 m_pos;

    Orientation()
    {
        m_scale = glm::vec3(1.0f);
        m_rotation = glm::vec3(0.0f);
        m_pos = glm::vec3(0.0f);
    }
};

class Pipeline
{
public:
    Pipeline()
    {
        m_scale = glm::vec3(1.0f);
        m_worldPos = glm::vec3(0.0f);
        m_rotateInfo = glm::vec3(0.0f);
    }

    void Scale(float s)
    {
        Scale(s, s, s);
    }

    void Scale(const glm::vec3 &scale)
    {
        Scale(scale.x, scale.y, scale.z);
    }

    void Scale(float ScaleX, float ScaleY, float ScaleZ)
    {
        m_scale.x = ScaleX;
        m_scale.y = ScaleY;
        m_scale.z = ScaleZ;
    }

    void WorldPos(float x, float y, float z)
    {
        m_worldPos.x = x;
        m_worldPos.y = y;
        m_worldPos.z = z;
    }

    void WorldPos(const glm::vec3 &Pos)
    {
        m_worldPos = Pos;
    }

    void Rotate(float RotateX, float RotateY, float RotateZ)
    {
        m_rotateInfo.x = RotateX;
        m_rotateInfo.y = RotateY;
        m_rotateInfo.z = RotateZ;
    }

    void Rotate(const glm::vec3 &r)
    {
        Rotate(r.x, r.y, r.z);
    }

    void SetPerspectiveProj(const PersProjInfo &p)
    {
        m_persProjInfo = p;
    }

    void SetOrthographicProj(const OrthoProjInfo &p)
    {
        m_orthoProjInfo = p;
    }

    void SetCamera(const glm::vec3 &Pos, const glm::vec3 &Target, const glm::vec3 &Up)
    {
        m_camera.Pos = Pos;
        m_camera.Target = Target;
        m_camera.Up = Up;
    }

    void SetCamera(const Camera &camera)
    {
        SetCamera(camera.GetPos(), camera.GetTarget(), camera.GetUp());
    }

    void Orient(const Orientation &o)
    {
        m_scale = o.m_scale;
        m_worldPos = o.m_pos;
        m_rotateInfo = o.m_rotation;
    }

    void Orient(const WorldTrans &w)
    {
        m_scale = glm::vec3(w.GetScale());
        m_rotateInfo = w.GetRotation();
        m_worldPos = w.GetPos();
    }

    const glm::mat4 &GetWPTrans();
    const glm::mat4 &GetWVTrans();
    const glm::mat4 &GetVPTrans();
    const glm::mat4 &GetWVPTrans();
    const glm::mat4 &GetWVOrthoPTrans();
    const glm::mat4 &GetWorldTrans();
    const glm::mat4 &GetViewTrans();
    const glm::mat4 &GetProjTrans();

private:
    glm::vec3 m_scale;
    glm::vec3 m_worldPos;
    glm::vec3 m_rotateInfo;

    PersProjInfo m_persProjInfo;
    OrthoProjInfo m_orthoProjInfo;

    struct
    {
        glm::vec3 Pos;
        glm::vec3 Target;
        glm::vec3 Up;
    } m_camera;

    glm::mat4 m_WVPtransformation;
    glm::mat4 m_VPtransformation;
    glm::mat4 m_WPtransformation;
    glm::mat4 m_WVtransformation;
    glm::mat4 m_Wtransformation;
    glm::mat4 m_Vtransformation;
    glm::mat4 m_ProjTransformation;
};
