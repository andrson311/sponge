#pragma once

#include <list>
#include "camera.h"
#include "lighting.h"
#include "model.h"
#include "keys.h"

#define MAX_NUM_ROTATIONS 8

class SceneObject : public Object
{
public:
    // position
    void SetPosition(float x, float y, float z)
    {
        m_pos.x = x;
        m_pos.y = y;
        m_pos.z = z;
    }

    void SetPosition(const glm::vec3 &pos) { m_pos = pos; }
    void SetPosition(const SceneObject *pOtherObject) { m_pos = pOtherObject->GetPosition(); }
    const glm::vec3 &GetPosition() const { return m_pos; }

    void TranslateBy(float x, float y, float z)
    {
        m_pos.x += x;
        m_pos.y += y;
        m_pos.z += z;
    }
    void TranslateBy(const glm::vec3 &v)
    {
        m_pos += v;
    }

    // rotation
    void SetRotation(float x, float y, float z);
    void SetRotation(const glm::vec3 &rot);
    void PushRotation(const glm::vec3 &rot);
    void ResetRotations() { m_numRotations = 0; }
    void RotateBy(float x, float y, float z);
    void SetQuaternion(const glm::quat &q) { m_quaternion = q; }
    const glm::quat &GetQuaternion() const { return m_quaternion; }
    const glm::vec3 &GetRotation() const { return m_rotations[0]; }

    // scale
    void SetScale(float x, float y, float z)
    {
        m_scale.x = x;
        m_scale.y = y;
        m_scale.z = z;
    }
    void SetScale(const glm::vec3 &scale) { m_scale = scale; }
    float GetScale() const { return m_scale.x; }

    glm::mat4 GetMatrix() const;

    // colors
    void SetFlatColor(float r, float g, float b)
    {
        m_flatColor.r = r;
        m_flatColor.g = g;
        m_flatColor.b = b;
        m_flatColor.a = 1.0f;
    }
    void SetFlatColor(const glm::vec4 color) { m_flatColor = color; }
    const glm::vec4 &GetFlatColor() const { return m_flatColor; }

    void SetColorMod(float r, float g, float b)
    {
        m_colorMod.r = r;
        m_colorMod.g = g;
        m_colorMod.b = b;
    }
    void SetColorMod(float m) { m_colorMod = glm::vec3(m); }
    void SetColorMod(const glm::vec3 &col) { m_colorMod = col; }
    glm::vec3 GetColorMod() const { return m_colorMod; }

    glm::vec3 GetForwardDir() const;

    void SetCubeMapping(bool enable) { m_isCubemapping = enable; }
    bool IsCubeMapping() const { return m_isCubemapping; }

    void SetCubeMipmapLevel(int level) { m_cubeMipmapLevel = level; }
    int GetCubeMipmapLevel() const { return m_cubeMipmapLevel; }

protected:
    SceneObject();
    void CalcRotationStack(glm::mat4 &rot) const;

    glm::vec3 m_pos = glm::vec3(0.0f);
    glm::vec3 m_scale = glm::vec3(1.0f);

private:
    glm::vec3 m_rotations[MAX_NUM_ROTATIONS];
    int m_numRotations = 0;
    glm::vec4 m_flatColor = glm::vec4(-1.0f);
    glm::vec3 m_colorMod = glm::vec3(1.0f);
    glm::quat m_quaternion = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
    bool m_isCubemapping = false;
    int m_cubeMipmapLevel = 0;
};

struct InfiniteGridConfig
{
    bool Enabled = false;
    float Size = 100.0f;
    float CellSize = 0.025f;
    glm::vec4 ColorThin = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    glm::vec4 ColorThick = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    float MinPixelsBetweenCells = 2.0f;
    bool ShadowsEnabled = false;
};

struct SSAOParams
{
    float scale = 1.0f;
    float bias = 0.2f;
    float radius = 0.2f;
    float occScale = 25.0f;
    float attScale = 1.0f;
    float distScale = 0.5f;
};

enum TONE_MAP_METHOD
{
    TONE_MAP_METHOD_NONE = 0,
    TONE_MAP_METHOD_REINHARD = 1,
    TONE_MAP_METHOD_BRUNO_OPSENICA = 2,
    TONE_MAP_METHOD_WITH_EXPOSURE = 3,
};

enum RENDER_MODE
{
    RENDER_MODE_FULL = 0,
    RENDER_MODE_TEXTURE_ONLY = 1,
    RENDER_MODE_LIGHTING_ONLY = 2,
    RENDER_MODE_NORMALS = 3,
    RENDER_MODE_TEXCOORDS = 4,
    RENDER_MODE_BASE_COLOR = 5,
    RENDER_MODE_WHITE = 6,
    RENDER_MODE_PBR_DIFFUSE_CONTRIB = 7,
    RENDER_MODE_PBR_F = 8,
};

enum TERRAIN_RENDER_MODE
{
    TERRAIN_RENDER_MODE_FULL = 0,
    TERRAIN_RENDER_MODE_HEIGHT = 1,
    TERRAIN_RENDER_MODE_LIGHTING_ONLY = 2,
    TERRAIN_RENDER_MODE_NORMALS = 3,
    TERRAIN_RENDER_MODE_SIMPLE_TEXCOORDS = 4
};

struct ModelLoadFlags
{
    bool ConvertToLeftHanded = false;
};

class SceneConfig
{
public:
    SceneConfig();

    void ControlShadowMapping(bool EnableShadowMapping) { m_shadowMappingEnabled = EnableShadowMapping; }
    bool IsShadowMappingEnabled() const { return m_shadowMappingEnabled; }

    void ControlPicking(bool EnablePicking) { m_pickingEnabled = EnablePicking; }
    bool IsPickingEnabled() const { return m_pickingEnabled; }

    void ControlSkybox(bool EnableSkybox) { m_skyboxEnabled = EnableSkybox; }
    bool IsSkyboxEnabled() const { return m_skyboxEnabled; }

    InfiniteGridConfig &GetInfiniteGrid() { return m_infiniteGridConfig; }

    void ControlRefRefract(bool Enable) { m_refRefractEnabled = Enable; }
    bool IsRefRefractEnabled() const { return m_refRefractEnabled; }

    void SetReflectionFactor(float f) { m_reflectionFactor = std::min(1.0f, std::max(f, 0.0f)); }
    float GetReflectionFactor() const { return m_reflectionFactor; }

    void SetMatRefRefractFactor(float f) { m_matRefRefractFactor = std::min(1.0f, std::max(f, 0.0f)); }
    float GetMatRefRefractFactor() const { return m_matRefRefractFactor; }

    void SetIndexOfRefraction(float f) { m_indexOfRefraction = f; }
    float GetIndexOfRefraction() const { return m_indexOfRefraction; }

    void SetFresnelPower(float f) { m_fresnelPower = f; }
    float GetFresnelPower() const { return m_fresnelPower; }

    void SetEnvMap(int EnvMap) { m_envMap = EnvMap; }
    int GetEnvMap() const { return m_envMap; }

    void SetIrradianceMap(int IrradianceMap) { m_irradianceMap = IrradianceMap; }
    int GetIrradianceMap() const { return m_irradianceMap; }

    SSAOParams &GetSSAOParams() { return m_ssaoParams; }

    void ControlSSAO(bool Enable) { m_ssaoEnabled = Enable; }
    bool IsSSAOEnabled() const { return m_ssaoEnabled; }

    void ControlHDR(bool Enable) { m_hdrEnabled = Enable; }
    bool IsHDREnabled() const { return m_hdrEnabled; }

    void ControlSSGI(bool Enable) { m_ssgiEnabled = Enable; }
    bool IsSSGIEnabled() const { return m_ssgiEnabled; }

    void ControlBloom(bool Enable)
    {
        m_bloom.Enabled = Enable;

        if (Enable)
        {
            m_hdrEnabled = true; // bloom requires HDR
        }
    }
    bool IsBloomEnabled() const { return m_bloom.Enabled; }

    void SetHDRParams(float AverageLuminance, float Exposure)
    {
        m_hdrAverageLuminance = AverageLuminance;
        m_hdrExposure = Exposure;
    }
    void GetHDRParams(float &AverageLuminance, float &Exposure)
    {
        AverageLuminance = m_hdrAverageLuminance;
        Exposure = m_hdrExposure;
    }

    void SetToneMapMethod(TONE_MAP_METHOD Method) { m_toneMapMethod = Method; }
    TONE_MAP_METHOD GetToneMapMethod() const { return m_toneMapMethod; }

    void ControlGammaCorrection(bool Enable) { m_enableGamma = Enable; }
    bool IsGammaCorrectionEnabled() const { return m_enableGamma; }

    void SetLuminanceThreshold(float Scale) { m_bloom.LuminanceThreshold = Scale; }
    float GetLuminanceThreshold() const { return m_bloom.LuminanceThreshold; }

    void SetBloomStrength(float Strength) { m_bloom.Strength = Strength; }
    float GetBloomStrength() const { return m_bloom.Strength; }

    void SetBloomSigmaV(float SigmaV) { m_bloom.SigmaV = SigmaV; }
    float GetBloomSigmaV() const { return m_bloom.SigmaV; }

    void SetBloomSigmaH(float SigmaH) { m_bloom.SigmaH = SigmaH; }
    float GetBloomSigmaH() const { return m_bloom.SigmaH; }

    void ForcePBRDisabled(bool Disabled) { m_disablePBR = Disabled; }
    bool IsPBRDisabled() const { return m_disablePBR; }

    void SetProjectedTexture(int texture) { m_projectedTexture = texture; }
    int GetProjectedTexture() const { return m_projectedTexture; }

    void SetProjectionMatrix(const glm::mat4 &m) { m_projectionMat = m; }
    const glm::mat4 &GetProjectionMatrix() const { return m_projectionMat; }

    void SetRenderMode(RENDER_MODE mode) { m_renderMode = mode; }
    RENDER_MODE GetRenderMode() const { return m_renderMode; }

    void SetTerrainGrid(void *pGrid) { m_terrain.pGrid = pGrid; }
    void *GetTerrainGrid() const { return m_terrain.pGrid; }

    void SetTerrainHeightMap(int tex) { m_terrain.m_heightMap = tex; }
    int GetTerrainHeightMap() const { return m_terrain.m_heightMap; }

    void SetTerrainTexture(int index, int tex)
    {
        if (index >= 0 && index < std::size(m_terrain.m_textures))
        {
            m_terrain.m_textures[index] = tex;
        }
        else
        {
            printf("Error! Invalid terrain texture index %d\n", index);
            assert(0);
            exit(0);
        }
    }
    int GetTerrainTexture(int index) const
    {
        if (index >= 0 && index < std::size(m_terrain.m_textures))
        {
            return m_terrain.m_textures[index];
        }
        else
        {
            printf("Error! Invalid terrain texture index %d\n", index);
            assert(0);
            exit(0);
        }
    }

    int GetTerrainTextureCount() const { return std::size(m_terrain.m_textures); }

    void SetTerrainMaxHeight(float maxHeight) { m_terrain.m_maxHeight = maxHeight; }
    float GetTerrainMaxHeight() const { return m_terrain.m_maxHeight; }

    void SetTerrainHorizontalScale(float scale) { m_terrain.m_horizontalScale = scale; }
    float GetTerrainHorizontalScale() const { return m_terrain.m_horizontalScale; }

    void SetTerrainLowHeightPercent(float percent) { m_terrain.m_lowHeightPercent = percent; }
    float GetTerrainLowHeightPercent() const { return m_terrain.m_lowHeightPercent; }

    void SetTerrainHighHeightPercent(float percent) { m_terrain.m_highHeightPercent = percent; }
    float GetTerrainHighHeightPercent() const { return m_terrain.m_highHeightPercent; }

    void SetTerrainSunlightDir(const glm::vec3 &dir) { m_terrain.m_sunlightDir = dir; }
    const glm::vec3 &GetTerrainSunlightDir() const { return m_terrain.m_sunlightDir; }

    void SetTerrainAmbientFactor(float factor) { m_terrain.m_ambientFactor = factor; }
    float GetTerrainAmbientFactor() const { return m_terrain.m_ambientFactor; }

    void SetTerrainTexTileSizeInWorldUnits(float size) { m_terrain.m_texTileSizeInWorldUnits = size; }
    float GetTerrainTexTileSizeInWorldUnits() const { return m_terrain.m_texTileSizeInWorldUnits; }

    void SetTerrainRenderMode(TERRAIN_RENDER_MODE mode) { m_terrainRenderMode = mode; }
    TERRAIN_RENDER_MODE GetTerrainRenderMode() const { return m_terrainRenderMode; }

    Texture *pBRDF_LUT = NULL;

private:
    bool m_shadowMappingEnabled = false;
    bool m_pickingEnabled = false;
    bool m_skyboxEnabled = false;
    InfiniteGridConfig m_infiniteGridConfig;
    bool m_refRefractEnabled = false;
    float m_reflectionFactor = 0.1f;
    float m_matRefRefractFactor = 0.5f;
    float m_indexOfRefraction = 1.0f;
    float m_fresnelPower = 1.0f;
    int m_envMap = -1;
    int m_irradianceMap = -1;
    SSAOParams m_ssaoParams;
    bool m_ssaoEnabled = false;
    bool m_hdrEnabled = false;
    float m_hdrAverageLuminance = 0.0f;
    float m_hdrExposure = 0.0f;
    TONE_MAP_METHOD m_toneMapMethod = TONE_MAP_METHOD_REINHARD;
    bool m_enableGamma = false;
    bool m_ssgiEnabled = false;
    struct
    {
        bool Enabled = false;
        float Strength = 1.0f;
        float LuminanceThreshold = 1.0f;
        float SigmaV = 25.0f;
        float SigmaH = 25.0f;
    } m_bloom;
    bool m_disablePBR = false;
    int m_projectedTexture = -1;
    glm::mat4 m_projectionMat;
    RENDER_MODE m_renderMode = RENDER_MODE_FULL;
    struct
    {
        void *pGrid = NULL;
        int m_heightMap = -1;
        int m_textures[4] = {-1, -1, -1, -1};
        float m_maxHeight = 0.0f;
        float m_texTileSizeInWorldUnits = 20.0f;
        float m_horizontalScale = 1.0f;
        float m_lowHeightPercent = 0.2f;
        float m_highHeightPercent = 0.8f;
        glm::vec3 m_sunlightDir = glm::vec3(0.0f, 1.0f, 0.0f);
        float m_ambientFactor = 0.2f;
    } m_terrain;
    TERRAIN_RENDER_MODE m_terrainRenderMode = TERRAIN_RENDER_MODE_FULL;
};

struct Key
{
    u_long Pressed = 0;
    u_long Released = 0;
};

struct InputState
{
    Key Keys[KEY_LAST];
};

class Scene
{
public:
    Scene();
    ~Scene() {}

    virtual SceneObject *CreateSceneObject(Model *pModel) = 0;
    virtual SceneObject *CreateSceneObject(const std::string &BasicShape) = 0;

    virtual void AddToRenderList(SceneObject *pSceneObject) = 0;
    virtual bool RemoveFromRenderList(SceneObject *pSceneObject) = 0;
    virtual std::list<SceneObject *> GetSceneObjectsList() = 0;

    virtual void SetCamera(const glm::vec3 &Pos, const glm::vec3 &Target) = 0;
    virtual void SetCameraZRange(float near, float far) = 0;
    virtual CameraFPS *GetCurrentCamera() = 0;
    virtual void SetCameraAcceleration(float acceleration) = 0;

    virtual SceneObject *GetPickedSceneObject() const = 0;
    virtual SceneConfig *GetConfig() = 0;

    virtual void LoadSkybox(const char *pFilename) = 0;
    virtual void StartSceneGUI(const char *pTitle) = 0;
    virtual void EndSceneGUI() = 0;

    virtual const InputState& GetInputState() = 0;

    std::vector<PointLight> &GetPointLights() { return m_pointLights; }
    std::vector<SpotLight> &GetSpotLights() { return m_spotLights; }
    std::vector<DirectionalLight> &GetDirLights() { return m_dirLights; }

    void SetClearColor(const glm::vec4 &Color)
    {
        m_clearColor = Color;
        m_clearFrame = true;
    }
    void DisableClear() { m_clearFrame = false; }

protected:
    bool m_clearFrame = false;
    glm::vec4 m_clearColor;

    std::vector<PointLight> m_pointLights;
    std::vector<SpotLight> m_spotLights;
    std::vector<DirectionalLight> m_dirLights;
};
