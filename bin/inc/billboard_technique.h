#pragma once

#include "technique.h"
#include "math_3d.h"

class BillboardTechnique : public Technique
{
public:
    BillboardTechnique() {};

    virtual bool Init();
    void SetVP(const glm::mat4 &VP);
    void SetCameraPosition(const glm::vec3 &Pos);
    void SetColorTextureUnit(u_int TextureUnit);

private:
    GLuint m_VPLocation;
    GLuint m_cameraPosLocation;
    GLuint m_colorMapLocation;
};
