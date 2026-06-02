#pragma once

#include <glm/glm.hpp>
#include "technique.h"
#include "mesh.h"

struct BaseLight
{
    glm::vec3 Color;
    float AmbientIntensity;

    BaseLight()
    {
        Color = glm::vec3(1.0f, 1.0f, 1.0f);
        AmbientIntensity = 0.0f;
    }
};

class DirectionalLight : public BaseLight
{
public:
    glm::vec3 WorldDirection = glm::vec3(0.0f);
    float DiffuseIntensity = 0.0f;

    void CalcLocalDirection(const glm::mat4 &World);
    const glm::vec3 &GetLocalDirection() const { return LocalDirection; }

private:
    glm::vec3 LocalDirection = glm::vec3(0.0f);
};

class LightingTechnique : public Technique
{
public:
    LightingTechnique() {};

    virtual bool Init();

    void SetWVP(const glm::mat4 &WVP);
    void SetTextureUnit(u_int TextureUnit);
    void SetDirectionalLight(const DirectionalLight &Light);
    void SetMaterial(const Material &material);

private:
    GLint WVPLoc;
    GLint sampleLoc;

    struct
    {
        GLuint AmbientColor;
        GLuint DiffuseColor;
    } materialLoc;

    struct 
    {
        GLuint Color;
        GLuint AmbientIntensity;
        GLuint Direction;
        GLuint DiffuseIntensity;
    } dirLightLoc;
};
