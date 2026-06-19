#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class WorldTrans
{
public:
    WorldTrans() {}

    void SetScale(float scale);
    void SetRotation(float x, float y, float z);
    void SetPosition(float x, float y, float z);
    void SetPosition(const glm::vec3 &pos);
    void Rotate(float x, float y, float z);

    glm::mat4 GetMatrix() const;
    glm::mat4 GetReversedTranslationMatrix() const;
    glm::mat4 GetReversedRotationMatrix() const;

    glm::vec3 WorldPosToLocalPos(const glm::vec3 &WorldPos) const;
    glm::vec3 WorldDirToLocalDir(const glm::vec3 &WorldDir) const;

    float GetScale() const { return m_scale; }
    glm::vec3 GetPos() const { return m_pos; }
    glm::vec3 GetRotation() const { return m_rotation; }

private:
    float m_scale = 1.0f;
    glm::vec3 m_rotation = glm::vec3(0.0f);
    glm::vec3 m_pos = glm::vec3(0.0f);
};