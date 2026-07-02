#pragma once

#include <string>
#include <vector>

#include "util.h"
#include "texture.h"
#include "billboard_technique.h"

class BillboardList
{
public:
    BillboardList();
    ~BillboardList();

    bool Init(const std::string &TexFilename, const std::vector<glm::vec3> &Positions);
    void Render(const glm::mat4 &VP, const glm::vec3 &CameraPos);

private:
    void CreatePositionBuffer(const std::vector<glm::vec3> &Positions);

    int m_numPoints = 0;
    GLuint m_vao = INVALID_OGL_VALUE;
    GLuint m_vb = INVALID_OGL_VALUE;
    Texture *m_pTexture;
    BillboardTechnique m_technique;
};
