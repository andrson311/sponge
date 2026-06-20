#include "../inc/camera.h"
#include <GL/freeglut.h>
#include <stdio.h>
#include <glm/gtc/quaternion.hpp>
#include <GLFW/glfw3.h>

static int MARGIN = 40;
static float EDGE_STEP = 0.5f;

Camera::Camera(int WindowWidth, int WindowHeight)
{
    m_windowWidth = WindowWidth;
    m_windowHeight = WindowHeight;
    m_pos = glm::vec3(0.0f, 0.0f, 0.0f);
    m_target = glm::vec3(0.0f, 0.0f, -1.0f);
    m_up = glm::vec3(0.0f, 1.0f, 0.0f);

    InitInternal();
}

Camera::Camera(const PersProjInfo &persProjInfo, const glm::vec3 &Pos, const glm::vec3 &Target, const glm::vec3 &Up)
{
    InitCamera(persProjInfo, Pos, Target, Up);
}

Camera::Camera(const OrthoProjInfo &orthoProjInfo, const glm::vec3 &Pos, const glm::vec3 &Target, const glm::vec3 &Up)
{
    InitCamera(orthoProjInfo, Pos, Target, Up);
}

void Camera::InitCamera(const PersProjInfo &persProjInfo, const glm::vec3 &Pos, const glm::vec3 &Target, const glm::vec3 &Up)
{
    m_persProjInfo = persProjInfo;
    float aspectRatio = (float)persProjInfo.Width / (float)persProjInfo.Height;
    m_projection = glm::perspective(
        glm::radians(persProjInfo.FOV),
        aspectRatio,
        persProjInfo.zNear,
        persProjInfo.zFar);

    m_windowWidth = (int)persProjInfo.Width;
    m_windowHeight = (int)persProjInfo.Height;

    m_pos = Pos;
    m_target = glm::normalize(Target);
    m_up = glm::normalize(Up);

    InitInternal();
}

void Camera::InitCamera(const OrthoProjInfo &orthoProjInfo, const glm::vec3 &Pos, const glm::vec3 &Target, const glm::vec3 &Up)
{
    m_projection = glm::ortho(
        orthoProjInfo.l,
        orthoProjInfo.r,
        orthoProjInfo.b,
        orthoProjInfo.t,
        orthoProjInfo.n,
        orthoProjInfo.f);

    m_windowWidth = (int)orthoProjInfo.Width;
    m_windowHeight = (int)orthoProjInfo.Height;

    m_pos = Pos;
    m_target = glm::normalize(Target);
    m_up = glm::normalize(Up);

    InitInternal();
}

void Camera::InitInternal()
{
    glm::vec3 HTarget = glm::normalize(glm::vec3(m_target.x, 0.0f, m_target.z));

    m_AngleH = -glm::degrees(atan2(m_target.z, m_target.x));
    m_AngleV = -glm::degrees(asin(m_target.y));

    m_OnUpperEdge = false;
    m_OnLowerEdge = false;
    m_OnLeftEdge = false;
    m_OnRightEdge = false;
    m_mousePos = glm::ivec2(m_windowWidth / 2, m_windowHeight / 2);
}

void Camera::SetPosition(float x, float y, float z)
{
    m_pos = glm::vec3(x, y, z);
}

void Camera::SetPosition(const glm::vec3 &pos)
{
    m_pos = pos;
}

void Camera::SetTarget(float x, float y, float z)
{
    m_target = glm::vec3(x, y, z);
    InitInternal();
}

void Camera::SetTarget(const glm::vec3 &target)
{
    m_target = target;
    InitInternal();
}

void Camera::OnKeyboard(int key)
{
    bool CameraChangedPos = false;

    switch (key)
    {
    case GLFW_KEY_W:
        m_pos += (m_target * m_speed);
        CameraChangedPos = true;
        break;

    case GLFW_KEY_S:
        m_pos -= (m_target * m_speed);
        CameraChangedPos = true;
        break;

    case GLFW_KEY_A:
    {
        glm::vec3 Left = glm::normalize(glm::cross(m_up, m_target));
        Left *= m_speed;
        m_pos += Left;
        CameraChangedPos = true;
        break;
    }

    case GLFW_KEY_D:
    {
        glm::vec3 Right = glm::normalize(glm::cross(m_target, m_up));
        Right *= m_speed;
        m_pos += Right;
        CameraChangedPos = true;
        break;
    }

    case GLFW_KEY_UP:
        m_AngleV += m_speed;
        Update();
        break;

    case GLFW_KEY_DOWN:
        m_AngleV -= m_speed;
        Update();
        break;

    case GLFW_KEY_LEFT:
        m_AngleH -= m_speed;
        Update();
        break;

    case GLFW_KEY_RIGHT:
        m_AngleH += m_speed;
        Update();
        break;

    case GLFW_KEY_PAGE_UP:
        m_pos.y += m_speed;
        CameraChangedPos = true;
        break;

    case GLFW_KEY_PAGE_DOWN:
        m_pos.y -= m_speed;
        CameraChangedPos = true;
        break;

    case GLFW_KEY_KP_ADD:
        m_speed += 0.1f;
        printf("Speed increased to %f\n", m_speed);
        break;

    case GLFW_KEY_KP_SUBTRACT:
        m_speed -= 0.1f;
        if (m_speed < 0.1f)
        {
            m_speed = 0.1f;
        }
        printf("Speed decreased to %f\n", m_speed);
        break;

    default:
        break;
    }
}

void Camera::OnMouse(int x, int y)
{
    int DeltaX = x - m_mousePos.x;
    int DeltaY = y - m_mousePos.y;

    m_mousePos = glm::ivec2(x, y);

    m_AngleH -= (float)DeltaX / 20.0f;
    m_AngleV += (float)DeltaY / 20.0f;

    if (x <= MARGIN)
    {
        m_OnLeftEdge = true;
        m_OnRightEdge = false;
    }
    else if (x >= (m_windowWidth - MARGIN))
    {
        m_OnRightEdge = true;
        m_OnLeftEdge = false;
    }
    else
    {
        m_OnLeftEdge = false;
        m_OnRightEdge = false;
    }

    if (y <= MARGIN)
    {
        m_OnUpperEdge = true;
        m_OnLowerEdge = false;
    }
    else if (y >= (m_windowHeight - MARGIN))
    {
        m_OnLowerEdge = true;
        m_OnUpperEdge = false;
    }
    else
    {
        m_OnLowerEdge = false;
        m_OnUpperEdge = false;
    }

    Update();
}

void Camera::UpdateMousePosSilent(int x, int y)
{
    m_mousePos = glm::ivec2(x, y);
}

void Camera::OnRender()
{
    bool ShouldUpdate = false;

    if (m_OnLeftEdge)
    {
        m_AngleH += EDGE_STEP;
        ShouldUpdate = true;
    }
    else if (m_OnRightEdge)
    {
        m_AngleH -= EDGE_STEP;
        ShouldUpdate = true;
    }

    if (m_OnUpperEdge)
    {
        if (m_AngleV > -90.0f)
        {
            m_AngleV -= EDGE_STEP;
            ShouldUpdate = true;
        }
    }
    else if (m_OnLowerEdge)
    {
        if (m_AngleV < 90.0f)
        {
            m_AngleV += EDGE_STEP;
            ShouldUpdate = true;
        }
    }

    if (ShouldUpdate)
    {
        Update();
    }
}

void Camera::Update()
{
    glm::vec3 Yaxis(0.0f, 1.0f, 0.0f);

    // Horizontal rotation around Y axis using quaternion
    glm::quat hRot = glm::angleAxis(glm::radians(m_AngleH), Yaxis);
    glm::vec3 View = glm::normalize(hRot * glm::vec3(1.0f, 0.0f, 0.0f));

    // Vertical rotation around horizontal axis using quaternion
    glm::vec3 U = glm::normalize(glm::cross(Yaxis, View));
    glm::quat vRot = glm::angleAxis(glm::radians(m_AngleV), U);
    View = glm::normalize(vRot * View);

    m_target = glm::normalize(View);
    m_up = glm::normalize(glm::cross(m_target, U));
}

glm::mat4 Camera::GetMatrix() const
{
    return glm::lookAt(
        m_pos,
        m_pos + m_target,
        m_up);
}

glm::mat4 Camera::GetViewProjMatrix() const
{
    glm::mat4 View = GetMatrix();
    glm::mat4 Projection = GetProjectionMat();
    glm::mat4 ViewProj = Projection * View;
    return ViewProj;
}

glm::mat4 Camera::GetViewportMatrix() const
{
    float HalfW = m_windowWidth / 2.0f;
    float HalfH = m_windowHeight / 2.0f;

    glm::mat4 Viewport = glm::mat4(
        HalfW, 0.0f, 0.0f, 0.0f,
        0.0f, HalfH, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        HalfW, HalfH, 0.0f, 1.0f);

    return Viewport;
}

void Camera::SetSpeed(float Speed)
{
    if (Speed <= 0.0f)
    {
        printf("Invalid camera speed %f\n", Speed);
        exit(0);
    }
    
    m_speed = Speed;
}
