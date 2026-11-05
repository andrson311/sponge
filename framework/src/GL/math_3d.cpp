#include <algorithm>
#include <cfloat>
#include <cmath>

#include "math_3d.h"

void CalcTightLightProjection(const glm::mat4 &CameraView,
                              const glm::vec3 &LightDir,
                              const PersProjInfo &persProjInfo,
                              glm::vec3 &LightPosWorld,
                              OrthoProjInfo &orthoProjInfo)
{
    const float nearPlane = std::max(persProjInfo.zNear, 0.001f);
    const float farPlane = std::max(persProjInfo.zFar, nearPlane + 0.001f);
    const float aspect = persProjInfo.Width / std::max(persProjInfo.Height, 1.0f);
    const float tangent = std::tan(glm::radians(persProjInfo.FOV) * 0.5f);
    const glm::mat4 inverseView = glm::inverse(CameraView);
    const glm::vec3 cameraPosition = glm::vec3(inverseView[3]);
    const glm::vec3 right = glm::normalize(glm::vec3(inverseView[0]));
    const glm::vec3 up = glm::normalize(glm::vec3(inverseView[1]));
    const glm::vec3 forward = -glm::normalize(glm::vec3(inverseView[2]));

    glm::vec3 corners[8];
    int index = 0;
    for (float distance : {nearPlane, farPlane})
    {
        const float halfHeight = tangent * distance;
        const float halfWidth = halfHeight * aspect;
        const glm::vec3 center = cameraPosition + forward * distance;
        for (float y : {-1.0f, 1.0f})
            for (float x : {-1.0f, 1.0f})
                corners[index++] = center + right * (x * halfWidth) + up * (y * halfHeight);
    }

    glm::vec3 center(0.0f);
    for (const glm::vec3 &corner : corners)
        center += corner;
    center /= 8.0f;

    glm::vec3 direction = glm::normalize(LightDir);
    if (glm::length2(direction) < 0.5f)
        direction = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 lightUp = std::abs(glm::dot(direction, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.99f
                            ? glm::vec3(0.0f, 0.0f, 1.0f)
                            : glm::vec3(0.0f, 1.0f, 0.0f);

    float radius = 0.0f;
    for (const glm::vec3 &corner : corners)
        radius = std::max(radius, glm::length(corner - center));
    radius = std::max(radius, 1.0f);
    LightPosWorld = center - direction * (radius * 2.0f);

    const glm::mat4 lightView = glm::lookAt(LightPosWorld, center, lightUp);
    glm::vec3 minimum(FLT_MAX);
    glm::vec3 maximum(-FLT_MAX);
    for (const glm::vec3 &corner : corners)
    {
        const glm::vec3 lightCorner = glm::vec3(lightView * glm::vec4(corner, 1.0f));
        minimum = glm::min(minimum, lightCorner);
        maximum = glm::max(maximum, lightCorner);
    }

    orthoProjInfo.l = minimum.x;
    orthoProjInfo.r = maximum.x;
    orthoProjInfo.b = minimum.y;
    orthoProjInfo.t = maximum.y;
    orthoProjInfo.n = minimum.z;
    orthoProjInfo.f = maximum.z;
    orthoProjInfo.Width = persProjInfo.Width;
    orthoProjInfo.Height = persProjInfo.Height;
}
