#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "math.h"

class Camera
{
public:
    Camera() {};
    Camera(int WindowWidth, int WindowHeight);
    Camera(const PersProjInfo &persProjInfo, const glm::vec3 &Pos, const glm::vec3 &Target, const glm::vec3 &Up);
    Camera(const OrthoProjInfo &orthoProjInfo, const glm::vec3 &Pos, const glm::vec3 &Target, const glm::vec3 &Up);

    void InitCamera(const PersProjInfo &persProjInfo, const glm::vec3 &Pos, const glm::vec3 &Target, const glm::vec3 &Up);
    void InitCamera(const OrthoProjInfo &orthoProjInfo, const glm::vec3 &Pos, const glm::vec3 &Target, const glm::vec3 &Up);

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

    void OnKeyboard(int key);
    void OnMouse(int x, int y);
    void UpdateMousePosSilent(int x, int y);
    void OnRender();

    glm::mat4 GetMatrix() const;

    const glm::vec3 &GetPos() { return m_pos; }
    const glm::vec3 &GetTarget() { return m_target; }
    const glm::vec3 &GetUp() { return m_up; }

    const glm::mat4 GetProjectionMat() const { return m_projection; }
    const PersProjInfo &GetPersProjInfo() const { return m_persProjInfo; }

    glm::mat4 GetViewProjMatrix() const;
    glm::mat4 GetViewMatrix() const { return GetMatrix(); }
    glm::mat4 GetViewportMatrix() const;

    void SetSpeed(float Speed);
    void SetName(const std::string &Name) { m_name = Name; }

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

    float m_AngleH = 0.0f;
    float m_AngleV = 0.0f;

    bool m_OnUpperEdge = false;
    bool m_OnLowerEdge = false;
    bool m_OnLeftEdge = false;
    bool m_OnRightEdge = false;

    glm::ivec2 m_mousePos = glm::ivec2(0);

    PersProjInfo m_persProjInfo;
    glm::mat4 m_projection;
};
