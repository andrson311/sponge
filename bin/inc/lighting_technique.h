#pragma once

#include <glm/glm.hpp>
#include "technique.h"
#include "mesh.h"

class BaseLight
{
public:
    glm::vec3 Color = glm::vec3(1.0f);
    float AmbientIntensity = 0.0f;
    float DiffuseIntensity = 0.0f;
};

class DirectionalLight : public BaseLight
{
public:
    glm::vec3 WorldDirection = glm::vec3(0.0f);
    void CalcLocalDirection(const WorldTrans &worldTransform);
    const glm::vec3 &GetLocalDirection() const { return LocalDirection; }

private:
    glm::vec3 LocalDirection = glm::vec3(0.0f);
};

struct LightAttenuation
{
    float Constant = 1.0f;
    float Linear = 0.0f;
    float Exp = 0.0f;
};

class PointLight : public BaseLight
{
public:
    glm::vec3 WorldPosition = glm::vec3(0.0f);
    LightAttenuation Attenuation;

    void CalcLocalPosition(const WorldTrans &worldTransform);
    const glm::vec3 &GetLocalPosition() const { return LocalPosition; }

private:
    glm::vec3 LocalPosition = glm::vec3(0.0f);
};

class SpotLight : public PointLight
{
public:
    glm::vec3 WorldDirection = glm::vec3(0.0f);
    float Cutoff = 0.0f;

    void CalcLocalDirectionAndPosition(const WorldTrans &worldTransform);
    const glm::vec3 &GetLocalDirection() const { return LocalDirection; }

private:
    glm::vec3 LocalDirection = glm::vec3(0.0f);
};

class LightingTechnique : public Technique
{
public:
    static const u_int MAX_POINT_LIGHTS = 2;
    static const u_int MAX_SPOT_LIGHTS = 2;

    LightingTechnique() {};

    virtual bool Init();

    void SetWVP(const glm::mat4 &WVP);
    void SetTextureUnit(u_int TextureUnit);
    void SetSpecularExponentTextureUnit(u_int TextureUnit);
    void SetDirectionalLight(const DirectionalLight &Light);
    void SetPointLights(u_int NumLights, const PointLight *pLights);
    void SetSpotLights(u_int NumLights, const SpotLight *pLights);
    void SetCameraLocalPos(const glm::vec3 &CameraLocalPos);
    void SetMaterial(const Material &material);
    void SetBoneTransform(u_int Index, const glm::mat4 &Transform);

private:
    GLint WVPLoc;
    GLint samplerLoc;
    GLuint samplerSpecularExponentLoc;
    GLuint CameraLocalPosLoc;
    GLuint NumPointLightsLocation;
    GLuint NumSpotLightsLocation;

    struct
    {
        GLuint AmbientColor;
        GLuint DiffuseColor;
        GLuint SpecularColor;
    } materialLoc;

    struct
    {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint Direction;
        GLuint DiffuseIntensity;
    } dirLightLoc;

    struct
    {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint Position;
        GLuint DiffuseIntensity;

        struct
        {
            GLuint Constant;
            GLuint Linear;
            GLuint Exp;
        } Atten;
    } PointLightsLocation[MAX_POINT_LIGHTS];

    struct
    {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint DiffuseIntensity;
        GLuint Position;
        GLuint Direction;
        GLuint Cutoff;

        struct
        {
            GLuint Constant;
            GLuint Linear;
            GLuint Exp;
        } Atten;
    } SpotLightsLocation[MAX_SPOT_LIGHTS];

    GLuint m_boneLocation[MAX_BONES];
};
