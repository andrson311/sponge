#include "../inc/world_transform.h"

void WorldTrans::SetScale(float scale)
{
    m_scale = scale;
}

void WorldTrans::SetRotation(float x, float y, float z)
{
    m_rotation = glm::vec3(x, y, z);
}

void WorldTrans::SetPosition(float x, float y, float z)
{
    m_pos = glm::vec3(x, y, z);
}

void WorldTrans::Rotate(float x, float y, float z)
{
    m_rotation += glm::vec3(x, y, z);
    m_rotation = glm::mod(m_rotation, glm::two_pi<float>());
}

glm::mat4 WorldTrans::GetMatrix()
{
    glm::mat4 Scale = glm::scale(
        glm::mat4(1.0f),
        glm::vec3(m_scale));

    glm::mat4 Rotation = glm::rotate(
        glm::mat4(1.0f),
        m_rotation.x,
        glm::vec3(1.0f, 0.0f, 0.0f));

    Rotation = glm::rotate(
        Rotation,
        m_rotation.y,
        glm::vec3(0.0f, 1.0f, 0.0f));

    Rotation = glm::rotate(
        Rotation,
        m_rotation.z,
        glm::vec3(0.0f, 0.0f, 1.0f));

    glm::mat4 Translation = glm::translate(
        glm::mat4(1.0f),
        m_pos);

    return Translation * Rotation * Scale;
}

glm::mat4 WorldTrans::GetReversedTranslationMatrix()
{
    glm::mat4 ReversedTranslation = glm::translate(glm::mat4(1.0f), -m_pos);
    return ReversedTranslation;
}

glm::mat4 WorldTrans::GetReversedRotationMatrix()
{
    glm::mat4 ReversedRotation = glm::rotate(glm::mat4(1.0f), -m_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    ReversedRotation = glm::rotate(ReversedRotation, -m_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    ReversedRotation = glm::rotate(ReversedRotation, -m_rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    return ReversedRotation;
}