#pragma once

#include "technique.h"
#include "math_3d.h"
#include "world_transform.h"
#include "lighting.h"
#include "core_material.h"
#include "core_model.h"
#include "scene.h"

class BaseLightingTechnique : public Technique
{
public:
    BaseLightingTechnique() = default;

    virtual bool Init() override;

    virtual void SetMaterial(const CoreMaterial &material) = 0;

    virtual void ControlRefRefract(bool enable) {}
    virtual void SetMaterialToRefRefractFactor(float f) {}
    virtual void SetRefractETA(float f) {}

    virtual void ControlSpecularExponent(bool isEnabled) {}
    virtual void SetReflectionFactor(float f) {}
    virtual void SetFresnelPower(float f) {}

    virtual void SetColorMod(const glm::vec4 &colorMod) {}
    virtual void SetColorAdd(const glm::vec4 &colorAdd) {}

    virtual void SetLightWVP(const glm::mat4 &lightWVP) {}
    virtual void ControlNormalMap(bool enable) {}
    
    virtual void ControlCubemapping(bool enable) {}
    virtual void SetCubeMipmapLevel(int level) {}
    virtual void SetProjectionMatrix(const glm::mat4& m) {}

    void SetWVP(const glm::mat4 &WVP) { m_WVP.Set(WVP); }
    void SetWorldMatrix(const glm::mat4 &world) { m_WorldMatrix.Set(world); }
    void SetNormalMatrix(const glm::mat3 &normalMatrix) { m_NormalMatrix.Set(normalMatrix); }
    void SetLightVP(const glm::mat4 &lightVP) { m_LightVP.Set(lightVP); }
    void SetVP(const glm::mat4 &VP) { m_VP.Set(VP); }
    void SetCameraWorldPos(const glm::vec3 &cameraWorldPos) { m_CameraWorldPos.Set(cameraWorldPos); }
    void ControlParallaxMap(bool enable) { m_HasHeightMap.Set(enable); }
    void ControlShadows(bool shadowsEnabled) { m_ShadowsEnabled.Set(shadowsEnabled); }
    void ControlIndirectRender(bool isRenderIndirect) { m_IsIndirectRender.Set(isRenderIndirect); }
    void ControlPVP(bool isPVP) { m_IsPVP.Set(isPVP); }
    void SetNumLights(int numLights) { m_NumLights.Set(numLights); }
    void SetRenderMode(RENDER_MODE mode) { m_RenderMode.Set(mode); }

private:
    Uniform<glm::mat4> m_WVP;
    Uniform<glm::mat4> m_WorldMatrix;
    Uniform<glm::mat3> m_NormalMatrix;
    Uniform<glm::mat4> m_LightVP;
    Uniform<bool> m_HasHeightMap;
    Uniform<glm::vec3> m_CameraWorldPos;
    Uniform<bool> m_ShadowsEnabled;
    Uniform<bool> m_IsIndirectRender;
    Uniform<bool> m_IsPVP;
    Uniform<glm::mat4> m_VP;
    Uniform<int> m_NumLights;
    Uniform<RENDER_MODE> m_RenderMode;
};
