#include "math_3d.h"

// axis-aligned bounding box

void AABB::Add(const glm::vec3 &v)
{
    MinX = glm::min(MinX, v.x);
    MinY = glm::min(MinY, v.y);
    MinZ = glm::min(MinZ, v.z);

    MaxX = glm::max(MaxX, v.x);
    MaxY = glm::max(MaxY, v.y);
    MaxZ = glm::max(MaxZ, v.z);
}

void AABB::Add(const glm::vec4 &v)
{
    Add(glm::vec3(v));
}

void AABB::UpdateOrthoInfo(struct OrthoProjInfo &o)
{
    o.r = MaxX;
    o.l = MinX;
    o.b = MinY;
    o.t = MaxY;
    o.n = -MaxZ;
    o.f = -MinZ;
}

void AABB::Print()
{
    printf("X: [%f,%f]\n", MinX, MaxX);
    printf("Y: [%f,%f]\n", MinY, MaxY);
    printf("Z: [%f,%f]\n", MinZ, MaxZ);
}

// frustum

void Frustum::CalcCorners(const PersProjInfo &persProjInfo)
{
    float AR = persProjInfo.Height / persProjInfo.Width;
    float tanHalfFOV = tanf(glm::radians(persProjInfo.FOV / 2.0f));

    float NearZ = -persProjInfo.zNear;
    float NearX = persProjInfo.zNear * tanHalfFOV;
    float NearY = persProjInfo.zNear * tanHalfFOV * AR;

    NearTopLeft = glm::vec4(-NearX, NearY, NearZ, 1.0f);
    NearBottomLeft = glm::vec4(-NearX, -NearY, NearZ, 1.0f);
    NearTopRight = glm::vec4(NearX, NearY, NearZ, 1.0f);
    NearBottomRight = glm::vec4(NearX, -NearY, NearZ, 1.0f);

    float FarZ = -persProjInfo.zFar;
    float FarX = persProjInfo.zFar * tanHalfFOV;
    float FarY = persProjInfo.zFar * tanHalfFOV * AR;

    FarTopLeft = glm::vec4(-FarX, FarY, FarZ, 1.0f);
    FarBottomLeft = glm::vec4(-FarX, -FarY, FarZ, 1.0f);
    FarTopRight = glm::vec4(FarX, FarY, FarZ, 1.0f);
    FarBottomRight = glm::vec4(FarX, -FarY, FarZ, 1.0f);
}

void Frustum::Transform(const glm::mat4 &m)
{
    NearTopLeft = m * NearTopLeft;
    NearBottomLeft = m * NearBottomLeft;
    NearTopRight = m * NearTopRight;
    NearBottomRight = m * NearBottomRight;

    FarTopLeft = m * FarTopLeft;
    FarBottomLeft = m * FarBottomLeft;
    FarTopRight = m * FarTopRight;
    FarBottomRight = m * FarBottomRight;
}

void Frustum::CalcAABB(AABB &aabb)
{
    aabb.Add(NearTopLeft);
    aabb.Add(NearBottomLeft);
    aabb.Add(NearTopRight);
    aabb.Add(NearBottomRight);

    aabb.Add(FarTopLeft);
    aabb.Add(FarBottomLeft);
    aabb.Add(FarTopRight);
    aabb.Add(FarBottomRight);
}

void Frustum::Print()
{
    printf("NearTopLeft (%f, %f, %f, %f)\n", NearTopLeft.x, NearTopLeft.y, NearTopLeft.z, NearTopLeft.w);
    printf("NearBottomLeft (%f, %f, %f, %f)\n", NearBottomLeft.x, NearBottomLeft.y, NearBottomLeft.z, NearBottomLeft.w);
    printf("NearTopRight (%f, %f, %f, %f)\n", NearTopRight.x, NearTopRight.y, NearTopRight.z, NearTopRight.w);
    printf("NearBottomRight (%f, %f, %f, %f)\n", NearBottomRight.x, NearBottomRight.y, NearBottomRight.z, NearBottomRight.w);

    printf("FarTopLeft (%f, %f, %f, %f)\n", FarTopLeft.x, FarTopLeft.y, FarTopLeft.z, FarTopLeft.w);
    printf("FarBottomLeft (%f, %f, %f, %f)\n", FarBottomLeft.x, FarBottomLeft.y, FarBottomLeft.z, FarBottomLeft.w);
    printf("FarTopRight (%f, %f, %f, %f)\n", FarTopRight.x, FarTopRight.y, FarTopRight.z, FarTopRight.w);
    printf("FarBottomRight (%f, %f, %f, %f)\n", FarBottomRight.x, FarBottomRight.y, FarBottomRight.z, FarBottomRight.w);
}

void CalcTightLightProjection(const glm::mat4 &CameraView,      // in
                              const glm::vec3 &LightDir,        // in
                              const PersProjInfo &persProjInfo, // in
                              glm::vec3 &LightPosWorld,         // out
                              OrthoProjInfo &orthoProjInfo)
{
    Frustum frustum;
    frustum.CalcCorners(persProjInfo);

    glm::mat4 InverseCameraView = glm::inverse(CameraView);
    frustum.Transform(InverseCameraView);

    Frustum view_frustum_in_world_space = frustum;

    glm::vec3 Origin(0.0f);
    glm::vec3 Up(0.0f, 1.0f, 0.0f);
    glm::mat4 LightView = glm::lookAt(Origin, LightDir, Up);
    frustum.Transform(LightView);

    AABB aabb;
    frustum.CalcAABB(aabb);

    glm::vec3 BottomLeft(aabb.MinX, aabb.MinY, aabb.MinZ);
    glm::vec3 TopRight(aabb.MaxX, aabb.MaxY, aabb.MinZ);
    glm::vec4 LightPosWorld4d = glm::vec4((BottomLeft + TopRight) / 2.0f, 1.0f);

    glm::mat4 LightViewInv = glm::inverse(LightView);
    LightPosWorld = glm::vec3(LightPosWorld4d.x, LightPosWorld4d.y, LightPosWorld4d.z);

    LightView = glm::lookAt(LightPosWorld, LightPosWorld + LightDir, Up);
    view_frustum_in_world_space.Transform(LightView);

    AABB final_aabb;
    view_frustum_in_world_space.CalcAABB(final_aabb);
    final_aabb.UpdateOrthoInfo(orthoProjInfo);
}

int CalcNextPowerOfTwo(int x)
{
    int ret = 1;

    if (x == 1)
    {
        return 2;
    }

    while (ret < x)
    {
        ret = ret * 2;
    }

    return ret;
}

float RandomFloat()
{
    float Max = RAND_MAX;
    return ((float)random() / Max);
}

float RandomFloatRange(float Start, float End)
{
    if (End == Start)
    {
        return Start;
    }

    if (End < Start)
    {
        printf("Invalid random range: (%f, %f)\n", Start, End);
        exit(0);
    }

    float Delta = End - Start;
    float RandomValue = RandomFloat() * Delta + Start;

    return RandomValue;
}
