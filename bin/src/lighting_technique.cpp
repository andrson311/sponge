#include <glm/gtc/type_ptr.hpp>
#include "../inc/lighting_technique.h"

void DirectionalLight::CalcLocalDirection(const glm::mat4& World) {
    glm::mat3 World3f(World);

    glm::mat3 WorldToLocal = glm::transpose(World3f);

    LocalDirection = glm::normalize(WorldToLocal * WorldDirection);
}

bool LightingTechnique::Init()
{
    if (!Technique::Init())
    {
        return false;
    }

    if (!AddShader(GL_VERTEX_SHADER, "lighting.vs"))
    {
        return false;
    }

    if (!AddShader(GL_FRAGMENT_SHADER, "lighting.fs"))
    {
        return false;
    }

    if (!Finalize())
    {
        return false;
    }

    WVPLoc = GetUniformLocation("gWVP");
    sampleLoc = GetUniformLocation("gSampler");
    materialLoc.AmbientColor = GetUniformLocation("gMaterial.AmbientColor");
    materialLoc.DiffuseColor = GetUniformLocation("gMaterial.DiffuseColor");
    dirLightLoc.Color = GetUniformLocation("gDirectionalLight.Color");
    dirLightLoc.AmbientIntensity = GetUniformLocation("gDirectionalLight.AmbientIntensity");
    dirLightLoc.Direction = GetUniformLocation("gDirectionalLight.Direction");
    dirLightLoc.DiffuseIntensity = GetUniformLocation("gDirectionalLight.DiffuseIntensity");

    if (WVPLoc == 0xFFFFFFFF ||
        sampleLoc == 0xFFFFFFFF ||
        materialLoc.AmbientColor == 0xFFFFFFFF || 
        materialLoc.DiffuseColor == 0xFFFFFFFF || 
        dirLightLoc.Color == 0xFFFFFFFF || 
        dirLightLoc.AmbientIntensity == 0xFFFFFFFF || 
        dirLightLoc.Direction == 0xFFFFFFFF || 
        dirLightLoc.DiffuseIntensity == 0xFFFFFFFF)
    {
        return false;
    }

    return true;
}

void LightingTechnique::SetWVP(const glm::mat4 &WVP)
{
    glUniformMatrix4fv(WVPLoc, 1, GL_FALSE, glm::value_ptr(WVP));
}

void LightingTechnique::SetTextureUnit(u_int TextureUnit)
{
    glUniform1i(sampleLoc, TextureUnit);
}

void LightingTechnique::SetDirectionalLight(const DirectionalLight &Light)
{
    glUniform3f(dirLightLoc.Color,            Light.Color.x, Light.Color.y, Light.Color.z);
    glUniform1f(dirLightLoc.AmbientIntensity, Light.AmbientIntensity);
    glUniform1f(dirLightLoc.DiffuseIntensity, Light.DiffuseIntensity);

    glm::vec3 LocalDir = Light.GetLocalDirection();
    glUniform3f(dirLightLoc.Direction, LocalDir.x, LocalDir.y, LocalDir.z);
}

void LightingTechnique::SetMaterial(const Material &material)
{
    glUniform3f(materialLoc.AmbientColor,
        material.AmbientColor.r, material.AmbientColor.g, material.AmbientColor.b);
    glUniform3f(materialLoc.DiffuseColor,
        material.DiffuseColor.r, material.DiffuseColor.g, material.DiffuseColor.b);
}
