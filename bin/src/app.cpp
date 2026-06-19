#include "../inc/app.h"

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
    if (m_mobileCamera)
    {
        m_pGameCamera->OnRender();
    }

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

    if (m_leftMouseButton.IsPressed)
    {
        HandleMouseButtonPressed();
    }
    else
    {
        m_clicked_object_id = -1;
    }

    RenderObjects();
}

void App::HandleMouseButtonPressed()
{
    glm::mat4 View = m_pGameCamera->GetMatrix();

    if (m_leftMouseButton.FirstTime)
    {
        PickingTexture::PixelInfo Pixel = m_pickingTexture.ReadPixel(
            m_leftMouseButton.x, WINDOW_HEIGHT - m_leftMouseButton.y - 1);

        if (Pixel.ObjectID != 0)
        {
            m_clicked_object_id = Pixel.ObjectID - 1;
            assert(m_clicked_object_id < std::size(m_worldPos));
            m_objViewSpacePos = View * glm::vec4(m_worldPos[m_clicked_object_id], 1.0f);
            m_leftMouseButton.FirstTime = false;
        }
        else
        {
            m_clicked_object_id = -1;
            return;
        }
    }

    DragTheObject();
}

void App::DragTheObject()
{
    glm::mat4 Projection = m_pGameCamera->GetProjectionMat();
    glm::mat4 ProjectionInv = glm::inverse(Projection);

    float mouse_x = (float)m_leftMouseButton.x;
    float mouse_y = (float)m_leftMouseButton.y;

    float ndc_x = (2.0f * mouse_x) / WINDOW_WIDTH - 1.0f;
    float ndc_y = 1.0f - (2.0f * mouse_y) / WINDOW_HEIGHT;

    glm::vec4 ray_ndc_4d(ndc_x, ndc_y, 1.0f, 1.0f);
    glm::vec4 ray_view_4d = ProjectionInv * ray_ndc_4d;

    if (ray_view_4d.w != 0.0f)
    {
        ray_view_4d /= ray_view_4d.w;
    }

    if (ray_view_4d.z == 0.0f)
    {
        return;
    }

    float t = m_objViewSpacePos.z / ray_view_4d.z;
    glm::vec4 view_space_intersect = ray_view_4d * t;
    view_space_intersect.w = 1.0f;

    glm::mat4 View = m_pGameCamera->GetMatrix();
    glm::mat4 InvView = glm::inverse(View);
    glm::vec4 point_world = InvView * view_space_intersect;
    m_worldPos[m_clicked_object_id] = glm::vec3(point_world);
}

void App::RenderObjects()
{
    m_lightingEffect.Enable();

    WorldTrans &worldTransform = pMesh->GetWorldTransform();
    glm::mat4 View = m_pGameCamera->GetMatrix();
    glm::mat4 Projection = m_pGameCamera->GetProjectionMat();

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

        if (i == m_clicked_object_id)
        {
            m_lightingEffect.SetColorMod(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        }
        else
        {
            m_lightingEffect.SetColorMod(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
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

    case GLFW_KEY_SPACE:
        if (state == GLFW_PRESS)
        {
            m_mobileCamera = !m_mobileCamera;
        }
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
    if (m_mobileCamera)
    {
        m_pGameCamera->OnMouse(x, y);
    }
    else
    {
        m_pGameCamera->UpdateMousePosSilent(x, y);
    }

    if (m_leftMouseButton.IsPressed)
    {
        m_leftMouseButton.x = x;
        m_leftMouseButton.y = y;
    }
}

void App::MouseCB(int button, int action, int x, int y)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        m_leftMouseButton.IsPressed = (action == GLFW_PRESS);

        if (!m_leftMouseButton.IsPressed)
        {
            m_leftMouseButton.FirstTime = true;
        }
    }

    m_leftMouseButton.x = x;
    m_leftMouseButton.y = y;
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
    worldTransform.SetRotation(0.0f, -glm::radians(90.0f), 0.0f);
}
