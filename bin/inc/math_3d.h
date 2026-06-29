#pragma once
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/type_ptr.hpp>

struct PersProjInfo
{
    float FOV = 0.0f;
    float Width = 0.0f;
    float Height = 0.0f;
    float zNear = 0.0f;
    float zFar = 0.0f;
};

struct OrthoProjInfo
{
    float r; // right
    float l; // left
    float b; // bottom
    float t; // top
    float n; // z near
    float f; // z far

    float Width;
    float Height;

    void Print()
    {
        printf("Left %f   Right %f\n", l, r);
        printf("Bottom %f Top %f\n", b, t);
        printf("Near %f   Far %f\n", n, f);
    }
};

void CalcTightLightProjection(const glm::mat4 &CameraView,      // in
                              const glm::vec3 &LightDir,        // in
                              const PersProjInfo &persProjInfo, // in
                              glm::vec3 &LightPosWorld,         // out
                              OrthoProjInfo &orthoProjInfo);

class AABB
{
public:
    AABB() {}

    void Add(const glm::vec3 &v);
    void Add(const glm::vec4 &v);
    void UpdateOrthoInfo(struct OrthoProjInfo &o);
    void Print();

    float MinX = FLT_MAX;
    float MaxX = -FLT_MAX;
    float MinY = FLT_MAX;
    float MaxY = -FLT_MAX;
    float MinZ = FLT_MAX;
    float MaxZ = -FLT_MAX;
};

class Frustum
{
public:
    glm::vec4 NearTopLeft;
    glm::vec4 NearBottomLeft;
    glm::vec4 NearTopRight;
    glm::vec4 NearBottomRight;

    glm::vec4 FarTopLeft;
    glm::vec4 FarBottomLeft;
    glm::vec4 FarTopRight;
    glm::vec4 FarBottomRight;

    Frustum() {};

    void CalcCorners(const PersProjInfo &persProjInfo);
    void Transform(const glm::mat4 &m);
    void CalcAABB(AABB &aabb);
    void Print();
};

class FrustumCulling
{
public:
    FrustumCulling(const glm::mat4 &ViewProj);

    void Update(const glm::mat4 &ViewProj);
    bool IsPointInsideViewFrustum(const glm::vec3 &p) const;
    bool IsAABBInsideViewFrustum(const glm::vec3 &MinCorner,
                                 const glm::vec3 &MaxCorner) const;

private:
    glm::vec4 m_leftClipPlane;
    glm::vec4 m_rightClipPlane;
    glm::vec4 m_bottomClipPlane;
    glm::vec4 m_topClipPlane;
    glm::vec4 m_nearClipPlane;
    glm::vec4 m_farClipPlane;
};

int CalcNextPowerOfTwo(int x);
float RandomFloat();
float RandomFloatRange(float Start, float End);
void CalcClipPlanes(const glm::mat4 &ViewProj,
                    glm::vec4 &l, glm::vec4 &r,
                    glm::vec4 &b, glm::vec4 &t,
                    glm::vec4 &n, glm::vec4 &f);
bool IsPointInsideViewFrustum(const glm::vec3 &p, const glm::mat4 &VP);
