#include "app_base.h"

static AppBase *app = NULL;

AppBase::AppBase()
{
    if (app)
    {
        printf("Base app already initialized\n");
        exit(1);
    }

    app = this;
}

AppBase::~AppBase()
{
    if (m_pGameCamera)
    {
        delete m_pGameCamera;
        m_pGameCamera = NULL;
    }
}

void AppBase::DefaultCreateWindow(int windowWidth, int windowHeight, const char *pWindowName)
{
    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;

    int major_ver = 0;
    int minor_ver = 0;
    bool is_full_screen = false;
    m_pWindow = InitGLFW(major_ver, minor_ver,
                         windowWidth, windowHeight,
                         is_full_screen, pWindowName);

    glfwSetCursorPos(m_pWindow, windowWidth / 2, windowHeight / 2);
}

void AppBase::DefaultInitGUI()
{
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    // setup gui style
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_pWindow, true);
    const char *glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void AppBase::DefaultCreateCameraPers()
{
    if ((m_windowWidth == 0) || (m_windowHeight == 0))
    {
        printf("Invalid window dimensions: width %d, height %d\n", m_windowWidth, m_windowHeight);
        exit(1);
    }

    if (m_pGameCamera)
    {
        printf("Camera already initialized\n");
        exit(1);
    }

    glm::vec3 Pos(0.0f);
    glm::vec3 Target(0.0f, 0.0f, 1.0f);
    glm::vec3 Up(0.0f, 1.0f, 0.0f);

    float FOV = 45.0f;
    float zNear = 1.0f;
    float zFar = 1000.0f;
    PersProjInfo persProjInfo = {
        FOV,
        (float)m_windowWidth,
        (float)m_windowHeight,
        zNear,
        zFar};

    m_pGameCamera = new Camera(persProjInfo, Pos, Target, Up);
}

bool AppBase::KeyboardCB(int key, int action, int mods)
{
    bool Handled = true;

    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            glfwDestroyWindow(m_pWindow);
            glfwTerminate();
            exit(0);

        case GLFW_KEY_P:
            m_isPaused = !m_isPaused;
            break;

        case GLFW_KEY_Z:
            m_isWireframe = !m_isWireframe;

            if (m_isWireframe)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            break;

        default:
            Handled = false;
        }
    }

    if (!Handled)
    {
        m_pGameCamera->OnKeyboard(key);
    }

    return false;
}

void AppBase::MouseMoveCB(int x, int y)
{
    if (!m_isPaused)
    {
        m_pGameCamera->OnMouse(x, y);
    }
}

static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    app->KeyboardCB(key, action, mods);
}

static void CursorPosCallback(GLFWwindow *window, double x, double y)
{
    app->MouseMoveCB((int)x, (int)y);
}

static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    app->MouseButtonCB(button, action, mods, (int)x, (int)y);
}

void AppBase::DefaultInitCallbacks()
{
    glfwSetKeyCallback(m_pWindow, KeyCallback);
    glfwSetCursorPosCallback(m_pWindow, CursorPosCallback);
    glfwSetMouseButtonCallback(m_pWindow, MouseButtonCallback);
}

void AppBase::SetWindowShouldClose()
{
    glfwSetWindowShouldClose(m_pWindow, GLFW_TRUE);
}

void AppBase::Run()
{
    float current_time = (float)glfwGetTime();

    while (!glfwWindowShouldClose(m_pWindow))
    {
        float time = (float)glfwGetTime();
        float dt = time - current_time;

        RenderSceneCB(dt);
        current_time = time;
        glfwSwapBuffers(m_pWindow);
        glfwPollEvents();
    }
}
