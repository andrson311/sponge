#include "app_terrain.h"

static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;

    AppTerrain *app = (AppTerrain *)glfwGetWindowUserPointer(window);
    if (app)
    {
        app->KeyboardCB(key, action);
    }
}

static void CursorPosCallback(GLFWwindow *window, double x, double y)
{
    AppTerrain *app = (AppTerrain *)glfwGetWindowUserPointer(window);
    if (app)
    {
        app->PassiveMouseCB((int)x, (int)y);
    }
}

static void MouseButtonCallback(GLFWwindow *window, int Button, int Action, int Mode)
{
    (void)Mode;

    AppTerrain *app = (AppTerrain *)glfwGetWindowUserPointer(window);
    if (app)
    {
        double x = 0.0;
        double y = 0.0;
        glfwGetCursorPos(window, &x, &y);
        app->MouseCB(Button, Action, (int)x, (int)y);
    }
}

AppTerrain::~AppTerrain()
{
    if (m_pGameCamera)
    {
        delete (m_pGameCamera);
    }
}

void AppTerrain::Init()
{
    CreateWindow();
    InitCallbacks();
    InitCamera();
    InitTerrain();
}

void AppTerrain::Run()
{
    while (!glfwWindowShouldClose(window))
    {
        ProcessHeldKeys();
        RenderScene();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void AppTerrain::RenderScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_pGameCamera->OnRender();
    m_terrain.Render(*m_pGameCamera);
}

void AppTerrain::PassiveMouseCB(int x, int y)
{
    m_pGameCamera->OnMouse(x, y);
}

void AppTerrain::KeyboardCB(u_int key, int state)
{
    if (state == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            glfwDestroyWindow(window);
            glfwTerminate();
            exit(0);

        case GLFW_KEY_F:
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
        }

        m_pGameCamera->OnKeyboard(key);
    }

}

void AppTerrain::ProcessHeldKeys()
{
    if (!m_pGameCamera)
    {
        return;
    }

    const int movementKeys[] = {
        GLFW_KEY_W,
        GLFW_KEY_A,
        GLFW_KEY_S,
        GLFW_KEY_D,
        GLFW_KEY_UP,
        GLFW_KEY_DOWN,
        GLFW_KEY_LEFT,
        GLFW_KEY_RIGHT,
        GLFW_KEY_PAGE_UP,
        GLFW_KEY_PAGE_DOWN};

    for (int key : movementKeys)
    {
        if (glfwGetKey(window, key) == GLFW_PRESS)
        {
            m_pGameCamera->OnKeyboard(key);
        }
    }
}

void AppTerrain::MouseCB(int button, int action, int x, int y)
{
}

void AppTerrain::CreateWindow()
{
    bool is_full_screen = false;
    window = InitGLFW(WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Terrain rendering test");
    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
}

void AppTerrain::InitCallbacks()
{
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
}

void AppTerrain::InitCamera()
{
    glm::vec3 Pos(0.0f, 5.0f, 0.0f);
    glm::vec3 Target(0.0f, 0.0f, 1.0f);
    glm::vec3 Up(0.0f, 1.0f, 0.0f);

    float FOV = 45.0f;
    float zNear = 0.1f;
    float zFar = 2000.0f;
    PersProjInfo persProjInfo = {
        FOV,
        (float)WINDOW_WIDTH,
        (float)WINDOW_HEIGHT,
        zNear,
        zFar};

    m_pGameCamera = new Camera(persProjInfo, Pos, Target, Up);
}

void AppTerrain::InitTerrain()
{
    float WorldScale = 4.0f;
    m_terrain.InitTerrain(WorldScale);
    m_terrain.LoadFromFile("assets/heightmaps/heightmap.save");
}
