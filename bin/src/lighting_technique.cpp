#include <glm/gtc/type_ptr.hpp>
#include "../inc/lighting_technique.h"

void DirectionalLight::CalcLocalDirection(const glm::mat4 &World)
{
    glm::mat3 World3f(World);

    glm::mat3 WorldToLocal = glm::transpose(World3f);

    LocalDirection = glm::normalize(WorldToLocal * WorldDirection);
}

void PointLight::CalcLocalPosition(const WorldTrans &worldTransform)
{
    LocalPosition = worldTransform.WorldPosToLocalPos(WorldPosition);
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
    samplerLoc = GetUniformLocation("gSampler");
    samplerSpecularExponentLoc = GetUniformLocation("gSamplerSpecularExponent");
    materialLoc.AmbientColor = GetUniformLocation("gMaterial.AmbientColor");
    materialLoc.DiffuseColor = GetUniformLocation("gMaterial.DiffuseColor");
    materialLoc.SpecularColor = GetUniformLocation("gMaterial.SpecularColor");
    dirLightLoc.Color = GetUniformLocation("gDirectionalLight.Base.Color");
    dirLightLoc.AmbientIntensity = GetUniformLocation("gDirectionalLight.Base.AmbientIntensity");
    dirLightLoc.Direction = GetUniformLocation("gDirectionalLight.Direction");
    dirLightLoc.DiffuseIntensity = GetUniformLocation("gDirectionalLight.Base.DiffuseIntensity");
    CameraLocalPosLoc = GetUniformLocation("gCameraLocalPos");
    NumPointLightsLocation = GetUniformLocation("gNumPointLights");

    for (u_int i = 0; i < std::size(PointLightsLocation); i++)
    {
        char Name[128];
        memset(Name, 0, sizeof(Name));
        snprintf(Name, sizeof(Name), "gPointLights[%d].Base.Color", i);
        PointLightsLocation[i].Color = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gPointLights[%d].Base.AmbientIntensity", i);
        PointLightsLocation[i].AmbientIntensity = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gPointLights[%d].LocalPos", i);
        PointLightsLocation[i].Position = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gPointLights[%d].Base.DiffuseIntensity", i);
        PointLightsLocation[i].DiffuseIntensity = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gPointLights[%d].Atten.Constant", i);
        PointLightsLocation[i].Atten.Constant = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gPointLights[%d].Atten.Linear", i);
        PointLightsLocation[i].Atten.Linear = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gPointLights[%d].Atten.Exp", i);
        PointLightsLocation[i].Atten.Exp = GetUniformLocation(Name);

        if (PointLightsLocation[i].Color == INVALID_UNIFORM_LOCATION ||
            PointLightsLocation[i].AmbientIntensity == INVALID_UNIFORM_LOCATION ||
            PointLightsLocation[i].Position == INVALID_UNIFORM_LOCATION ||
            PointLightsLocation[i].DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
            PointLightsLocation[i].Atten.Constant == INVALID_UNIFORM_LOCATION ||
            PointLightsLocation[i].Atten.Linear == INVALID_UNIFORM_LOCATION ||
            PointLightsLocation[i].Atten.Exp == INVALID_UNIFORM_LOCATION)
        {
            return false;
        }
    }

    if (WVPLoc == 0xFFFFFFFF ||
        samplerLoc == 0xFFFFFFFF ||
        samplerSpecularExponentLoc == 0xFFFFFFFF ||
        materialLoc.AmbientColor == 0xFFFFFFFF ||
        materialLoc.DiffuseColor == 0xFFFFFFFF ||
        materialLoc.SpecularColor == 0xFFFFFFFF ||
        CameraLocalPosLoc == 0xFFFFFFFF ||
        dirLightLoc.Color == 0xFFFFFFFF ||
        dirLightLoc.DiffuseIntensity == 0xFFFFFFFF ||
        dirLightLoc.Direction == 0xFFFFFFFF ||
        dirLightLoc.AmbientIntensity == 0xFFFFFFFF)
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
    glUniform1i(samplerLoc, TextureUnit);
}

void LightingTechnique::SetSpecularExponentTextureUnit(u_int TextureUnit)
{
    glUniform1i(samplerSpecularExponentLoc, TextureUnit);
}

void LightingTechnique::SetDirectionalLight(const DirectionalLight &Light)
{
    glUniform3f(dirLightLoc.Color, Light.Color.x, Light.Color.y, Light.Color.z);
    glUniform1f(dirLightLoc.AmbientIntensity, Light.AmbientIntensity);
    glUniform1f(dirLightLoc.DiffuseIntensity, Light.DiffuseIntensity);

    glm::vec3 LocalDir = Light.GetLocalDirection();
    glUniform3f(dirLightLoc.Direction, LocalDir.x, LocalDir.y, LocalDir.z);
}

void LightingTechnique::SetCameraLocalPos(const glm::vec3 &CameraLocalPos)
{
    glUniform3f(CameraLocalPosLoc, CameraLocalPos.x, CameraLocalPos.y, CameraLocalPos.z);
}

void LightingTechnique::SetMaterial(const Material &material)
{
    glUniform3f(materialLoc.AmbientColor,
                material.AmbientColor.r, material.AmbientColor.g, material.AmbientColor.b);
    glUniform3f(materialLoc.DiffuseColor,
                material.DiffuseColor.r, material.DiffuseColor.g, material.DiffuseColor.b);
    glUniform3f(materialLoc.SpecularColor,
                material.SpecularColor.r, material.SpecularColor.g, material.SpecularColor.b);
}

void LightingTechnique::SetPointLights(u_int NumLights, const PointLight *pLights)
{
    glUniform1i(NumPointLightsLocation, NumLights);

    for (u_int i = 0; i < NumLights; i++)
    {
        glUniform3f(PointLightsLocation[i].Color, pLights[i].Color.x, pLights[i].Color.y, pLights[i].Color.z);
        glUniform1f(PointLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
        glUniform1f(PointLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);
        const glm::vec3 &LocalPos = pLights[i].GetLocalPosition();
        glUniform3f(PointLightsLocation[i].Position, LocalPos.x, LocalPos.y, LocalPos.z);
        glUniform1f(PointLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
        glUniform1f(PointLightsLocation[i].Atten.Linear, pLights[i].Attenuation.Linear);
        glUniform1f(PointLightsLocation[i].Atten.Exp, pLights[i].Attenuation.Exp);
    }
}
