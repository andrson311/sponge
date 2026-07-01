#pragma once

#include "technique.h"
#include "math_3d.h"

class SkyboxTechnique : public Technique
{
public:
    SkyboxTechnique() {};

    virtual bool Init();

    void SetWVP(const glm::mat4 &WVP);
    void SetTextureUnit(u_int TextureUnit);

private:
    GLuint m_WVPLoc;
    GLuint m_textureLoc;
};