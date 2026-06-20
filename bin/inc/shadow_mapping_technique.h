#pragma once

#include <glm/glm.hpp>
#include "technique.h"
#include "util.h"

class ShadowMappingTechnique : public Technique
{
public:

    ShadowMappingTechnique() {};

    virtual bool Init();
    void SetWVP(const glm::mat4& WVP);

private:

    GLuint m_WVPLoc = INVALID_UNIFORM_LOCATION;
};
