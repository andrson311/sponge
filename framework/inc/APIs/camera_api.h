#pragma once

#include "math_3d.h"

class CameraAPI
{
public:
    virtual const glm::vec3 GetPosition() const = 0;
    virtual glm::mat4 GetViewportMatrix() const = 0;
    virtual glm::mat4 GetMatrix() const = 0;
    virtual const glm::mat4 GetProjectionMatrix() const = 0;
};
