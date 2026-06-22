#include "app.h"

CameraDirection gCameraDirections[NUM_CUBE_MAP_FACES] = {
    { GL_TEXTURE_CUBE_MAP_POSITIVE_X, glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f,  1.0f,  0.0f) },
    { GL_TEXTURE_CUBE_MAP_NEGATIVE_X, glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f,  1.0f,  0.0f) },
    { GL_TEXTURE_CUBE_MAP_POSITIVE_Y, glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f) },
    { GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f) },
    { GL_TEXTURE_CUBE_MAP_POSITIVE_Z, glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f,  1.0f,  0.0f) },
    { GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f,  1.0f,  0.0f) }
};

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
    m_pointLight.AmbientIntensity = 0.1f;
    m_pointLight.DiffuseIntensity = 0.9f;
    m_pointLight.Color = glm::vec3(1.0f);
    m_pointLight.WorldPosition = glm::vec3(0.0f, 2.0f, 0.0f);

    float FOV = 90.0f;
    float zNear = 0.1f;
    float zFar = 20.0f;

    m_lightPersProjMatrix = glm::perspective(glm::radians(FOV), 1.0f, zNear, zFar);
    m_cameraOrthoProjMatrix = glm::ortho(
        -WINDOW_WIDTH / 250.0f,
        WINDOW_WIDTH / 250.0f,
        -WINDOW_HEIGHT / 250.0f,
        WINDOW_HEIGHT / 250.0f,
        1.0f,
        100.0f);

    m_housePositions[0] = glm::vec3(0.0f, 0.0f, -8.0f); // near
    m_cylinderPositions[0] = glm::vec3(0.0f, 0.0f, -4.0f);
    m_housePositions[1] = glm::vec3(-8.0f, 0.0f, 0.0f); // left
    m_cylinderPositions[1] = glm::vec3(-4.0f, 0.0f, 1.0f);
    m_housePositions[2] = glm::vec3(8.0f, 0.0f, 0.0f); // right
    m_cylinderPositions[2] = glm::vec3(4.0f, 0.0f, 1.0f);
    m_housePositions[3] = glm::vec3(0.0f, 0.0f, 8.0f); // far
    m_cylinderPositions[3] = glm::vec3(0.0f, 0.0f, 4.0f);
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
    ShadowMapPass();
    LightingPass();
}

void App::ShadowMapPass()
{
    m_shadowMapTech.Enable();
    m_shadowMapTech.SetLightWorldPos(m_pointLight.WorldPosition);

    glClearColor(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);

    for (u_int i = 0; i < NUM_CUBE_MAP_FACES; i++)
    {
        m_shadowCubeMapFBO.BindForWriting(gCameraDirections[i].CubemapFace);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        glm::mat4 LightView = glm::lookAt(m_pointLight.WorldPosition,
                                          m_pointLight.WorldPosition + gCameraDirections[i].Target,
                                          gCameraDirections[i].Up);

        for (int i = 0; i < std::size(m_housePositions); i++)
        {
            m_pMesh1->SetPosition(m_housePositions[i]);
            glm::mat4 World = m_pMesh1->GetWorldMatrix();
            glm::mat4 WVP = m_lightPersProjMatrix * LightView * World;
            m_shadowMapTech.SetWVP(WVP);
            m_shadowMapTech.SetWorld(World);
            m_pMesh1->Render();
        }

        for (int i = 0; i < std::size(m_cylinderPositions); i++)
        {
            m_pMesh2->SetPosition(m_cylinderPositions[i]);
            glm::mat4 World = m_pMesh2->GetWorldMatrix();
            glm::mat4 WVP = m_lightPersProjMatrix * LightView * World;
            m_shadowMapTech.SetWVP(WVP);
            m_shadowMapTech.SetWorld(World);
            m_pMesh2->Render();
        }
    }
}

void App::LightingPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_lightingTech.Enable();
    m_shadowCubeMapFBO.BindForReading(SHADOW_CUBE_MAP_TEXTURE_UNIT);
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

    for (int i = 0; i < std::size(m_housePositions); i++)
    {
        m_pMesh1->SetPosition(m_housePositions[i]);
        glm::mat4 World = m_pMesh1->GetWorldMatrix();
        glm::mat4 WVP = CameraProjection * CameraView * World;
        m_lightingTech.SetWorldMatrix(World);
        m_lightingTech.SetWVP(WVP);

        glm::vec3 CameraLocalPos3f = m_pMesh1->GetWorldTransform().WorldPosToLocalPos(m_pGameCamera->GetPos());
        m_lightingTech.SetCameraLocalPos(CameraLocalPos3f);
        m_pointLight.CalcLocalPosition(m_pMesh1->GetWorldTransform());
        m_lightingTech.SetPointLights(1, &m_pointLight);
        m_pMesh1->Render(&m_lightingTech);
    }

    for (int i = 0; i < std::size(m_cylinderPositions); i++)
    {
        m_pMesh2->SetPosition(m_cylinderPositions[i]);
        glm::mat4 World = m_pMesh2->GetWorldMatrix();
        glm::mat4 WVP = CameraProjection * CameraView * World;
        m_lightingTech.SetWorldMatrix(World);
        m_lightingTech.SetWVP(WVP);

        glm::vec3 CameraLocalPos3f = m_pMesh2->GetWorldTransform().WorldPosToLocalPos(m_pGameCamera->GetPos());
        m_lightingTech.SetCameraLocalPos(CameraLocalPos3f);
        m_pointLight.CalcLocalPosition(m_pMesh2->GetWorldTransform());
        m_lightingTech.SetPointLights(1, &m_pointLight);
        m_pMesh2->Render(&m_lightingTech);
    }

    // render the terrain

    glm::mat4 World = m_pTerrain->GetWorldMatrix();
    glm::mat4 WVP = CameraProjection * CameraView * World;
    m_lightingTech.SetWorldMatrix(World);
    m_lightingTech.SetWVP(WVP);

    m_pointLight.CalcLocalPosition(m_pTerrain->GetWorldTransform());
    m_lightingTech.SetPointLights(1, &m_pointLight);

    glm::vec3 CameraLocalPos3f = m_pTerrain->GetWorldTransform().WorldPosToLocalPos(m_pGameCamera->GetPos());
    m_lightingTech.SetCameraLocalPos(CameraLocalPos3f);

    m_pTerrain->Render(&m_lightingTech);
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
            m_pointLight.AmbientIntensity += ATTEN_STEP;
            break;

        case GLFW_KEY_K:
            m_pointLight.DiffuseIntensity -= ATTEN_STEP;
            break;

        case GLFW_KEY_O:
            m_isOrthoCamera = !m_isOrthoCamera;
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
    // m_shadowMapFBO.Init(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT, 3, false, true, false);
    if (!m_shadowCubeMapFBO.Init(SHADOW_MAP_SIZE))
    {
        exit(1);
    }
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
    m_lightingTech.SetShadowCubeMapTextureUnit(SHADOW_CUBE_MAP_TEXTURE_UNIT_INDEX);
    // m_lightingTech.SetShadowMapTextureUnit(SHADOW_TEXTURE_UNIT_INDEX);
    // m_lightingTech.SetShadowMapSize(SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
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
    m_pMesh1->LoadMesh("assets/house/house.obj");
    // m_pMesh1->LoadMesh("assets/example/example1.glb");
    // m_pMesh1->SetPosition(0.0f, 0.0f, 10.0f);
    // m_pMesh1->SetRotation(glm::radians(-90.0f), 0.0f, 0.0f);
    // m_pMesh1->LoadMesh("assets/ordinary_house/ordinary_house.obj");

    m_pMesh2 = new BasicMesh();
    m_pMesh2->LoadMesh("assets/cylinder/cylinder.obj");

    m_pTerrain = new BasicMesh();
    m_pTerrain->LoadMesh("assets/box_terrain/box_terrain.obj");
    m_pTerrain->SetPosition(0.0f, 0.0f, 0.0f);
}