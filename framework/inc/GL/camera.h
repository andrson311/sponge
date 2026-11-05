#pragma once

#include "math_3d.h"
#include "glfw_camera_handler.h"
#include "camera_api.h"

struct MouseState
{
    glm::vec2 m_pos = glm::vec2(0.0f);
    bool m_isLeftButtonPressed = false;
    bool m_isRightButtonPressed = false;
};

class CameraFPS
{
public:
    CameraMovement m_movement;
    float m_damping = 5.0f;
    float m_maxSpeed = 10.0f;
    float m_fastCoef = 10.0f;
    float m_mouseSpeed = 4.0f;

    CameraFPS() {}
    CameraFPS(const glm::vec3 &position, const glm::vec3 &target,
              const glm::vec3 &up, PersProjInfo &perspectiveProjInfo);

    void Init(const glm::vec3 &pos, const glm::vec3 &target);
    void Init(const glm::vec3 &position, const glm::vec3 &target,
              const glm::vec3 &up, PersProjInfo &perspectiveProjInfo);
    void Init(float zNear, float zFar);

    void Update(float dt);

    void SetMousePosition(float x, float y);
    void HandleMouseButton(int button, int action, int mods);

    // get info
    const glm::mat4 GetProjectionMatrix() const { return m_persProjection; }
    glm::vec3 GetPosition() const { return m_cameraPosition; }
    glm::vec3 GetTarget() const;
    glm::vec3 GetUp() const { return m_up; }
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetVPMatrix() const;
    glm::mat4 GetVPMatrixNoTranslate() const;
    const PersProjInfo &GetPersProjInfo() const { return m_persProjInfo; }
    const glm::vec3 &GetVelocity() const { return m_velocity; }
    void SetAcceleration(float acceleration) { m_acceleration = acceleration; }
    glm::mat4 GetGlobalWorldRotation() const;

    // set info
    void SetPosition(const glm::vec3 &position) { m_cameraPosition = position; }
    void SetTarget(const glm::vec3 &target);
    void SetUp(const glm::vec3 &up);
    void SetCenter(const glm::vec3& center);
    void SetZ(float zNear, float zFar)
    {
        m_persProjInfo.zNear = zNear;
        m_persProjInfo.zFar = zFar;
    }

private:
    glm::vec3 CalcAcceleration();
    void CalcVelocity(float dt);
    void CalcCameraOrientation();

    void HandleLeftMouseDrag(const glm::vec2& delta);
    void HandleRightMouseDrag(const glm::vec2& delta);

    bool m_initialized = false;
    bool m_isFirstClick = true;
    float m_acceleration = 40.0f;
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;

    glm::mat4 m_persProjection = glm::mat4(0.0f);
    glm::vec3 m_cameraPosition = glm::vec3(0.0f);
    glm::quat m_cameraOrientation = glm::quat(glm::vec3(0.0f));
    glm::vec3 m_velocity = glm::vec3(0.0f);
    glm::vec2 m_oldMousePosition = glm::vec2(0.0f);
    glm::vec3 m_up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 m_worldRight = glm::vec3(1.0f, 0.0f, 0.0f);

    glm::quat m_globalWorldRotation = glm::quat(glm::vec3(0.0f));

    MouseState m_mouseState;
    PersProjInfo m_persProjInfo;
};
