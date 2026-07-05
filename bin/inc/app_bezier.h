#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#include "util.h"
#include "camera.h"
#include "glfw.h"
#include "bezier_curve_technique.h"
#include "passthru_vec2_technique.h"

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

// class VertexBuffer
// {
// public:
//     VertexBuffer() {}
//     ~VertexBuffer() {}

//     void Init(const std::vector<float> &Vertices);
//     void Update(const std::vector<float> &Vertices);
//     void Render(int topology_type);

// private:
//     GLuint m_vbo = -1;
//     GLuint m_vao = -1;
// };

class AppBezier
{
public:
    AppBezier() {}
    virtual ~AppBezier();

    void Init();
    void Run();
    void RenderSceneCB();
    void PassiveMouseCB(int x, int y);
    void KeyboardCB(u_int key, int state);

private:
    void CreateWindow();
    void InitCallbacks();
    void InitCamera();
    void InitShaders();
    void InitMesh();

    GLFWwindow *window = NULL;
    Camera *m_pGameCamera = NULL;
    bool m_isPaused = false;
    // VertexBuffer m_vertexBuffer;
    BezierCurveTechnique m_bezierCurveTech;
    PassthruVec2Technique m_passThruTech;
    std::vector<float> m_vertices = {
        -0.95f, -0.95f,
        -0.85f, 0.95f,
        0.5f, -0.95f,
        0.95f, 0.95f};
    
    int m_curVertex = 0;
    int m_numSegments = 50;
};

extern AppBezier *app;
