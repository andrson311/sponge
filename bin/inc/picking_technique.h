#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "util.h"
#include "technique.h"
#include "mesh_common.h"

class PickingTechnique : public Technique, public IRenderCallbacks
{
public:
    PickingTechnique() {};

    virtual bool Init();

    void SetWVP(const glm::mat4 &WVP);
    void SetObjectIndex(u_int ObjectIndex);
    void DrawStartCB(u_int DrawIndex);

private:
    GLuint m_WVPLocation;
    GLuint m_drawIndexLocation;
    GLuint m_objectIndexLocation;

};
