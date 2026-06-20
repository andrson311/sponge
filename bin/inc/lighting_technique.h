#pragma once

#include <glm/glm.hpp>
#include "technique.h"
#include "basic_mesh.h"

class BaseLight
{
public:
    glm::vec3 Color = glm::vec3(1.0f);
    float AmbientIntensity = 0.0f;
    float DiffuseIntensity = 0.0f;

    bool IsZero()
    {
        return ((AmbientIntensity == 0.0f) && (DiffuseIntensity == 0.0f));
    }
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

struct PBRLight
{
    glm::vec4 PosDir;
    glm::vec3 Intensity;
};

class LightingTechnique : public Technique, public IRenderCallbacks
{
public:
    static const u_int MAX_POINT_LIGHTS = 2;
    static const u_int MAX_SPOT_LIGHTS = 2;

    static const int SUBTECH_DEFAULT = 0;
    static const int SUBTECH_PASSTHROUGH_GS = 1;
    static const int SUBTECH_WIREFRAME_ON_MESH = 2;

    LightingTechnique() {};

    virtual bool Init(int SubTech = SUBTECH_DEFAULT);

    void SetWVP(const glm::mat4 &WVP);
    void SetWorldMatrix(const glm::mat4 &WVP);
    void SetViewportMatrix(const glm::mat4 &ViewportMatrix);

    void SetTextureUnit(u_int TextureUnit);

    void SetLightWVP(const glm::mat4 &LightWVP);
    void SetShadowMapTextureUnit(u_int TextureUnit);
    void SetShadowCubeMapTextureUnit(u_int TextureUnit);
    void SetShadowMapSize(u_int Width, u_int Height);
    void SetShadowMapFilterSize(u_int Size);
    void SetShadowMapOffsetTextureUnit(u_int TextureUnit);
    void SetShadowMapOffsetTextureParams(float TextureSize, float FilterSize, float Radius);

    void SetSpecularExponentTextureUnit(u_int TextureUnit);
    void SetAlbedoTextureUnit(u_int TextureUnit);
    void SetRoughnessTextureUnit(u_int TextureUnit);
    void SetMetallicTextureUnit(u_int TextureUnit);
    void SetNormalTextureUnit(u_int TextureUnit);

    void SetDirectionalLight(const DirectionalLight &Light, bool WithDir = true);
    void UpdateDirLightDirection(const DirectionalLight &DirLight);

    void SetPointLights(u_int NumLights, const PointLight *pLights, bool WithPos = true);
    void UpdatePointLightsPos(u_int NumLights, const PointLight *pLights);

    void SetSpotLights(u_int NumLights, const SpotLight *pLights, bool WithPosAndDir = true);
    void UpdateSpotLightsPosAndDir(u_int NumLights, const SpotLight *pLights);

    void SetCameraLocalPos(const glm::vec3 &CameraLocalPos);
    void SetCameraWorldPos(const glm::vec3 &CameraWorldPos);

    virtual void SetMaterial(const Material &material);

    void SetColorMod(const glm::vec4 &ColorMod);
    void SetColorAdd(const glm::vec4 &ColorAdd);

    void ControlRimLight(bool IsEnabled);
    void ControlCellShading(bool IsEnabled);

    virtual void ControlSpecularExponent(bool IsEnabled);

    void SetLinearFog(float FogStart, float FogEnd);
    void SetExpFog(float FogEnd, float FogDensity);
    void SetExpSquaredFog(float FogEnd, float FogDensity);
    void SetLayeredFog(float FogTop, float FogEnd);
    void SetFogColor(const glm::vec3 &FogColor);
    void SetAnimatedFog(float FogEnd, float FogDensity);
    void SetFogTime(float Time);

    void SetPBR(bool IsPBR);
    void SetPBRMaterial(const PBRMaterial &Material);

    void SetClipPlane(const glm::vec3 &Normal, const glm::vec3 &PointOnPlane);

    void SetWireframeWidth(float Width);
    void SetWireframeColor(const glm::vec4 &Color);

protected:
    bool InitCommon();

private:
    void SetExpFogCommon(float FogEnd, float FogDensity);

    int m_subTech = SUBTECH_DEFAULT;

    GLuint WVPLoc = INVALID_UNIFORM_LOCATION;
    GLuint WorldMatrixLoc = INVALID_UNIFORM_LOCATION;
    GLuint ViewportMatrixLoc = INVALID_UNIFORM_LOCATION;
    GLuint samplerLoc = INVALID_UNIFORM_LOCATION;

    GLuint LightWVPLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowCubeMapLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapWidthLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapHeightLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapFilterSizeLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapOffsetTextureLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapOffsetTextureSizeLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapOffsetFilterSizeLoc = INVALID_UNIFORM_LOCATION;
    GLuint shadowMapRandomRadiusLoc = INVALID_UNIFORM_LOCATION;

    GLuint samplerSpecularExponentLoc = INVALID_UNIFORM_LOCATION;

    GLuint CameraLocalPosLoc = INVALID_UNIFORM_LOCATION;
    GLuint CameraWorldPosLoc = INVALID_UNIFORM_LOCATION;

    GLuint NumPointLightsLoc = INVALID_UNIFORM_LOCATION;
    GLuint NumSpotLightsLoc = INVALID_UNIFORM_LOCATION;

    GLuint ColorModLoc = INVALID_UNIFORM_LOCATION;
    GLuint ColorAddLoc = INVALID_UNIFORM_LOCATION;

    GLuint EnableRimLightLoc = INVALID_UNIFORM_LOCATION;
    GLuint EnableCellShadingLoc = INVALID_UNIFORM_LOCATION;
    GLuint EnableSpecularExponentLoc = INVALID_UNIFORM_LOCATION;

    GLuint FogStartLoc = INVALID_UNIFORM_LOCATION;
    GLuint FogEndLoc = INVALID_UNIFORM_LOCATION;
    GLuint FogColorLoc = INVALID_UNIFORM_LOCATION;
    GLuint ExpFogDensityLoc = INVALID_UNIFORM_LOCATION;
    GLuint ExpSquaredFogEnabledLoc = INVALID_UNIFORM_LOCATION;
    GLuint LayeredFogTopLoc = INVALID_UNIFORM_LOCATION;
    GLuint FogTimeLoc = INVALID_UNIFORM_LOCATION;

    GLuint IsPBRLoc = INVALID_UNIFORM_LOCATION;

    GLuint ClipPlaneLoc = INVALID_UNIFORM_LOCATION;

    GLuint WireframeWidthLoc = INVALID_UNIFORM_LOCATION;
    GLuint WireframeColorLoc = INVALID_UNIFORM_LOCATION;

    GLuint AlbedoLoc = INVALID_UNIFORM_LOCATION;
    GLuint RoughnessLoc = INVALID_UNIFORM_LOCATION;
    GLuint MetallicLoc = INVALID_UNIFORM_LOCATION;
    GLuint NormalMapLoc = INVALID_UNIFORM_LOCATION;

    struct
    {
        GLuint AmbientColor = INVALID_UNIFORM_LOCATION;
        GLuint DiffuseColor = INVALID_UNIFORM_LOCATION;
        GLuint SpecularColor = INVALID_UNIFORM_LOCATION;
    } materialLoc;

    struct
    {
        GLuint Color = INVALID_UNIFORM_LOCATION;
        GLuint AmbientIntensity = INVALID_UNIFORM_LOCATION;
        GLuint Direction = INVALID_UNIFORM_LOCATION;
        GLuint DiffuseIntensity = INVALID_UNIFORM_LOCATION;
    } dirLightLoc;

    struct
    {
        GLuint Color = INVALID_UNIFORM_LOCATION;
        GLuint AmbientIntensity = INVALID_UNIFORM_LOCATION;
        GLuint DiffuseIntensity = INVALID_UNIFORM_LOCATION;
        GLuint LocalPos = INVALID_UNIFORM_LOCATION;
        GLuint WorldPos = INVALID_UNIFORM_LOCATION;

        struct
        {
            GLuint Constant = INVALID_UNIFORM_LOCATION;
            GLuint Linear = INVALID_UNIFORM_LOCATION;
            GLuint Exp = INVALID_UNIFORM_LOCATION;
        } Atten;
    } PointLightsLocation[MAX_POINT_LIGHTS];

    struct
    {
        GLuint Color = INVALID_UNIFORM_LOCATION;
        GLuint AmbientIntensity = INVALID_UNIFORM_LOCATION;
        GLuint DiffuseIntensity = INVALID_UNIFORM_LOCATION;
        GLuint Position = INVALID_UNIFORM_LOCATION;
        GLuint Direction = INVALID_UNIFORM_LOCATION;
        GLuint Cutoff = INVALID_UNIFORM_LOCATION;

        struct
        {
            GLuint Constant = INVALID_UNIFORM_LOCATION;
            GLuint Linear = INVALID_UNIFORM_LOCATION;
            GLuint Exp = INVALID_UNIFORM_LOCATION;
        } Atten;
    } SpotLightsLocation[MAX_SPOT_LIGHTS];

    struct
    {
        GLuint Roughness;
        GLuint IsMetal;
        GLuint Color;
        GLuint IsAlbedo;
    } PBRMaterialLoc;

    // GLuint m_boneLocation[MAX_BONES];
};
