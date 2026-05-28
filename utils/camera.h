#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
    public:
        Camera();
        void SetPosition(float x, float y, float z);
        void OnKeyboard(u_char Key);
        glm::mat4 GetMatrix();
    
    private:
        glm::vec3 m_pos;
        glm::vec3 m_target;
        glm::vec3 m_up;
        float m_speed = 0.1f;
};
