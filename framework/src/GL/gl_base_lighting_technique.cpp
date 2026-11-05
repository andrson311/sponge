#include "gl_base_lighting_technique.h"

bool BaseLightingTechnique::Init()
{
    return InitUniforms({{&m_WVP, "gWVP"},
                         {&m_WorldMatrix, "gWorld"},
                         {&m_NormalMatrix, "gNormalMatrix"},
                         //{&m_HasHeightMap, "gHasHeightMap"},
                         {&m_CameraWorldPos, "gCameraWorldPos"},
                         {&m_ShadowsEnabled, "gShadowsEnabled"},
                         {&m_IsIndirectRender, "gIsIndirectRender"},
                         {&m_IsPVP, "gIsPVP"},
                         {&m_VP, "gVP"},
                         {&m_LightVP, "gLightVP"},
                         {&m_NumLights, "gNumLights"},
                         {&m_RenderMode, "gRenderMode"}});
}
