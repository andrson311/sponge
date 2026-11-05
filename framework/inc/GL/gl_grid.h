#pragma once

#include <GL/glew.h>
#include <vector>
#include "model.h"
#include "math_3d.h"

class GLGrid : public Grid
{
public:
    GLGrid(int width, int depth);
    ~GLGrid();

    void Destroy();
    void Render();

private:
    struct Vertex
    {
        glm::vec3 Pos;
        glm::vec2 Tex;

        void InitVertex(int x, int z);
    };

    void CreateTriangleList(int width, int depth);
    void CreateGLState();

    void PopulateBuffers();
    void InitVertices(std::vector<Vertex> &vertices);
    void InitIndices(std::vector<u_int> &indices);

    int m_width = 0;
    int m_depth = 0;

    GLuint m_vao = 0;
    GLuint m_vb = 0;
    GLuint m_ib = 0;
};
