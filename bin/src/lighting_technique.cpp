#include <glm/gtc/type_ptr.hpp>
#include "../inc/lighting_technique.h"

void DirectionalLight::CalcLocalDirection(const WorldTrans &worldTransform)
{
    LocalDirection = worldTransform.WorldDirToLocalDir(WorldDirection);
}

void PointLight::CalcLocalPosition(const WorldTrans &worldTransform)
{
    LocalPosition = worldTransform.WorldPosToLocalPos(WorldPosition);
}

void SpotLight::CalcLocalDirectionAndPosition(const WorldTrans &worldTransform)
{
    CalcLocalPosition(worldTransform);
    LocalDirection = worldTransform.WorldDirToLocalDir(WorldDirection);
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
    NumSpotLightsLocation = GetUniformLocation("gNumSpotLights");

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
        dirLightLoc.AmbientIntensity == 0xFFFFFFFF ||
        NumPointLightsLocation == INVALID_UNIFORM_LOCATION ||
        NumSpotLightsLocation == INVALID_UNIFORM_LOCATION)
    {
        return false;
    }

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

    for (u_int i = 0; i < std::size(SpotLightsLocation); i++)
    {
        char Name[128];
        memset(Name, 0, sizeof(Name));
        snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Base.Color", i);
        SpotLightsLocation[i].Color = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Base.AmbientIntensity", i);
        SpotLightsLocation[i].AmbientIntensity = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.LocalPos", i);
        SpotLightsLocation[i].Position = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gSpotLights[%d].Direction", i);
        SpotLightsLocation[i].Direction = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gSpotLights[%d].Cutoff", i);
        SpotLightsLocation[i].Cutoff = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Base.DiffuseIntensity", i);
        SpotLightsLocation[i].DiffuseIntensity = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Constant", i);
        SpotLightsLocation[i].Atten.Constant = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Linear", i);
        SpotLightsLocation[i].Atten.Linear = GetUniformLocation(Name);

        snprintf(Name, sizeof(Name), "gSpotLights[%d].Base.Atten.Exp", i);
        SpotLightsLocation[i].Atten.Exp = GetUniformLocation(Name);

        if (SpotLightsLocation[i].Color == INVALID_UNIFORM_LOCATION ||
            SpotLightsLocation[i].AmbientIntensity == INVALID_UNIFORM_LOCATION ||
            SpotLightsLocation[i].Position == INVALID_UNIFORM_LOCATION ||
            SpotLightsLocation[i].Direction == INVALID_UNIFORM_LOCATION ||
            SpotLightsLocation[i].Cutoff == INVALID_UNIFORM_LOCATION ||
            SpotLightsLocation[i].DiffuseIntensity == INVALID_UNIFORM_LOCATION ||
            SpotLightsLocation[i].Atten.Constant == INVALID_UNIFORM_LOCATION ||
            SpotLightsLocation[i].Atten.Linear == INVALID_UNIFORM_LOCATION ||
            SpotLightsLocation[i].Atten.Exp == INVALID_UNIFORM_LOCATION)
        {

            return false;
        }
    }

    for (u_int i = 0; i < std::size(m_boneLocation); i++)
    {
        char Name[128];
        memset(Name, 0, sizeof(Name));
        snprintf(Name, sizeof(Name), "gBones[%d]", i);
        m_boneLocation[i] = GetUniformLocation(Name);
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

void LightingTechnique::SetSpotLights(u_int NumLights, const SpotLight *pLights)
{
    glUniform1i(NumSpotLightsLocation, NumLights);

    for (uint i = 0; i < NumLights; i++)
    {
        glUniform3f(SpotLightsLocation[i].Color,
                    pLights[i].Color.x, pLights[i].Color.y, pLights[i].Color.z);
        glUniform1f(SpotLightsLocation[i].AmbientIntensity, pLights[i].AmbientIntensity);
        glUniform1f(SpotLightsLocation[i].DiffuseIntensity, pLights[i].DiffuseIntensity);

        const glm::vec3 &LocalPos = pLights[i].GetLocalPosition();
        glUniform3f(SpotLightsLocation[i].Position, LocalPos.x, LocalPos.y, LocalPos.z);

        glm::vec3 Direction = glm::normalize(pLights[i].GetLocalDirection());
        glUniform3f(SpotLightsLocation[i].Direction, Direction.x, Direction.y, Direction.z);

        glUniform1f(SpotLightsLocation[i].Cutoff, cosf(glm::radians(pLights[i].Cutoff)));

        glUniform1f(SpotLightsLocation[i].Atten.Constant, pLights[i].Attenuation.Constant);
        glUniform1f(SpotLightsLocation[i].Atten.Linear, pLights[i].Attenuation.Linear);
        glUniform1f(SpotLightsLocation[i].Atten.Exp, pLights[i].Attenuation.Exp);
    }
}

void LightingTechnique::SetBoneTransform(uint Index, const glm::mat4 &Transform)
{
    if (Index >= MAX_BONES)
    {
        return;
    }
    
    glUniformMatrix4fv(m_boneLocation[Index], 1, GL_FALSE, glm::value_ptr(Transform));
}
