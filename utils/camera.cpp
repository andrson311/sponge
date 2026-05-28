#include "camera.h"
#include <GL/freeglut.h>
#include <stdio.h>

Camera::Camera()
{
    m_pos = glm::vec3(0.0f, 0.0f, 0.0f);
    m_target = glm::vec3(0.0f, 0.0f, -1.0f);
    m_up = glm::vec3(0.0f, 1.0f, 0.0f);
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
        m_pos.y -=  m_speed;
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

glm::mat4 Camera::GetMatrix() {
    return glm::lookAt(
        m_pos,
        m_pos + m_target,
        m_up
    );
}
