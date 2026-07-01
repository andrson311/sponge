#pragma once

#include "technique.h"
#include "math_3d.h"
#include "util.h"

class SkydomeTechnique : public Technique
{
public:
    SkydomeTechnique() {};

    virtual bool Init();

    void SetWVP(const glm::mat4 &WVP);
    // void SetRotate(const glm::mat4 &Rotate);
    void SetTextureUnit(u_int TextureUnit);

private:
    GLuint m_WVPLoc = -1;
    GLuint m_samplerLoc = -1;
};
