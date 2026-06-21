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
    m_spotLight.WorldPosition = glm::vec3(-20.0f, 20.0, 0.0f);
    m_spotLight.WorldDirection = glm::vec3(1.0f, -1.0f, 0.0f);
    m_spotLight.DiffuseIntensity = 0.9f;
    m_spotLight.AmbientIntensity = 0.2f;
    m_spotLight.Color = glm::vec3(1.0f);
    m_spotLight.Attenuation.Linear = 0.0f;
    m_spotLight.Attenuation.Exp = 0.0f;
    m_spotLight.Cutoff = 30.0f;

    float FOV = 45.0f;
    float zNear = 1.0f;
    float zFar = 50.0f;
    float ar = (float)SHADOW_MAP_WIDTH / (float)SHADOW_MAP_HEIGHT;
    m_lightPersProjMatrix = glm::perspective(glm::radians(FOV), ar, zNear, zFar);
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
        RenderSceneCB();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void App::RenderSceneCB()
{
    ShadowMapPass();
    LightingPass();
}

void App::ShadowMapPass()
{
    m_shadowMapFBO.BindForWriting();
    glClear(GL_DEPTH_BUFFER_BIT);
    m_shadowMapTech.Enable();

    glm::mat4 World = m_pMesh1->GetWorldMatrix();
    glm::vec3 Up(0.0f, 1.0f, 0.0f);
    glm::mat4 LightView = glm::lookAt(
        m_spotLight.WorldPosition,
        m_spotLight.WorldPosition + m_spotLight.WorldDirection,
        Up);

    glm::mat4 WVP = m_lightPersProjMatrix * LightView * World;
    m_shadowMapTech.SetWVP(WVP);

    m_pMesh1->Render();
}

void App::LightingPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_lightingTech.Enable();
    m_shadowMapFBO.BindDepthForReading(SHADOW_TEXTURE_UNIT);
    m_pGameCamera->OnRender();

    static float foo = 0.0f;
    foo += 0.002f;

    m_spotLight.WorldPosition = glm::vec3(-sinf(foo) * 15.0f, 8.0f, -cosf(foo) * 15.0f);
    m_spotLight.WorldDirection = m_pMesh1->GetPosition() - m_spotLight.WorldPosition;

    if (m_cameraOnLight)
    {
        m_pGameCamera->SetPosition(m_spotLight.WorldPosition);
        m_pGameCamera->SetTarget(m_spotLight.WorldDirection);
    }

    // render the main object

    glm::mat4 World = m_pMesh1->GetWorldMatrix();
    glm::mat4 CameraView = m_pGameCamera->GetMatrix();
    glm::mat4 CameraProjection = m_pGameCamera->GetProjectionMat();
    glm::mat4 WVP = CameraProjection * CameraView * World;
    m_lightingTech.SetWVP(WVP);

    glm::vec3 Up(0.0f, 1.0f, 0.0f);
    glm::mat4 LightView = glm::lookAt(
        m_spotLight.WorldPosition,
        m_spotLight.WorldPosition + m_spotLight.WorldDirection,
        Up);
    glm::mat4 LightWVP = m_lightPersProjMatrix * LightView * World;
    m_lightingTech.SetLightWVP(LightWVP);

    glm::vec3 CameraLocalPos3f = m_pMesh1->GetWorldTransform().WorldPosToLocalPos(m_pGameCamera->GetPos());
    m_lightingTech.SetCameraLocalPos(CameraLocalPos3f);

    m_spotLight.CalcLocalDirectionAndPosition(m_pMesh1->GetWorldTransform());
    m_lightingTech.SetSpotLights(1, &m_spotLight);
    m_lightingTech.SetMaterial(m_pMesh1->GetMaterial());
    m_pMesh1->Render();

    // render the terrain
    World = m_pTerrain->GetWorldMatrix();
    WVP = CameraProjection * CameraView * World;
    m_lightingTech.SetWVP(WVP);

    LightWVP = m_lightPersProjMatrix * LightView * World;
    m_lightingTech.SetLightWVP(LightWVP);

    m_spotLight.CalcLocalDirectionAndPosition(m_pTerrain->GetWorldTransform());
    m_lightingTech.SetSpotLights(1, &m_spotLight);
    m_lightingTech.SetMaterial(m_pTerrain->GetMaterial());

    CameraLocalPos3f = m_pTerrain->GetWorldTransform().WorldPosToLocalPos(m_pGameCamera->GetPos());
    m_lightingTech.SetCameraLocalPos(CameraLocalPos3f);

    m_pTerrain->Render();
}

#define ATTEN_STEP 0.01f
#define ANGLE_STEP 1.0f

void App::KeyBoardCB(u_int key, int state)
{
    if (state == GLFW_PRESS || state == GLFW_REPEAT)
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
            // m_directionalLight.AmbientIntensity += 0.05f;
            m_spotLight.Attenuation.Linear += ATTEN_STEP;
            m_spotLight.Attenuation.Linear += ATTEN_STEP;
            break;

        case GLFW_KEY_K:
            m_spotLight.Attenuation.Linear -= ATTEN_STEP;
            m_spotLight.Attenuation.Linear -= ATTEN_STEP;
            // m_directionalLight.AmbientIntensity -= 0.05f;
            break;

        case GLFW_KEY_O:
            m_spotLight.Attenuation.Exp += ATTEN_STEP;
            m_spotLight.Attenuation.Exp += ATTEN_STEP;
            // m_directionalLight.DiffuseIntensity += 0.05f;
            break;

        case GLFW_KEY_P:
            m_spotLight.Attenuation.Exp -= ATTEN_STEP;
            m_spotLight.Attenuation.Exp -= ATTEN_STEP;
            // m_directionalLight.DiffuseIntensity -= 0.05f;
            break;
        }
    }
    m_pGameCamera->OnKeyboard(key);
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
    // m_pMesh1->LoadMesh("assets/vanguard/Vanguard.dae");
    // m_pMesh1->LoadMesh("assets/example/example1.glb");
    // m_pMesh1->SetPosition(0.0f, 0.0f, 10.0f);
    // m_pMesh1->SetRotation(glm::radians(-90.0f), 0.0f, 0.0f);
    m_pMesh1->LoadMesh("assets/ordinary_house/ordinary_house.obj");

    m_pTerrain = new BasicMesh();
    m_pTerrain->LoadMesh("assets/box_terrain/box_terrain.obj");
    m_pTerrain->SetPosition(0.0f, 0.0f, 0.0f);
}