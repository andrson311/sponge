#include <GLFW/glfw3.h>
#include "camera.h"

CameraFPS::CameraFPS(const glm::vec3 &position, const glm::vec3 &target,
                     const glm::vec3 &up, PersProjInfo &persProjInfo)
{
    Init(position, target, up, persProjInfo);
}

void CameraFPS::Init(const glm::vec3 &pos, const glm::vec3 &target)
{
    if (!m_initialized)
    {
        printf("Error: Call the other init method first\n");
        assert(0);
        exit(1);
    }

    Init(pos, target, m_up, m_persProjInfo);
}

void CameraFPS::Init(float zNear, float zFar)
{
    m_persProjInfo.zNear = zNear;
    m_persProjInfo.zFar = zFar;

    Init(m_cameraPosition, GetTarget(), m_up, m_persProjInfo);
}

void CameraFPS::Init(const glm::vec3 &position, const glm::vec3 &target,
                     const glm::vec3 &up, PersProjInfo &persProjInfo)
{
    m_cameraPosition = position;
    m_up = glm::normalize(up);
    m_persProjInfo = persProjInfo;
    m_oldMousePosition = m_mouseState.m_pos;

    glm::vec3 senseDir = ((glm::abs(m_up.y) > 0.9f)
                              ? glm::vec3(0.0f, 0.0f, 1.0f)
                              : glm::vec3(0.0f, 1.0f, 0.0f));

    m_worldRight = glm::normalize(glm::cross(senseDir, m_up));

    // 1. calculate initial yaw and pitch from target direction
    glm::vec3 direction = glm::normalize(target);
    m_yaw = glm::atan(direction.x, -direction.z);
    m_pitch = glm::asin(direction.y);

    // 2. build the initial m_cameraOrientation quaternion
    CalcCameraOrientation();

    // 3. set up the projection matrix
    float aspectRatio = (float)m_persProjInfo.Width / (float)persProjInfo.Height;
    m_persProjection = glm::perspective(glm::radians(persProjInfo.FOV), aspectRatio,
                                        persProjInfo.zNear, persProjInfo.zFar);

    m_initialized = true;
}

void CameraFPS::Update(float dt)
{
    if (m_mouseState.m_isLeftButtonPressed || m_mouseState.m_isRightButtonPressed)
    {
        CalcCameraOrientation();
    }

    m_oldMousePosition = m_mouseState.m_pos;

    CalcVelocity(dt);

    m_cameraPosition += m_velocity * dt;
}

void CameraFPS::SetMousePosition(float x, float y)
{
    m_mouseState.m_pos.x = x / (float)m_persProjInfo.Width;
    m_mouseState.m_pos.y = y / (float)m_persProjInfo.Height;
}

void CameraFPS::HandleMouseButton(int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            m_mouseState.m_isLeftButtonPressed = true;
            m_isFirstClick = true;
        }
        else if (action == GLFW_RELEASE)
        {
            m_mouseState.m_isLeftButtonPressed = false;
        }
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
        {
            m_mouseState.m_isRightButtonPressed = true;
            m_isFirstClick = true;
        }
        else if (action == GLFW_RELEASE)
        {
            m_mouseState.m_isRightButtonPressed = false;
        }
    }
}

void CameraFPS::CalcCameraOrientation()
{
    if (m_isFirstClick)
    {
        m_oldMousePosition = m_mouseState.m_pos;
        m_isFirstClick = false;
    }

    glm::vec2 deltaMouse = m_mouseState.m_pos - m_oldMousePosition;

    if (m_mouseState.m_isRightButtonPressed)
    {
        HandleRightMouseDrag(deltaMouse);
    }
    else
    {
        HandleLeftMouseDrag(deltaMouse);
    }
}

void CameraFPS::HandleRightMouseDrag(const glm::vec2 &delta)
{
    float pitch = delta.y * m_mouseSpeed;
    float yaw = delta.x * m_mouseSpeed;

    // glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f));

    m_globalWorldRotation = qYaw * m_globalWorldRotation; // * qPitch
    m_globalWorldRotation = glm::normalize(m_globalWorldRotation);
}

void CameraFPS::HandleLeftMouseDrag(const glm::vec2 &delta)
{
    m_yaw += delta.x * m_mouseSpeed;
    m_pitch += delta.y * m_mouseSpeed;

    if (m_pitch > 1.5f)
        m_pitch = 1.5f;
    if (m_pitch < -1.5f)
        m_pitch = -1.5f;

    glm::quat qYaw, qPitch;
    qYaw = glm::angleAxis(m_yaw, m_up);
    qPitch = glm::angleAxis(m_pitch, m_worldRight);

    m_cameraOrientation = qPitch * qYaw;
    m_cameraOrientation = glm::normalize(m_cameraOrientation);
}

void CameraFPS::CalcVelocity(float dt)
{
    glm::vec3 acceleration = CalcAcceleration();

    if (acceleration == glm::vec3(0.0f))
    {
        m_velocity -= m_velocity * std::min(dt * m_damping, 1.0f);
    }
    else
    {
        m_velocity += acceleration * m_acceleration * dt;
        float maxSpeed = m_movement.fastSpeed ? m_maxSpeed * m_fastCoef : m_maxSpeed;
        if (glm::length(m_velocity) > maxSpeed)
        {
            m_velocity = glm::normalize(m_velocity) * maxSpeed;
        }
    }
}

glm::vec3 CameraFPS::CalcAcceleration()
{
    glm::mat4 v = glm::mat4_cast(m_cameraOrientation);
    glm::vec3 right = glm::vec3(v[0][0], v[1][0], v[2][0]);
    glm::vec3 forward = -glm::vec3(v[0][2], v[1][2], v[2][2]) * 0.1f;
    glm::vec3 up = glm::cross(right, forward);

    glm::vec3 acceleration(0.0f);

    if (m_movement.forward)
    {
        if (m_movement.ctrl)
        {
            forward.y = 0;
        }

        acceleration += forward;
    }

    if (m_movement.backward)
    {
        if (m_movement.ctrl)
        {
            forward.y = 0;
        }

        acceleration -= forward;
    }

    if (m_movement.strafeLeft)
    {
        acceleration -= right;
    }

    if (m_movement.strafeRight)
    {
        acceleration += right;
    }

    if (m_movement.up)
    {
        acceleration += up;
    }

    if (m_movement.down)
    {
        acceleration -= up;
    }

    if (m_movement.fastSpeed)
    {
        acceleration *= m_fastCoef;
    }

    if (m_movement.plus)
    {
        m_maxSpeed += 0.1f;
        m_acceleration += 0.01f;
    }

    if (m_movement.minus)
    {
        m_maxSpeed -= 0.1f;

        if (m_maxSpeed <= 0.1f)
        {
            m_maxSpeed = 0.1f;
        }

        m_acceleration -= 0.01f;

        if (m_acceleration <= 10.0f)
        {
            m_acceleration = 10.0f;
        }
    }

    return acceleration;
}

glm::mat4 CameraFPS::GetViewMatrix() const
{
    glm::mat4 t = glm::translate(glm::mat4(1.0f), -m_cameraPosition);
    glm::mat4 r = glm::mat4_cast(m_cameraOrientation);

    return r * t;
}

glm::mat4 CameraFPS::GetVPMatrix() const
{
    glm::mat4 view = GetViewMatrix();
    glm::mat4 VP = m_persProjection * view;

    return VP;
}

glm::mat4 CameraFPS::GetVPMatrixNoTranslate() const
{
    glm::mat4 view = glm::mat4_cast(m_cameraOrientation);
    glm::mat4 VP = m_persProjection * view;

    return VP;
}

void CameraFPS::SetTarget(const glm::vec3 &target)
{
    Init(m_cameraPosition, target, m_up, m_persProjInfo);
}

void CameraFPS::SetUp(const glm::vec3 &up)
{
    Init(m_cameraPosition, GetTarget(), up, m_persProjInfo);
}

void CameraFPS::SetCenter(const glm::vec3 &center)
{
    glm::vec3 target = center - m_cameraPosition;
    Init(m_cameraPosition, target, m_up, m_persProjInfo);
}

glm::vec3 CameraFPS::GetTarget() const
{
    glm::mat4 view = GetViewMatrix();
    glm::vec3 forward = glm::vec3(view[0][2], view[1][2], view[2][2]);

    return forward;
}

glm::mat4 CameraFPS::GetGlobalWorldRotation() const
{
    return glm::mat4_cast(m_globalWorldRotation);
}
