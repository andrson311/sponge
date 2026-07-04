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
    m_dirLight.WorldDirection = glm::vec3(1.0f, -1.0f, 0.0f);
    m_dirLight.AmbientIntensity = 1.2f;
    m_dirLight.DiffuseIntensity = 0.4f;
}

App::~App()
{
    if (m_pGameCamera)
    {
        delete m_pGameCamera;
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
    // InitBillboardList();

    m_startTime = GetCurrentTimeMillis();
    m_currentTime = m_startTime;
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

    m_phongRenderer.Render(m_pMesh1);
}

void App::ShadowMapPass()
{
    m_shadowMapFBO.BindForWriting();
    glClear(GL_DEPTH_BUFFER_BIT);
    m_shadowMapTech.Enable();

    glCullFace(GL_FRONT);

    glm::vec3 Origin(0.0f);
    glm::vec3 Up(0.0f, 1.0f, 0.0f);

    glm::mat4 World = m_pMesh1->GetWorldMatrix();
    glm::mat4 LightView = glm::lookAt(Origin, m_dirLight.WorldDirection, Up);
    glm::mat4 WVP = m_lightOrthoProjMatrix * LightView * World;
    m_shadowMapTech.SetWVP(WVP);
    m_pMesh1->Render();
}

void App::LightingPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_lightingTech.Enable();

    glCullFace(GL_BACK);

    m_shadowMapFBO.BindForReading(SHADOW_TEXTURE_UNIT);
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

    glm::mat4 World = m_pMesh1->GetWorldMatrix();
    glm::mat4 WVP = CameraProjection * CameraView * World;
    m_lightingTech.SetWVP(WVP);

    glm::mat4 LightWVP = m_lightOrthoProjMatrix * LightView * World;
    m_lightingTech.SetWVP(WVP);

    glm::vec3 CameraLocalPos3f = m_pMesh1->GetWorldTransform().WorldPosToLocalPos(m_pGameCamera->GetPos());
    m_lightingTech.SetCameraLocalPos(CameraLocalPos3f);
    m_dirLight.CalcLocalDirection(m_pMesh1->GetWorldTransform());
    m_lightingTech.SetDirectionalLight(m_dirLight);
    m_pMesh1->Render();

    // render the terrain

    World = m_pTerrain->GetWorldMatrix();
    WVP = CameraProjection * CameraView * World;
    m_lightingTech.SetWVP(WVP);

    LightWVP = m_lightOrthoProjMatrix * LightView * World;
    m_lightingTech.SetLightWVP(LightWVP);

    m_dirLight.CalcLocalDirection(m_pTerrain->GetWorldTransform());
    m_lightingTech.SetDirectionalLight(m_dirLight);
    m_lightingTech.SetMaterial(m_pTerrain->GetMaterial());

    CameraLocalPos3f = m_pTerrain->GetWorldTransform().WorldPosToLocalPos(m_pGameCamera->GetPos());
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

        case GLFW_KEY_L:
            m_cameraOnLight = !m_cameraOnLight;
            if (!m_cameraOnLight)
            {
                m_pGameCamera->SetPosition(m_cameraPos);
                m_pGameCamera->SetTarget(m_cameraTarget);
            }
            break;

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
        }
    }

    m_pGameCamera->OnKeyboard(key);
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
    bool ForPCF = true;

    if (!m_shadowMapFBO.Init(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, ForPCF))
    {
        exit(1);
    }

    m_pShadowMapOffsetTexture = new ShadowMapOffsetTexture(m_shadowMapOffsetTextureSize, m_shadowMapOffsetFilterSize);
    m_pShadowMapOffsetTexture->Bind(SHADOW_MAP_RANDOM_OFFSET_TEXTURE_UNIT);

    // m_shadowMapFBO.Init(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 3, false, true, false);

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
    float zFar = 1000.0f;
    PersProjInfo persProjInfo = {
        FOV,
        (float)WINDOW_WIDTH,
        (float)WINDOW_HEIGHT,
        zNear,
        zFar};

    m_pGameCamera = new Camera(persProjInfo, Pos, Target, Up);
    m_pGameCamera->SetSpeed(0.1f);
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
    // m_lightingTech.SetShadowMapTextureUnit(SHADOW_TEXTURE_UNIT_INDEX);
    // m_lightingTech.SetShadowMapSize(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    // m_lightingTech.SetShadowMapFilterSize(m_shadowMapFilterSize);
    // m_lightingTech.SetShadowMapOffsetTextureUnit(SHADOW_MAP_RANDOM_OFFSET_TEXTURE_UNIT_INDEX);
    // m_lightingTech.SetShadowMapOffsetTextureParams(
    //     (float)m_shadowMapOffsetTextureSize,
    //     (float)m_shadowMapOffsetFilterSize,
    //     m_shadowMapSampleRadius);

    //    m_lightingTech.SetSpecularExponentTextureUnit(SPECULAR_EXPONENT_UNIT_INDEX);

    if (!m_shadowMapTech.Init())
    {
        printf("Error initializing the shadow mapping technique\n");
        exit(1);
    }
}

void App::InitRenderer()
{
    m_phongRenderer.InitPhongRenderer(LightingTechnique::SUBTECH_WIREFRAME_ON_MESH);
    m_phongRenderer.SetCamera(m_pGameCamera);
    m_phongRenderer.SetDirLight(m_dirLight);
    m_phongRenderer.SetWireframeLineWidth(1.0f);
    m_phongRenderer.SetWireframeColor(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    // m_phongRenderer.SetPointLights(std::size(m_pointLights), &m_pointLights[0]);
    // m_phongRenderer.SetPBR(true);
}

void App::InitMesh()
{
    m_pMesh1 = new BasicMesh();

    m_pMesh1->LoadMesh("assets/vanguard/Vanguard.dae");
    m_pMesh1->SetRotation(glm::radians(90.0f), glm::radians(180.0f), 0.0f);
    m_pMesh1->SetPosition(0.0f, 0.0f, 5.0f);
}

void App::InitBillboardList()
{
#define NUM_ROWS 20
#define NUM_COLS 20

    std::vector<glm::vec3> Positions;
    Positions.resize(NUM_ROWS * NUM_COLS);
    glm::vec3 Base(-20.0f, 0.0f, -20.0f);

    for (u_int j = 0; j < NUM_ROWS; j++)
    {
        for (u_int i = 0; i < NUM_COLS; i++)
        {
            glm::vec3 Pos((float)i * 2.0f, 0.0f, (float)j * 2.0f);
            Positions[j * NUM_COLS + i] = Base + Pos;
        }
    }

    if (!m_billboardList.Init("assets/billboards/death-159120_1280.png", Positions))
    {
        printf("Error\n");
        exit(0);
    }
}