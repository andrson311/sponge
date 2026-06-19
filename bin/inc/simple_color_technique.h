#pragma once

#include <glm/glm.hpp>
#include "technique.h"
#include "util.h"

class SimpleColorTechnique : public Technique
{
public:
    SimpleColorTechnique() {};

    virtual bool Init();

    void SetWVP(const glm::mat4 &WVP);

private:
    GLuint m_WVPLocation;
};
