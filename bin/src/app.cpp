#include "app.h"

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
    m_dirLight.AmbientIntensity = 0.5f;
    m_dirLight.DiffuseIntensity = 0.9f;
    m_dirLight.Color = glm::vec3(1.0f);
    m_dirLight.WorldDirection = glm::vec3(1.0f, -0.8f, -0.7f);

    // initialize an orthographic projection matrix for the directional light
    m_lightOrthoProjMatrix = glm::ortho(
        -20.0f,
        20.0f,
        -20.0f,
        20.0f,
        -20.0f,
        20.0f);

    // initialize an orthographic projection matrix for the camera
    m_cameraOrthoProjMatrix = glm::ortho(
        -WINDOW_WIDTH / 250.0F,
        WINDOW_WIDTH / 250.0F,
        -WINDOW_HEIGHT / 250.0F,
        WINDOW_HEIGHT / 250.0F,
        1.0f,
        100.0f);

    m_positions[0] = glm::vec3(0.0f, 0.0f, -12.0f);
    m_positions[1] = glm::vec3(0.0f);
    m_positions[2] = glm::vec3(0.0f, 0.0f, 15.0f);
}

App::~App()
{
    if (m_pGameCamera)
    {
        delete m_pGameCamera;
    }

    if (m_pMesh1)
    {
        delete m_pMesh1;
    }
}

void App::Init()
{
    CreateWindow();
    CreateShadowMap();
    InitCallbacks();
    InitCamera();
    InitMesh();
    InitShaders();
}

void App::Run()
{
    while (!glfwWindowShouldClose(window))
    {
        ProcessHeldKeys();
        RenderSceneCB();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void App::RenderSceneCB()
{
    static float foo = 0.0f;
    foo += 0.01f;

    m_dirLight.WorldDirection = glm::vec3(sinf(foo), -0.5f, cosf(foo));

    ShadowMapPass();
    LightingPass();
}

void App::ShadowMapPass()
{
    m_shadowMapFBO.BindForWriting();
    glClear(GL_DEPTH_BUFFER_BIT);
    m_shadowMapTech.Enable();

    glm::mat4 LightView;
    glm::vec3 Origin(0.0f);
    glm::vec3 Up(0.0f, 1.0f, 0.0f);

    for (int i = 0; i < std::size(m_positions); i++)
    {
        m_pMesh1->SetPosition(m_positions[i]);
        glm::mat4 World = m_pMesh1->GetWorldMatrix();
        glm::mat4 LightView = glm::lookAt(Origin, m_dirLight.WorldDirection, Up);
        glm::mat4 WVP = m_lightOrthoProjMatrix * LightView * World;
        m_shadowMapTech.SetWVP(WVP);
        m_pMesh1->Render();
    }
}

void App::LightingPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_lightingTech.Enable();
    m_shadowMapFBO.BindDepthForReading(SHADOW_TEXTURE_UNIT);
    m_pGameCamera->OnRender();

    if (m_cameraOnLight)
    {
        m_pGameCamera->SetPosition(m_dirLight.WorldDirection * 2.0f + glm::vec3(0.0f, 5.0f, 0.0f));
        m_pGameCamera->SetTarget(glm::vec3(0.0f) - m_dirLight.WorldDirection);
    }

    // render the main object

    glm::mat4 CameraView = m_pGameCamera->GetMatrix();
    glm::mat4 CameraProjection;

    if (m_isOrthoCamera)
    {
        CameraProjection = m_cameraOrthoProjMatrix;
    }
    else
    {
        CameraProjection = m_pGameCamera->GetProjectionMat();
    }

    glm::vec3 Origin(0.0f);
    glm::vec3 Up(0.0f, 1.0f, 0.0f);
    glm::mat4 LightView = glm::lookAt(Origin, m_dirLight.WorldDirection, Up);

    m_lightingTech.SetMaterial(m_pMesh1->GetMaterial());

    for (int i = 0; i < std::size(m_positions); i++)
    {
        m_pMesh1->SetPosition(m_positions[i]);
        glm::mat4 World = m_pMesh1->GetWorldMatrix();
        glm::mat4 WVP = CameraProjection * CameraView * World;
        m_lightingTech.SetWVP(WVP);

        glm::mat4 LightWVP = m_lightOrthoProjMatrix * LightView * World;
        m_lightingTech.SetLightWVP(LightWVP);

        glm::vec3 CameraLocalPos3f = m_pMesh1->GetWorldTransform().WorldPosToLocalPos(m_pGameCamera->GetPos());
        m_lightingTech.SetCameraLocalPos(CameraLocalPos3f);
        m_dirLight.CalcLocalDirection(m_pMesh1->GetWorldTransform());
        m_lightingTech.SetDirectionalLight(m_dirLight);
        m_pMesh1->Render();
    }

    // render the terrain

    glm::mat4 World = m_pTerrain->GetWorldMatrix();
    glm::mat4 WVP = CameraProjection * CameraView * World;
    m_lightingTech.SetWVP(WVP);

    glm::mat4 LightWVP = m_lightOrthoProjMatrix * LightView * World;
    m_lightingTech.SetLightWVP(LightWVP);

    m_dirLight.CalcLocalDirection(m_pTerrain->GetWorldTransform());
    m_lightingTech.SetDirectionalLight(m_dirLight);
    m_lightingTech.SetMaterial(m_pTerrain->GetMaterial());

    glm::vec3 CameraLocalPos3f = m_pTerrain->GetWorldTransform().WorldPosToLocalPos(m_pGameCamera->GetPos());
    m_lightingTech.SetCameraLocalPos(CameraLocalPos3f);

    m_pTerrain->Render();
}

#define ATTEN_STEP 0.01f
#define ANGLE_STEP 1.0f

void App::KeyBoardCB(u_int key, int state)
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
            break;

        case GLFW_KEY_L:
            m_cameraOnLight = !m_cameraOnLight;
            if (!m_cameraOnLight)
            {
                m_pGameCamera->SetPosition(m_cameraPos);
                m_pGameCamera->SetTarget(m_cameraTarget);
            }
            break;

        case GLFW_KEY_I:
            m_dirLight.AmbientIntensity += ATTEN_STEP;
            break;

        case GLFW_KEY_K:
            m_dirLight.DiffuseIntensity -= ATTEN_STEP;
            break;
        }
    }
}

void App::ProcessHeldKeys()
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

void App::PassiveMouseCB(int x, int y)
{
    m_pGameCamera->OnMouse(x, y);
}

void App::MouseCB(int button, int action, int x, int y)
{
}

void App::CreateWindow()
{
    bool is_full_screen = false;
    window = InitGLFW(WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "Test window");
    glfwSetWindowUserPointer(window, this);
    glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
}

void App::CreateShadowMap()
{
    m_shadowMapFBO.Init(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, 3, false, true, false);
}

void App::InitCallbacks()
{
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
}

void App::InitCamera()
{
    glm::vec3 Pos(0.0f, 0.0f, 0.0f);
    glm::vec3 Target(0.0f, 0.0f, 1.0f);
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
}

void App::InitShaders()
{
    if (!m_lightingTech.Init())
    {
        printf("Error initializing the lighting technique\n");
        exit(1);
    }

    m_lightingTech.Enable();
    m_lightingTech.SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetShadowMapTextureUnit(SHADOW_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetShadowMapSize(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
    //    m_lightingTech.SetSpecularExponentTextureUnit(SPECULAR_EXPONENT_UNIT_INDEX);

    if (!m_shadowMapTech.Init())
    {
        printf("Error initializing the shadow mapping technique\n");
        exit(1);
    }
}

void App::InitMesh()
{
    m_pMesh1 = new BasicMesh();
    // m_pMesh1 = new SkinnedMesh();
    // m_pMesh1->LoadMesh("assets/vanguard/Vanguard.dae");
    m_pMesh1->LoadMesh("assets/vanguard/Vanguard.dae");
    // m_pMesh1->LoadMesh("assets/example/example1.glb");
    m_pMesh1->SetPosition(0.0f, 0.0f, 10.0f);
    m_pMesh1->SetRotation(glm::radians(-90.0f), 0.0f, 0.0f);
    // m_pMesh1->LoadMesh("assets/ordinary_house/ordinary_house.obj");

    m_pTerrain = new BasicMesh();
    m_pTerrain->LoadMesh("assets/box_terrain/box_terrain.obj");
    m_pTerrain->SetPosition(0.0f, 0.0f, 0.0f);
}