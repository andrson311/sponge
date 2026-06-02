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

class LightingTechnique : public Technique
{
public:
    LightingTechnique() {};

    virtual bool Init();

    void SetWVP(const glm::mat4 &WVP);
    void SetTextureUnit(u_int TextureUnit);
    void SetLight(const BaseLight &Light);
    void SetMaterial(const Material& material);

private:
    GLint WVPLoc;
    GLint sampleLoc;
    GLint lightColorLoc;
    GLint lightAmbientIntensityLoc;
    GLint materialAmbientColorLoc;
};
