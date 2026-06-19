#include "../inc/app.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    (void)scancode;
    (void)mods;

    App *app = (App *)glfwGetWindowUserPointer(window);
    if (app)
    {
        app->KeyBoardCB(key, action);
    }
}

static void CursorPosCallback(GLFWwindow *window, double x, double y)
{
    App *app = (App *)glfwGetWindowUserPointer(window);
    if (app)
    {
        app->PassiveMouseCB((int)x, (int)y);
    }
}

static void MouseButtonCallback(GLFWwindow *window, int Button, int Action, int Mode)
{
    (void)Mode;

    App *app = (App *)glfwGetWindowUserPointer(window);
    if (app)
    {
        double x = 0.0;
        double y = 0.0;
        glfwGetCursorPos(window, &x, &y);
        app->MouseCB(Button, Action, (int)x, (int)y);
    }
}

App::App()
{
    m_directionalLight.Color = glm::vec3(1.0f);
    m_directionalLight.AmbientIntensity = 3.0f;
    m_directionalLight.DiffuseIntensity = 0.1f;
    m_directionalLight.WorldDirection = glm::vec3(-1.0f, 0.0f, 0.0f);

    m_worldPos[0] = glm::vec3(-10.0f, 0.0f, 5.0f);
    m_worldPos[1] = glm::vec3(10.0f, 0.0f, 5.0f);
    m_worldPos[2] = glm::vec3(0.0f, 2.0f, 20.0f);
}

App::~App()
{
    if (m_pGameCamera)
    {
        delete m_pGameCamera;
    }

    if (pMesh)
    {
        delete pMesh;
    }
}

void App::Init()
{
    CreateWindow();
    InitCallbacks();
    InitCamera();
    InitMesh();
    InitShaders();
}

void App::Run()
{
    while (!glfwWindowShouldClose(window))
    {
        RenderSceneCB();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void App::RenderSceneCB()
{
    m_pGameCamera->OnRender();

    if (m_leftMouseButton.IsPressed)
    {
        PickingPhase();
    }

    RenderPhase();
}

void App::PickingPhase()
{
    m_pickingTexture.EnableWriting();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_pickingEffect.Enable();

    WorldTrans &worldTransform = pMesh->GetWorldTransform();
    glm::mat4 View = m_pGameCamera->GetMatrix();
    glm::mat4 Projection = m_pGameCamera->GetProjectionMat();

    for (u_int i = 0; i < (int)std::size(m_worldPos); i++)
    {
        worldTransform.SetPosition(m_worldPos[i]);

        // Background is zero, the real objects start at 1
        m_pickingEffect.SetObjectIndex(i + 1);
        glm::mat4 World = worldTransform.GetMatrix();
        glm::mat4 WVP = Projection * View * World;
        m_pickingEffect.SetWVP(WVP);
        pMesh->Render(&m_pickingEffect);
    }

    m_pickingTexture.DisableWriting();
}

void App::RenderPhase()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    WorldTrans &worldTransform = pMesh->GetWorldTransform();
    glm::mat4 View = m_pGameCamera->GetMatrix();
    glm::mat4 Projection = m_pGameCamera->GetProjectionMat();

    int clicked_object_id = -1;
    if (m_leftMouseButton.IsPressed)
    {
        PickingTexture::PixelInfo Pixel = m_pickingTexture.ReadPixel(
            m_leftMouseButton.x,
            WINDOW_HEIGHT - m_leftMouseButton.y - 1);

        if (Pixel.ObjectID != 0)
        {
            clicked_object_id = Pixel.ObjectID - 1;
            assert(clicked_object_id < std::size(m_worldPos));
            m_simpleColorEffect.Enable();
            worldTransform.SetPosition(m_worldPos[clicked_object_id]);
            glm::mat4 World = worldTransform.GetMatrix();
            glm::mat4 WVP = Projection * View * World;
            m_simpleColorEffect.SetWVP(WVP);
            pMesh->Render(Pixel.DrawID, Pixel.PrimID);
        }
    }

    m_lightingEffect.Enable();

    for (u_int i = 0; i < std::size(m_worldPos); i++)
    {
        worldTransform.SetPosition(m_worldPos[i]);
        glm::mat4 World = worldTransform.GetMatrix();
        glm::mat4 WVP = Projection * View * World;
        m_lightingEffect.SetWVP(WVP);
        glm::vec3 CameraLocalPos3f = worldTransform.WorldPosToLocalPos(m_pGameCamera->GetPos());
        m_lightingEffect.SetCameraLocalPos(CameraLocalPos3f);
        m_directionalLight.CalcLocalDirection(worldTransform);
        m_lightingEffect.SetDirectionalLight(m_directionalLight);

        if (i == clicked_object_id)
        {
            m_lightingEffect.SetColorMod(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        }
        else
        {
            m_lightingEffect.SetColorMod(glm::vec4(1.0f));
        }

        pMesh->Render(NULL);
    }
}

void App::KeyBoardCB(u_int key, int state)
{
    switch (key)
    {
    case GLFW_KEY_ESCAPE:
    case GLFW_KEY_Q:
        glfwDestroyWindow(window);
        glfwTerminate();
        exit(0);
        break;

    case GLFW_KEY_I:
        m_directionalLight.AmbientIntensity += 0.05f;
        break;

    case GLFW_KEY_K:
        m_directionalLight.AmbientIntensity -= 0.05f;
        break;

    case GLFW_KEY_O:
        m_directionalLight.DiffuseIntensity += 0.05f;
        break;

    case GLFW_KEY_L:
        m_directionalLight.DiffuseIntensity -= 0.05f;
        break;

    default:
        m_pGameCamera->OnKeyboard(key);
    }
}

void App::PassiveMouseCB(int x, int y)
{
    m_pGameCamera->OnMouse(x, y);
}

void App::MouseCB(int button, int action, int x, int y)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        m_leftMouseButton.IsPressed = (action == GLFW_PRESS);
        m_leftMouseButton.x = x;
        m_leftMouseButton.y = y;
    }
}

void App::CreateWindow()
{
    bool is_full_screen = false;
    window = InitGLFW(WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Test window");
    glfwSetWindowUserPointer(window, this);
}

void App::InitCallbacks()
{
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
}

void App::InitCamera()
{
    glm::vec3 Pos(0.0f, 5.0f, -22.0f);
    glm::vec3 Target(0.0f, -0.2f, 1.0f);
    glm::vec3 Up(0.0f, 1.0f, 0.0f);

    float FOV = 45.0f;
    float zNear = 0.1f;
    float zFar = 100.0f;
    PersProjInfo persProjInfo = {
        FOV,
        (float)WINDOW_WIDTH,
        (float)WINDOW_HEIGHT,
        zNear,
        zFar};

    m_pGameCamera = new Camera(persProjInfo, Pos, Target, Up);

    if (!m_lightingEffect.Init())
    {
        printf("Error initializing the lighting technique.\n");
        exit(1);
    }
}

void App::InitShaders()
{
    m_lightingEffect.Enable();
    m_lightingEffect.SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    m_lightingEffect.SetSpecularExponentTextureUnit(SPECULAR_TEXTURE_UNIT_INDEX);
    m_lightingEffect.SetMaterial(pMesh->GetMaterial());

    m_pickingTexture.Init(WINDOW_WIDTH, WINDOW_HEIGHT);

    if (!m_pickingEffect.Init())
    {
        exit(1);
    }

    if (!m_simpleColorEffect.Init())
    {
        exit(1);
    }
}

void App::InitMesh()
{
    pMesh = new Mesh();
    pMesh->LoadMesh("assets/spider/spider.obj");

    WorldTrans &worldTransform = pMesh->GetWorldTransform();
    worldTransform.SetScale(0.1f);
    worldTransform.SetRotation(0.0f, -90.0f, 0.0f);
}

