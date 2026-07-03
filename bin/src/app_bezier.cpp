#include "app_bezier.h"

void VertexBuffer::Init(const std::vector<float> &Vertices)
{
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
}

void VertexBuffer::Update(const std::vector<float> &Vertices)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::Render(int topology_type)
{
    if ((topology_type != GL_POINTS) && (topology_type != GL_PATCHES))
    {
        printf("Invalid topology type 0x%x\n", topology_type);
        exit(1);
    }

    glBindVertexArray(m_vao);
    glDrawArrays(topology_type, 0, 4);
}

AppBezier::~AppBezier()
{
    if (m_pGameCamera)
    {
        delete m_pGameCamera;
        m_pGameCamera = NULL;
    }
}

void AppBezier::Init()
{
    CreateWindow();
    InitCallbacks();
    InitCamera();
    InitMesh();
    InitShaders();

    glClearColor(135.0f / 255.0f, 206.0f / 255.0f, 235.0f / 255.0f, 0.0f);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CLIP_DISTANCE0);
    glPointSize(10.0f);
    glLineWidth(10.0f);
}

void AppBezier::Run()
{
    while (!glfwWindowShouldClose(window))
    {
        RenderSceneCB();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void AppBezier::RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_pGameCamera->OnRender();

    m_bezierCurveTech.Enable();
    m_bezierCurveTech.SetWVP(m_pGameCamera->GetViewProjMatrix());
    m_bezierCurveTech.SetNumSegments(m_numSegments);
    m_vertexBuffer.Render(GL_PATCHES);

    m_passThruTech.Enable();
    m_vertexBuffer.Render(GL_POINTS);
}

void AppBezier::PassiveMouseCB(int x, int y)
{
    if (!m_isPaused)
    {
        m_pGameCamera->OnMouse(x, y);
    }
}

void AppBezier::KeyboardCB(u_int key, int state)
{
    bool Handled = true;
    bool UpdateVertices = false;
    float step = 0.01f;

    if (state == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            glfwDestroyWindow(window);
            glfwTerminate();
            exit(0);

        case GLFW_KEY_P:
            m_isPaused = !m_isPaused;
            break;

        case GLFW_KEY_1:
            m_curVertex = 0;
            break;

        case GLFW_KEY_2:
            m_curVertex = 1;
            break;

        case GLFW_KEY_3:
            m_curVertex = 2;
            break;

        case GLFW_KEY_4:
            m_curVertex = 3;
            break;

        case GLFW_KEY_UP:
            m_vertices[m_curVertex * 2 + 1] += step;
            UpdateVertices = true;
            break;

        case GLFW_KEY_DOWN:
            m_vertices[m_curVertex * 2 + 1] -= step;
            UpdateVertices = true;
            break;

        case GLFW_KEY_LEFT:
            m_vertices[m_curVertex * 2] -= step;
            UpdateVertices = true;
            break;

        case GLFW_KEY_RIGHT:
            m_vertices[m_curVertex * 2] += step;
            UpdateVertices = true;
            break;

        case GLFW_KEY_A:
            m_numSegments++;
            break;

        case GLFW_KEY_Y:
            if (m_numSegments > 0)
            {
                m_numSegments--;
            }
            break;

        default:
            Handled = false;
        }
    }

    if (UpdateVertices)
    {
        m_vertexBuffer.Update(m_vertices);
    }

    if (!Handled)
    {
        m_pGameCamera->OnKeyboard(key);
    }
}

void AppBezier::CreateWindow()
{
    bool is_full_screen = false;
    window = InitGLFW(WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Bezier curve test");
    glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
}

void AppBezier::InitCallbacks()
{
    glfwSetKeyCallback(window, KeyCallback);
}

void AppBezier::InitCamera()
{
    glm::vec3 CameraPos(0.0f, 0.0f, -1.0f);
    glm::vec3 CameraTarget(0.0f, 0.0f, 1.0f);
    glm::vec3 Up(0.0f, 1.0f, 0.0f);

    float c = 3.5f;
    OrthoProjInfo orthoProjInfo;
    orthoProjInfo.l = -0.4f * c;
    orthoProjInfo.r = 0.4f * c;
    orthoProjInfo.b = -0.3f * c;
    orthoProjInfo.t = 0.3f * c;
    orthoProjInfo.n = 0.1f;
    orthoProjInfo.f = 100.0f;
    orthoProjInfo.Width = WINDOW_WIDTH;
    orthoProjInfo.Height = WINDOW_HEIGHT;

    m_pGameCamera = new Camera(orthoProjInfo, CameraPos, CameraTarget, Up);
}

void AppBezier::InitShaders()
{
    if (!m_bezierCurveTech.Init())
    {
        printf("Error initializing the bezier curve technique\n");
        exit(1);
    }

    m_bezierCurveTech.Enable();
    m_bezierCurveTech.SetNumSegments(m_numSegments);
    m_bezierCurveTech.SetLineColor(1.0f, 1.0f, 0.5f, 1.0f);

    if (!m_passThruTech.Init())
    {
        printf("Error initializing the passthru technique\n");
        exit(1);
    }

    m_passThruTech.Enable();
    m_passThruTech.SetColor(1.0f, 0.0f, 0.0f);
}

void AppBezier::InitMesh()
{
    m_vertexBuffer.Init(m_vertices);
}

AppBezier *app = NULL;

static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    app->KeyboardCB(key, action);
}
