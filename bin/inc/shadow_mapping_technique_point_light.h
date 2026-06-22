#pragma once

#include "util.h"
#include "technique.h"

class ShadowMappingPointLightTechnique : public Technique
{
public:
    ShadowMappingPointLightTechnique() {};

    virtual bool Init();

    void SetWVP(const glm::mat4 &WVP);
    void SetWorld(const glm::mat4 &World);
    void SetLightWorldPos(const glm::vec3 &Pos);

private:
    GLint m_WVPLoc = INVALID_UNIFORM_LOCATION;
    GLint m_worldMatrixLoc = INVALID_UNIFORM_LOCATION;
    GLint m_lightWorldPosLoc = INVALID_UNIFORM_LOCATION;
};
