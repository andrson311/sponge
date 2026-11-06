#include <stdio.h>
#include <stdlib.h>

#include "imgui.h"

#include "basic_glfw_camera.h"
#include "glfw.h"
#include "cubemap_texture.h"
#include "terrain_grid.h"
#include "gl_rendering_system.h"
#include "gl_grid.h"
#include "gl_model.h"

#define NUM_TEXTURES 1024

extern CoreRenderingSystem *g_pRenderingSystem;

static GLenum WrapModeToGLWrapMode[NUM_WRAP_MODE] = {
    GL_CLAMP_TO_EDGE,
    GL_MIRRORED_REPEAT,
    GL_REPEAT,
    GL_CLAMP_TO_BORDER};

static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    ((RenderingSystemGL *)g_pRenderingSystem)->OnKeyCallback(window, key, scancode, action, mods);
}

static void CursorPosCallback(GLFWwindow *window, double x, double y)
{
    ((RenderingSystemGL *)g_pRenderingSystem)->OnCursorPosCallback(window, x, y);
}

static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mode)
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    ((RenderingSystemGL *)g_pRenderingSystem)->OnMouseButtonCallback(window, button, action, mode);
}

RenderingSystemGL::RenderingSystemGL(GameCallbacks *pGameCallbacks, bool loadBasicShapes) : CoreRenderingSystem(pGameCallbacks, loadBasicShapes)
{
    m_textures.resize(NUM_TEXTURES, 0);
}

void RenderingSystemGL::Shutdown()
{
    if (m_pWindow)
    {
        glfwDestroyWindow(m_pWindow);
        glfwTerminate();
    }

    delete this;
}

void RenderingSystemGL::ReloadShaders()
{
    m_forwardRenderer.ReloadShaders();
}

void *RenderingSystemGL::CreateWindowInternal(const char *pWindowName)
{
    int major_ver = 4;
    int minor_ver = 6;
    bool is_full_screen = false;
    m_pWindow = InitGLFW(major_ver, minor_ver, m_windowWidth, m_windowHeight, is_full_screen, pWindowName);

    glfwSetCursorPos(m_pWindow, m_windowWidth / 2, m_windowHeight / 2);
    SetDefaultGLState();
    InitCallbacks();

    m_forwardRenderer.InitForwardRenderer(this);

    return m_pWindow;
}

Scene *RenderingSystemGL::CreateEmptyScene()
{
    return new GLScene(this);
}

CoreModel *RenderingSystemGL::LoadModelInternal(const std::string &filename, const ModelLoadFlags &flags)
{
    GLModel *pModel = new GLModel(this);

    if (!pModel->LoadAssimpModel(filename, flags))
    {
        delete pModel;
        pModel = NULL;
    }

    return pModel;
}

CoreModel *RenderingSystemGL::LoadMeshInternal(const std::string &filename)
{
    GLModel *pModel = new GLModel(this);

    if (!pModel->LoadMesh(filename))
    {
        delete pModel;
        pModel = NULL;
    }

    return pModel;
}

Grid *RenderingSystemGL::CreateGridInternal(int width, int depth)
{
    GLGrid *pGrid = new GLGrid(width, depth);
    return pGrid;
}

int RenderingSystemGL::LoadTexture2D(const std::string &filename, TextureConfig *pConfig)
{
    if (m_numTextures == m_textures.size())
    {
        printf("%s:%d: out of texture space\n", __FILE__, __LINE__);
        exit(0);
    }

    Texture *pTexture = new Texture(GL_TEXTURE_2D, filename);
    pTexture->Load();

    m_textures[m_numTextures] = pTexture;
    int ret = m_numTextures;
    m_numTextures++;

    printf("2D texture '%s' loaded, handle %d\n", filename.c_str(), ret);
    return ret;
}

int RenderingSystemGL::LoadTexture2D(const void *pData, int width, int height, TextureConfig *pConfig)
{
    if (m_numTextures == m_textures.size())
    {
        printf("%s:%d: out of texture space\n", __FILE__, __LINE__);
        exit(0);
    }

    Texture *pTexture = new Texture(GL_TEXTURE_2D);
    const int numChannels = pConfig && pConfig->m_numChannels > 0 ? pConfig->m_numChannels : 4;
    if (pConfig && pConfig->m_isFloat)
    {
        pTexture->LoadF32(width, height, static_cast<const float *>(pData));
    }
    else
    {
        pTexture->LoadRaw(width, height, numChannels, static_cast<const u_char *>(pData));
    }

    m_textures[m_numTextures] = pTexture;
    int ret = m_numTextures;
    m_numTextures++;

    printf("2D texture loaded from buffer, size %dx%d, handle %d\n", width, height, ret);

    return ret;
}

void RenderingSystemGL::UpdateTexture2D(int textureHandle, const void *pData)
{
    Texture *pTexture = (Texture *)GetTexture(textureHandle);
    pTexture->Update(pData);
}

int RenderingSystemGL::LoadCubemapTexture(const std::string &filename)
{
    if (m_numTextures == m_textures.size())
    {
        printf("%s:%d: out of texture space\n", __FILE__, __LINE__);
        exit(0);
    }

    const char *pExt = strrchr(filename.c_str(), '.');
    bool isKTX = pExt && !strcmp(pExt, ".ktx");
    BaseCubemapTexture *pTexture = NULL;

    if (isKTX)
    {
        pTexture = new CubemapTexture;
        ((CubemapTexture *)pTexture)->LoadKTX(filename);
    }
    else
    {
        pTexture = new CubemapEctTexture(filename);
        pTexture->Load();
    }

    m_textures[m_numTextures] = pTexture;
    int ret = m_numTextures;
    m_numTextures++;

    printf("Cubemap texture '%s' loaded, handle %d\n", filename.c_str(), ret);

    return ret;
}

BaseTexture *RenderingSystemGL::GetTexture(int textureHandle)
{
    if (textureHandle < 0)
    {
        printf("%s:%d: invalid texture handle %d\n", __FILE__, __LINE__, textureHandle);
        exit(0);
    }

    if (textureHandle >= m_numTextures)
    {
        printf("%s:%d: invalid texture handle %d\n", __FILE__, __LINE__, textureHandle);
        exit(0);
    }

    BaseTexture *pTexture = m_textures[textureHandle];

    if (!pTexture)
    {
        printf("%s:%d: invalid texture handle %d\n", __FILE__, __LINE__, textureHandle);
        exit(0);
    }

    return pTexture;
}

void RenderingSystemGL::SetDefaultGLState()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

void RenderingSystemGL::Execute()
{
    if (!m_pCamera)
    {
        printf("%s:%d - camera has not been set\n", __FILE__, __LINE__);
        return;
    }

    double startTime = glfwGetTime();
    double curTime = glfwGetTime();
    double prevTime = curTime;

    for (int i = 0; i < 3; i++)
    {
        curTime = glfwGetTime();
        glfwSwapBuffers(m_pWindow);
        glfwPollEvents();
    }

    while (!glfwWindowShouldClose(m_pWindow))
    {
        prevTime = curTime;
        curTime = glfwGetTime();
        double deltaTime = curTime - prevTime;
        double totalRuntime = curTime - startTime;

        m_elapsedTime = curTime - startTime;
        m_pCamera->Update((float)deltaTime);
        if (m_pScene)
        {
            m_forwardRenderer.Render(m_pWindow, (GLScene *)m_pScene, m_pGameCallbacks, totalRuntime, deltaTime);
        }
        else
        {
            printf("Warning! no scene is set in the rendering subsystem\n");
        }
        glfwSwapBuffers(m_pWindow);
        glfwPollEvents();
    }
}

void RenderingSystemGL::ImGuiTextureWindow(const char *pTitle)
{
    ImGui::Begin(pTitle, nullptr);

    const ImVec2 vMin = ImGui::GetWindowContentRegionMin();
    const ImVec2 vMax = ImGui::GetWindowContentRegionMax();

    GLuint texID = m_forwardRenderer.GetSSAOTextureHandle();

    ImGui::Image((void *)(intptr_t)texID,
                 ImVec2(vMax.x - vMin.x, vMax.y - vMin.y),
                 ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));

    ImGui::End();
}

void RenderingSystemGL::InitCallbacks()
{
    glfwSetKeyCallback(m_pWindow, KeyCallback);
    glfwSetCursorPosCallback(m_pWindow, CursorPosCallback);
    glfwSetMouseButtonCallback(m_pWindow, MouseButtonCallback);
}

void RenderingSystemGL::OnKeyCallback(GLFWwindow *pWindow, int key, int scancode, int action, int mods)
{
    if (m_pScene)
    {
        if (action != GLFW_REPEAT)
        {
            InputState &is = m_pScene->GetInputStateInternal();

            if (action == GLFW_RELEASE)
            {
                is.Keys[key].Released = is.Keys[key].Pressed;
            }
            else
            {
                is.Keys[key].Pressed++;
            }
        }
    }

    bool handledByGame = m_pGameCallbacks->OnKeyboard(key, action);
    bool handledByCamera = false;

    if (!handledByGame)
    {
        switch (key)
        {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            Shutdown();
            exit(0);

        default:
            handledByCamera = GLFWCameraHandler(m_pCamera->m_movement, key, action, mods);
        }
    }
}

void RenderingSystemGL::OnCursorPosCallback(GLFWwindow *pWindow, double x, double y)
{
    bool handledByGame = m_pGameCallbacks->OnMouseMove((int)x, (int)y);

    int windowWidth, windowHeight;
    glfwGetWindowSize(m_pWindow, &windowWidth, &windowHeight);
    m_pCamera->SetMousePosition((float)x, (float)y);
}

static bool IsMouseControlByImGUI()
{
    if (!ImGui::GetCurrentContext())
    {
        return false;
    }

    ImGuiIO &io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

void RenderingSystemGL::OnMouseButtonCallback(GLFWwindow *pWindow, int button, int action, int mode)
{
    int mousePosX = 0;
    int mousePosY = 0;
    GetMousePos(pWindow, mousePosX, mousePosY);
    bool handledByGame = m_pGameCallbacks->OnMouseButton(button, action, mode, mousePosX, mousePosY);

    if (!handledByGame && !IsMouseControlByImGUI())
    {
        m_pCamera->HandleMouseButton(button, action, mode);
    }
}

void RenderingSystemGL::GetMousePos(void *pWindow, int &x, int &y)
{
    GLFWwindow *pGLFWwindow = (GLFWwindow *)pWindow;
    double mousePosX = 0.0;
    double mousePosY = 0.0;
    glfwGetCursorPos(pGLFWwindow, &mousePosX, &mousePosY);
    x = (int)mousePosX;
    y = (int)mousePosY;
}

void *RenderingSystemGL::CreateTerrainGrid(int width, int depth)
{
    std::vector<TerrainVertex> vertices;
    std::vector<u_int> indices;

    CreateTerrainGridVectors(width, depth, vertices, indices);

    TerrainGrid *pGrid = new TerrainGrid((u_int)indices.size());

    glCreateVertexArrays(1, &pGrid->m_vao);
    glCreateBuffers(1, &pGrid->m_vbo);
    glCreateBuffers(1, &pGrid->m_ibo);

    glNamedBufferStorage(pGrid->m_vbo, vertices.size() * sizeof(TerrainVertex), vertices.data(), 0);
    glNamedBufferStorage(pGrid->m_ibo, indices.size() * sizeof(u_int), indices.data(), 0);

    glVertexArrayVertexBuffer(pGrid->m_vao, 0, pGrid->m_vbo, 0, sizeof(TerrainVertex));
    glVertexArrayElementBuffer(pGrid->m_vao, pGrid->m_ibo);

    // attribute 0: position vector (vec2)
    glEnableVertexArrayAttrib(pGrid->m_vao, 0);
    glVertexArrayAttribFormat(pGrid->m_vao, 0, 2, GL_FLOAT, GL_FALSE, offsetof(TerrainVertex, position));
    glVertexArrayAttribBinding(pGrid->m_vao, 0, 0);

    // attribute 1: texture UV coordinates (vec2)
    glEnableVertexArrayAttrib(pGrid->m_vao, 1);
    glVertexArrayAttribFormat(pGrid->m_vao, 1, 2, GL_FLOAT, GL_FALSE, offsetof(TerrainVertex, texCoords));
    glVertexArrayAttribBinding(pGrid->m_vao, 1, 0);

    return pGrid;
}

int RenderingSystemGL::GetTextureAPIHandle(int textureHandle)
{
    Texture *pTexture = (Texture *)GetTexture(textureHandle);

    GLuint ret = pTexture->GetTexture();
    return ret;
}

void TerrainGrid::Render()
{
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);
}
