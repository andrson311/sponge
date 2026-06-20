#pragma once

#include <glm/glm.hpp>

class CameraAPI
{
public:

    virtual const glm::vec3 GetPos() const = 0;
    virtual glm::mat4 GetViewportMatrix() const = 0;
    virtual glm::mat4 GetMatrix() const = 0;
    virtual const glm::mat4 GetProjectionMat() const = 0;
};
