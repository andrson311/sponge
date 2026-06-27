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
    InitGUI();
}

void AppTerrain::Run()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ProcessHeldKeys();

        if (m_showGui)
        {
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Terrain Demo 5"); // Create a window called "Hello, world!" and append into it.

            ImGui::SliderFloat("Max height", &this->m_maxHeight, 0.0f, 1000.0f);
            ImGui::SliderFloat("Terrain roughness", &this->m_roughness, 0.0f, 5.0f);

            static float Height0 = 64.0f;
            static float Height1 = 128.0f;
            static float Height2 = 192.0f;
            static float Height3 = 256.0f;

            ImGui::SliderFloat("Height0", &Height0, 0.0f, 64.0f);
            ImGui::SliderFloat("Height1", &Height1, 64.0f, 128.0f);
            ImGui::SliderFloat("Height2", &Height2, 128.0f, 192.0f);
            ImGui::SliderFloat("Height3", &Height3, 192.0f, 256.0f);

            if (ImGui::Button("Generate"))
            {
                m_terrain.Destroy();
                srandom(getpid());
                m_terrain.CreateMidpointDisplacement(m_terrainSize, m_roughness, m_minHeight, m_maxHeight);
                m_terrain.SetTextureHeights(Height0, Height1, Height2, Height3);
            }

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();

            // Rendering
            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
        RenderScene();
        glfwSwapBuffers(window);
    }
}

void AppTerrain::RenderScene()
{
    if (!m_showGui)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    static float foo = 0.0f;
    foo += 0.002f;

    // float R = 1100.0f;
    // float S = 512.0f;

    // glm::vec3 Pos(S + cosf(foo) * R, 375.0f, S + sinf(foo) * R);
    // m_pGameCamera->SetPosition(Pos);

    // glm::vec3 Center(S, Pos.y * 0.60f, S);
    // glm::vec3 Target = Center - Pos;
    // m_pGameCamera->SetTarget(Target);
    // m_pGameCamera->SetUp(0.0f, 1.0f, 0.0f);

    m_pGameCamera->OnRender();

    float y = std::min(-0.4f, cosf(foo));
    glm::vec3 LightDir(sinf(foo * 5.0f), y, cosf(foo * 5.0f));

    m_terrain.SetLightDir(LightDir);
    m_terrain.Render(*m_pGameCamera);
}

void AppTerrain::PassiveMouseCB(int x, int y)
{
    if (!m_showGui)
    {
        m_pGameCamera->OnMouse(x, y);
    }
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

        case GLFW_KEY_P:
            m_isPaused = !m_isPaused;
            break;

        case GLFW_KEY_SPACE:
            m_showGui = !m_showGui;
            break;
        }
    }

    m_pGameCamera->OnKeyboard(key);
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
    glm::vec3 Pos(250.0f, 450.0f, -150.0f);
    glm::vec3 Target(0.0f, -0.25f, 1.0f);
    glm::vec3 Up(0.0f, 1.0f, 0.0f);

    float FOV = 45.0f;
    float zNear = 0.1f;
    float zFar = 5000.0f;
    PersProjInfo persProjInfo = {
        FOV,
        (float)WINDOW_WIDTH,
        (float)WINDOW_HEIGHT,
        zNear,
        zFar};

    m_pGameCamera = new Camera(persProjInfo, Pos, Target, Up);
    m_pGameCamera->SetSpeed(2.0f);
}

// #define USE_TEXTURE_GENERATOR

void AppTerrain::InitTerrain()
{
#ifdef USE_TEXTURE_GENERATOR
    InitTerrainTextureGenerator();
#else
    InitTerrainMultiTextures();
#endif
    m_terrain.SaveToFile("heightmap.png");
}

void AppTerrain::InitTerrainTextureGenerator()
{
    float WorldScale = 1.0f;
    float TextureScale = 10.0f;

    m_terrain.InitTerrain(WorldScale, TextureScale);

    int Size = 512;
    float Roughness = 1.0f;
    float MinHeight = 0.0f;
    float MaxHeight = 156.0f;

    m_terrain.CreateMidpointDisplacement(Size, Roughness, MinHeight, MaxHeight);

    TextureGenerator TexGen;

    TexGen.LoadTile("assets/terrain_textures/rock02_2.jpg");
    // TexGen.LoadTile("assets/terrain_textures/IMGP5487_seamless.jpg");
    // TexGen.LoadTile("assets/terrain_textures/IMGP5525_seamless.jpg");
    TexGen.LoadTile("assets/terrain_textures/rock01.jpg");

    TexGen.LoadTile("assets/terrain_textures/tilable-IMG_0044-verydark.png");

    // TexGen.LoadTile("assets/terrain_textures/grass1.jpg");
    // TexGen.LoadTile("assets/terrain_textures/Rock6.png");

    TexGen.LoadTile("assets/terrain_textures/water.png");
    int TextureSize = 1024;

    Texture *pTexture = TexGen.GenerateTexture(TextureSize, &m_terrain, MinHeight, MaxHeight);
    m_terrain.SetTexture(pTexture);
}

void AppTerrain::InitTerrainMultiTextures()
{
    float WorldScale = 2.0f;

    float TextureScale = 4.0f;

    std::vector<std::string> TextureFilenames;
    TextureFilenames.push_back("assets/terrain_textures/IMGP5525_seamless.jpg");
    TextureFilenames.push_back("assets/terrain_textures/IMGP5487_seamless.jpg");
    TextureFilenames.push_back("assets/terrain_textures/tilable-IMG_0044-verydark.png");
    TextureFilenames.push_back("assets/terrain_textures/water.png");

    m_terrain.InitTerrain(WorldScale, TextureScale, TextureFilenames);
    m_terrain.CreateMidpointDisplacement(m_terrainSize, m_roughness, m_minHeight, m_maxHeight);
    
    glm::vec3 LightDir(1.0f, -1.0f, 0.0f);
    m_terrain.SetLightDir(LightDir);
}

void AppTerrain::InitGUI()
{
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    const char *glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);
}
