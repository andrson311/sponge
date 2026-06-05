#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
    Camera(int WindowWidth, int WindowHeight);
    Camera(
        int WindowWidth,
        int WindowHeight,
        const glm::vec3 &Pos,
        const glm::vec3 &Target,
        const glm::vec3 &Up);

    void SetPosition(float x, float y, float z);
    void OnKeyboard(u_char Key);
    void OnMouse(int x, int y);
    void OnRender();
    glm::mat4 GetMatrix();
    const glm::vec3 &GetPos() { return m_pos; }
    const glm::vec3 &GetTarget() { return m_target; }

private:
    void Init();
    void Update();

    glm::vec3 m_pos;
    glm::vec3 m_target;
    glm::vec3 m_up;

    float m_speed = 0.1f;

    int m_windowWidth;
    int m_windowHeight;

    float m_AngleH;
    float m_AngleV;

    bool m_OnUpperEdge;
    bool m_OnLowerEdge;
    bool m_OnLeftEdge;
    bool m_OnRightEdge;

    glm::ivec2 m_mousePos;
};
