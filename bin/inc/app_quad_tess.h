#pragma once

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#include "util.h"
#include "quad_tess_technique.h"
#include "vertex_buffer.h"
#include "app_base.h"

class AppQuadTess : public AppBase
{
public:
    AppQuadTess() {}
    ~AppQuadTess() {}

    void Init();
    virtual void RenderSceneCB(float dt) override;
    void RenderGUI();

private:
    void InitCamera();
    void InitShaders();
    void InitMesh();

    VertexBuffer m_vertexBuffer;
    QuadTessTechnique m_quadTessTech;
    std::vector<float> m_vertices = {
        -1.0f, -1.0f, // Bottom left
        1.0f, -1.0f,  // Bottom right
        1.0f, 1.0f,   // Top right
        -1.0f, 1.0f}; // Top left
    
    float m_outerLevelLeft = 4;
    float m_outerLevelBottom = 4;
    float m_outerLevelRight = 4;
    float m_outerLevelTop = 4;

    float m_innerLevelLeftRight = 4;
    float m_innerLevelTopBottom = 4;
};
