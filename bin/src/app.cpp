#include "app.h"

CameraDirection gCameraDirections[NUM_CUBE_MAP_FACES] = {
    {GL_TEXTURE_CUBE_MAP_POSITIVE_X, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_X, glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
    {GL_TEXTURE_CUBE_MAP_POSITIVE_Y, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)},
    {GL_TEXTURE_CUBE_MAP_POSITIVE_Z, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
    {GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)}};

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
    m_dirLight.WorldDirection = glm::vec3(1.0f, -0.5f, 0.0f);

    OrthoProjInfo cameraOrthoProjInfo;
    cameraOrthoProjInfo.l = -WINDOW_WIDTH / 250.0f;
    cameraOrthoProjInfo.r = WINDOW_WIDTH / 250.0f;
    cameraOrthoProjInfo.t = -WINDOW_HEIGHT / 250.0f;
    cameraOrthoProjInfo.b = WINDOW_HEIGHT / 250.0f;
    cameraOrthoProjInfo.n = 1.0f;
    cameraOrthoProjInfo.f = 100.0f;

    m_cameraOrthoProjMatrix = glm::ortho(
        cameraOrthoProjInfo.l,
        cameraOrthoProjInfo.r,
        cameraOrthoProjInfo.b,
        cameraOrthoProjInfo.t,
        cameraOrthoProjInfo.n,
        cameraOrthoProjInfo.f);

    m_positions[0] = glm::vec3(0.0f, 0.0f, -15.0f);
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
    // CreateShadowMap();
    InitCallbacks();
    InitCamera();
    InitMesh();
    // InitShaders();
    InitRenderer();

    m_startTime = GetCurrentTimeMillis();
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_pGameCamera->OnRender();

    long long CurrentTime = GetCurrentTimeMillis();
    float RunningTime = (float)((double)CurrentTime - (double)m_startTime) / 1000.0f;

    if (m_isAnimatedFog)
    {
        static float FogTime = 0.0f;
        float scale = 0.0000000000035f;
        FogTime += scale * ((float)random() / 10.0f) * RunningTime;
        m_phongRenderer.UpdateAnimatedFogTime(FogTime);
    }

    m_phongRenderer.Render(m_pTerrain);

    // static float foo = 0.001f;
    // foo += 0.01f;

    // if (foo >= glm::two_pi<float>())
    // {
    //     foo = 0.001f;
    // }

    // if (m_cameraOnLight)
    // {
    //     m_pGameCamera->SetPosition(m_dirLight.WorldDirection * 2.0f + glm::vec3(0.0f, 5.0f, 0.0f));
    //     m_pGameCamera->SetTarget(glm::vec3(0.0f) - m_dirLight.WorldDirection);
    // }

    // m_dirLight.WorldDirection = glm::vec3(sinf(foo), -1.0f, cosf(foo));

    // ShadowMapPass();
    // LightingPass();
}

void App::ShadowMapPass()
{
    m_shadowMapFBO.BindForWriting();
    glClear(GL_DEPTH_BUFFER_BIT);
    m_shadowMapTech.Enable();

    OrthoProjInfo LightOrthoProjInfo;
    CalcTightLightProjection(
        m_pGameCamera->GetMatrix(),
        m_dirLight.WorldDirection,
        m_pGameCamera->GetPersProjInfo(),
        m_lightWorldPos,
        LightOrthoProjInfo);

    glm::mat4 LightView;
    glm::vec3 Up(0.0f, 1.0f, 0.0f);

    m_lightOrthoProjMatrix = glm::ortho(
        LightOrthoProjInfo.l,
        LightOrthoProjInfo.r,
        LightOrthoProjInfo.b,
        LightOrthoProjInfo.t,
        LightOrthoProjInfo.n,
        LightOrthoProjInfo.f);

    for (int i = 0; i < std::size(m_positions); i++)
    {
        m_pMesh1->SetPosition(m_positions[i]);
        glm::mat4 World = m_pMesh1->GetWorldMatrix();
        glm::mat4 LightView = glm::lookAt(
            m_lightWorldPos,
            m_lightWorldPos + m_dirLight.WorldDirection,
            Up);
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

    glm::vec3 Up(0.0f, 1.0f, 0.0f);
    glm::mat4 LightView = glm::lookAt(
        m_lightWorldPos,
        m_lightWorldPos + m_dirLight.WorldDirection,
        Up);
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

        case GLFW_KEY_0:
            printf("No fog\n");
            m_isAnimatedFog = false;
            m_phongRenderer.DisableFog();
            break;

        case GLFW_KEY_1:
            printf("Linear fog\n");
            m_isAnimatedFog = false;
            m_phongRenderer.SetLinearFog(m_fogStart, m_fogEnd, m_fogColor);
            break;

        case GLFW_KEY_2:
            printf("Exponential fog\n");
            m_isAnimatedFog = false;
            m_phongRenderer.SetExpFog(m_fogEnd, m_fogColor, m_fogDensity);
            break;

        case GLFW_KEY_3:
            printf("Exponential squared fog\n");
            m_isAnimatedFog = false;
            m_phongRenderer.SetExpSquaredFog(m_fogEnd, m_fogColor, m_fogDensity);
            break;

        case GLFW_KEY_4:
            printf("Layered fog\n");
            m_isAnimatedFog = false;
            m_phongRenderer.SetLayeredFog(m_fogTop, m_fogEnd, m_fogColor);
            break;

        case GLFW_KEY_5:
            printf("Animated fog\n");
            m_phongRenderer.SetAnimatedFog(m_fogEnd, m_fogDensity, m_fogColor);
            m_isAnimatedFog = true;
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
    m_shadowMapFBO.Init(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 3, false, true, false);
    // if (!m_shadowCubeMapFBO.Init(SHADOW_MAP_SIZE))
    // {
    //     exit(1);
    // }
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
    // m_lightingTech.SetShadowCubeMapTextureUnit(SHADOW_CUBE_MAP_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetShadowMapTextureUnit(SHADOW_TEXTURE_UNIT_INDEX);
    m_lightingTech.SetShadowMapSize(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    //    m_lightingTech.SetSpecularExponentTextureUnit(SPECULAR_EXPONENT_UNIT_INDEX);

    if (!m_shadowMapTech.Init())
    {
        printf("Error initializing the shadow mapping technique\n");
        exit(1);
    }
}

void App::InitRenderer()
{
    m_phongRenderer.InitPhongRenderer();
    m_phongRenderer.SetCamera(m_pGameCamera);
    m_phongRenderer.SetDirLight(m_dirLight);
}

void App::InitMesh()
{
    // m_pMesh1 = new BasicMesh();
    // m_pMesh1 = new SkinnedMesh();
    // m_pMesh1->LoadMesh("assets/vanguard/Vanguard.dae");
    // m_pMesh1->LoadMesh("assets/house/house.obj");
    // m_pMesh1->LoadMesh("assets/example/example1.glb");
    // m_pMesh1->SetPosition(0.0f, 0.0f, 10.0f);
    // m_pMesh1->SetRotation(glm::radians(-90.0f), 0.0f, 0.0f);
    // m_pMesh1->LoadMesh("assets/ordinary_house/ordinary_house.obj");

    m_pTerrain = new BasicMesh();
    //m_pTerrain->LoadMesh("assets/box_terrain/box_terrain.obj");
    m_pTerrain->LoadMesh("assets/terrain2/terrain2.obj");
    m_pTerrain->SetPosition(0.0f, 0.0f, 0.0f);
}