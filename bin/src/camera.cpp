#include "../inc/camera.h"
#include <GL/freeglut.h>
#include <stdio.h>
#include <glm/gtc/quaternion.hpp>

static int MARGIN = 10;
static float EDGE_STEP = 1.0f;

Camera::Camera(int WindowWidth, int WindowHeight)
{
    m_windowWidth = WindowWidth;
    m_windowHeight = WindowHeight;
    m_pos = glm::vec3(0.0f, 0.0f, 0.0f);
    m_target = glm::vec3(0.0f, 0.0f, -1.0f);
    m_up = glm::vec3(0.0f, 1.0f, 0.0f);

    Init();
}

Camera::Camera(
    int WindowWidth,
    int WindowHeight,
    const glm::vec3 &Pos,
    const glm::vec3 &Target,
    const glm::vec3 &Up)
{
    m_windowWidth = WindowWidth;
    m_windowHeight = WindowHeight;
    m_pos = Pos;
    m_target = glm::normalize(Target);
    m_up = glm::normalize(Up);

    Init();
}

void Camera::Init()
{
    glm::vec3 HTarget = glm::normalize(glm::vec3(m_target.x, 0.0f, m_target.z));

    float Angle = glm::degrees(asin(abs(HTarget.z)));

    if (HTarget.z >= 0.0f)
    {
        if (HTarget.x >= 0.0f)
        {
            m_AngleH = 360.0f - Angle;
        }
        else
        {
            m_AngleH = 180.0f + Angle;
        }
    }
    else
    {
        if (HTarget.x >= 0.0f)
        {
            m_AngleH = Angle;
        }
        else
        {
            m_AngleH = 180.0f - Angle;
        }
    }

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

void Camera::OnKeyboard(u_char Key)
{
    switch (Key)
    {
    case GLUT_KEY_UP:
        m_pos += m_target * m_speed;
        break;
    case GLUT_KEY_DOWN:
        m_pos -= m_target * m_speed;
        break;
    case GLUT_KEY_LEFT:
    {
        glm::vec3 Left = glm::normalize(glm::cross(m_target, m_up));
        m_pos += Left * m_speed;
    }
    break;
    case GLUT_KEY_RIGHT:
    {
        glm::vec3 Right = glm::normalize(glm::cross(m_up, m_target));
        m_pos += Right * m_speed;
    }
    break;
    case GLUT_KEY_PAGE_UP:
        m_pos.y += m_speed;
        break;
    case GLUT_KEY_PAGE_DOWN:
        m_pos.y -= m_speed;
        break;
    case '+':
        m_speed += 0.1f;
        printf("Speed changed to %f\n", m_speed);
        break;
    case '-':
        m_speed -= 0.1f;
        printf("Speed changed to %f\n", m_speed);
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

    if (DeltaX == 0)
    {
        if (x <= MARGIN)
            m_OnLeftEdge = true;
        else if (x >= (m_windowWidth - MARGIN))
            m_OnRightEdge = true;
    }
    else
    {
        m_OnLeftEdge = false;
        m_OnRightEdge = false;
    }

    if (DeltaY == 0)
    {
        if (y <= MARGIN)
            m_OnUpperEdge = true;
        else if (y >= (m_windowHeight - MARGIN))
            m_OnLowerEdge = true;
    }
    else
    {
        m_OnUpperEdge = false;
        m_OnLowerEdge = false;
    }

    Update();
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
        Update();
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

glm::mat4 Camera::GetMatrix()
{
    return glm::lookAt(
        m_pos,
        m_pos + m_target,
        m_up);
}
