#include <glm/gtc/type_ptr.hpp>
#include "lighting_technique.h"

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
    lightColorLoc = GetUniformLocation("gLight.Color");
    lightAmbientIntensityLoc = GetUniformLocation("gLight.AmbientIntensity");
    materialAmbientColorLoc = GetUniformLocation("gMaterial.AmbientColor");

    if (WVPLoc == -1 ||
        sampleLoc == -1 ||
        lightColorLoc == -1 ||
        lightAmbientIntensityLoc == -1 ||
        materialAmbientColorLoc == -1)
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

void LightingTechnique::SetLight(const BaseLight &Light)
{
    glUniform3f(lightColorLoc, Light.Color.x, Light.Color.y, Light.Color.z);
    glUniform1f(lightAmbientIntensityLoc, Light.AmbientIntensity);
}

void LightingTechnique::SetMaterial(const Material &material)
{
    glUniform3f(materialAmbientColorLoc,
                material.AmbientColor.r,
                material.AmbientColor.g,
                material.AmbientColor.b);
}
