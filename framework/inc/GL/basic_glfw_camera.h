#pragma once

#include "math_3d.h"
#include "camera_api.h"

class BasicCamera : public CameraAPI
{
public:
    BasicCamera() = default;
    BasicCamera(int windowWidth, int windowHeight);
    BasicCamera(const PersProjInfo &persProjInfo, const glm::vec3 &pos, const glm::vec3 &target, const glm::vec3 &up);
    BasicCamera(const OrthoProjInfo &orthoProjInfo, const glm::vec3 &pos, const glm::vec3 &target, const glm::vec3 &up);

    void InitCamera(const PersProjInfo &persProjInfo, const glm::vec3 &pos, const glm::vec3 &target, const glm::vec3 &up);
    void InitCamera(const OrthoProjInfo &orthoProjInfo, const glm::vec3 &pos, const glm::vec3 &target, const glm::vec3 &up);

    void SetPosition(float x, float y, float z);
    void SetPosition(const glm::vec3 &pos);

    void SetTarget(float x, float y, float z);
    void SetTarget(const glm::vec3 &target);

    void SetUp(float x, float y, float z)
    {
        m_up.x = x;
        m_up.y = y;
        m_up.z = z;
    }

    bool OnKeyboard(int key);
    void OnMouse(int x, int y);
    void UpdateMousePosSilent(int x, int y);
    void OnRender();

    glm::mat4 GetMatrix() const;
    const glm::vec3 GetPos() const { return m_pos; }
    const glm::vec3 &GetTarget() const { return m_target; }
    const glm::vec3 &GetUp() const { return m_up; }

    const glm::mat4 GetProjectionMatrix() const { return m_projection; }
    const PersProjInfo &GetPersProjInfo() const { return m_persProjInfo; }

    glm::mat4 GetViewProjMatrix() const;
    glm::mat4 GetViewMatrix() const { return GetMatrix(); }
    glm::mat4 GetViewportMatrix() const;

    void SetSpeed(float speed);
    void SetName(const std::string &name) { m_name = name; }
    const std::string &GetName() const { return m_name; }

private:
    void InitInternal();
    void Update();

    std::string m_name;

    glm::vec3 m_pos;
    glm::vec3 m_target;
    glm::vec3 m_up;

    float m_speed = 0.2f;
    int m_windowWidth = 0;
    int m_windowHeight = 0;

    float m_angleH = 0.0f;
    float m_angleV = 0.0f;

    bool m_onUpperEdge = false;
    bool m_onLowerEdge = false;
    bool m_onLeftEdge = false;
    bool m_onRightEdge = false;

    glm::ivec2 m_mousePos = glm::ivec2(0, 0);

    PersProjInfo m_persProjInfo;
    glm::mat4 m_projection;
};