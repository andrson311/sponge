#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "basic_glfw_camera.h"

static int MARGIN = 40;
static float EDGE_STEP = 0.5f;

BasicCamera::BasicCamera(int windowWidth, int windowHeight)
{
    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;

    m_pos = glm::vec3(0.0f);
    m_target = glm::vec3(0.0f, 0.0f, 1.0f);
    m_up = glm::vec3(0.0f, 1.0f, 0.0f);

    InitInternal();
}

BasicCamera::BasicCamera(const PersProjInfo &persProjInfo, const glm::vec3 &pos, const glm::vec3 &target, const glm::vec3 &up)
{
    InitCamera(persProjInfo, pos, target, up);
}

BasicCamera::BasicCamera(const OrthoProjInfo &orthoProjInfo, const glm::vec3 &pos, const glm::vec3 &target, const glm::vec3 &up)
{
    InitCamera(orthoProjInfo, pos, target, up);
}

void BasicCamera::InitCamera(const PersProjInfo &persProjInfo, const glm::vec3 &pos, const glm::vec3 &target, const glm::vec3 &up)
{
    m_persProjInfo = persProjInfo;
    m_projection = glm::perspectiveFov(glm::radians(persProjInfo.FOV),
                                       persProjInfo.Width, persProjInfo.Height,
                                       persProjInfo.zNear, persProjInfo.zFar);
    m_windowWidth = (int)persProjInfo.Width;
    m_windowHeight = (int)persProjInfo.Height;

    m_pos = pos;
    m_target = glm::normalize(target);
    m_up = glm::normalize(up);

    InitInternal();
}

void BasicCamera::InitCamera(const OrthoProjInfo &orthoProjInfo, const glm::vec3 &pos, const glm::vec3 &target, const glm::vec3 &up)
{
    m_projection = glm::ortho(orthoProjInfo.l, orthoProjInfo.r,
                              orthoProjInfo.b, orthoProjInfo.t,
                              orthoProjInfo.n, orthoProjInfo.f);
    m_windowWidth = (int)orthoProjInfo.Width;
    m_windowHeight = (int)orthoProjInfo.Height;

    m_pos = pos;
    m_target = glm::normalize(target);
    m_up = glm::normalize(up);

    InitInternal();
}

void BasicCamera::InitInternal()
{
    glm::vec3 HTarget(m_target.x, 0.0f, m_target.z);
    HTarget = glm::normalize(HTarget);

    m_angleH = -glm::degrees(atan2f(m_target.z, m_target.x));
    m_angleV = -glm::degrees(asinf(m_target.y));

    m_onUpperEdge = false;
    m_onLowerEdge = false;
    m_onLeftEdge = false;
    m_onRightEdge = false;

    m_mousePos.x = m_windowWidth / 2;
    m_mousePos.y = m_windowHeight / 2;
}

void BasicCamera::SetPosition(float x, float y, float z)
{
    m_pos.x = x;
    m_pos.y = y;
    m_pos.z = z;
}

void BasicCamera::SetPosition(const glm::vec3 &pos)
{
    SetPosition(pos.x, pos.y, pos.z);
}

void BasicCamera::SetTarget(float x, float y, float z)
{
    m_target.x = x;
    m_target.y = y;
    m_target.z = z;
    m_target = glm::normalize(m_target);

    InitInternal();
}

void BasicCamera::SetTarget(const glm::vec3 &target)
{
    SetTarget(target.x, target.y, target.z);
}

bool BasicCamera::OnKeyboard(int key)
{
    bool cameraChangesPos = false;

    switch (key)
    {
    case GLFW_KEY_W:
        m_pos += (m_target * m_speed);
        cameraChangesPos = true;
        break;

    case GLFW_KEY_S:
        m_pos -= (m_target * m_speed);
        cameraChangesPos = true;
        break;

    case GLFW_KEY_A:
    {
        glm::vec3 left = glm::normalize(glm::cross(m_target, m_up));
        left *= m_speed;
        m_pos += left;
        cameraChangesPos = true;
    }
    break;

    case GLFW_KEY_D:
    {
        glm::vec3 right = glm::normalize(glm::cross(m_up, m_target));
        right *= m_speed;
        m_pos += right;
        cameraChangesPos = true;
    }
    break;

    case GLFW_KEY_UP:
        m_angleV += m_speed;
        Update();
        break;

    case GLFW_KEY_DOWN:
        m_angleV -= m_speed;
        Update();
        break;

    case GLFW_KEY_LEFT:
        m_angleH -= m_speed;
        Update();
        break;

    case GLFW_KEY_RIGHT:
        m_angleH += m_speed;
        Update();
        break;

    case GLFW_KEY_PAGE_UP:
        m_pos.y += m_speed;
        cameraChangesPos = true;
        break;

    case GLFW_KEY_PAGE_DOWN:
        m_pos.y -= m_speed;
        cameraChangesPos = true;
        break;

    case GLFW_KEY_KP_ADD:
        m_speed += 0.1f;
        printf("Speed changed to %f\n", m_speed);
        break;

    case GLFW_KEY_KP_SUBTRACT:
        m_speed -= 0.1f;
        if (m_speed < 0.1f)
        {
            m_speed = 0.1f;
        }

        printf("Speed changed to %f\n", m_speed);
        break;

    default:
        break;
    }

    return cameraChangesPos;
}

void BasicCamera::OnMouse(int x, int y)
{
    int deltaX = x - m_mousePos.x;
    int deltaY = y - m_mousePos.y;

    m_mousePos.x = x;
    m_mousePos.y = y;

    m_angleH += (float)deltaX / 20.0f;
    m_angleV += (float)deltaY / 20.0f;

    if (x <= MARGIN)
    {
        m_onLeftEdge = true;
        m_onRightEdge = false;
    }
    else if (x >= (m_windowWidth - MARGIN))
    {
        m_onRightEdge = true;
        m_onLeftEdge = false;
    }
    else
    {
        m_onLeftEdge = false;
        m_onRightEdge = false;
    }

    if (y <= MARGIN)
    {
        m_onUpperEdge = true;
        m_onLowerEdge = false;
    }
    else if (y >= (m_windowHeight - MARGIN))
    {
        m_onLowerEdge = true;
        m_onUpperEdge = false;
    }
    else
    {
        m_onUpperEdge = false;
        m_onLowerEdge = false;
    }

    Update();
}

void BasicCamera::UpdateMousePosSilent(int x, int y)
{
    m_mousePos.x = x;
    m_mousePos.y = y;
}

void BasicCamera::OnRender()
{
    bool shouldUpdate = false;

    if (m_onLeftEdge)
    {
        m_angleH -= EDGE_STEP;
        shouldUpdate = true;
    }
    else if (m_onRightEdge)
    {
        m_angleH += EDGE_STEP;
        shouldUpdate = true;
    }

    if (m_onUpperEdge)
    {
        if (m_angleV > -90.0f)
        {
            m_angleV -= EDGE_STEP;
            shouldUpdate = true;
        }
    }
    else if (m_onLowerEdge)
    {
        if (m_angleV < 90.0f)
        {
            m_angleV += EDGE_STEP;
            shouldUpdate = true;
        }
    }

    if (shouldUpdate)
    {
        Update();
    }
}

void BasicCamera::Update()
{
    glm::vec3 Yaxis(0.0f, 1.0f, 0.0f);

    glm::vec3 view(1.0f, 0.0f, 0.0f);
    view = glm::rotate(view, glm::radians(m_angleH), Yaxis);
    view = glm::normalize(view);

    glm::vec3 u = glm::normalize(glm::cross(Yaxis, view));
    view = glm::rotate(view, glm::radians(m_angleV), u);

    m_target = glm::normalize(view);
    m_up = glm::normalize(glm::cross(m_target, u));
}

glm::mat4 BasicCamera::GetMatrix() const
{
    return glm::lookAt(m_pos, m_pos + m_target, m_up);
}

glm::mat4 BasicCamera::GetViewProjMatrix() const
{
    glm::mat4 view = GetMatrix();
    glm::mat4 projection = GetProjectionMatrix();
    glm::mat4 VP = projection * view;
    return VP;
}

glm::mat4 BasicCamera::GetViewportMatrix() const
{
    float halfW = m_windowWidth / 2.0f;
    float halfH = m_windowHeight / 2.0f;

    glm::mat4 viewport = glm::mat4(halfW, 0.0f, 0.0f, 0.0f,
                                   0.0f, halfH, 0.0f, 0.0f,
                                   0.0f, 0.0f, 1.0f, 0.0f,
                                   halfW, halfH, 0.0f, 1.0f);
    return viewport;
}

void BasicCamera::SetSpeed(float speed)
{
    if (speed <= 0.0f)
    {
        printf("Invalid camera speed %f\n", speed);
        exit(0);
    }

    m_speed = speed;
}
