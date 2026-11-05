#include "base_gl_app.h"

BaseGLApp::BaseGLApp(int windowWidth, int windowHeight, const char *pWindowName)
{
    bool loadBasicShapes = false;
    m_pRenderingSystem = RenderingSystem::CreateRenderingSystem(RENDERING_SYSTEM_GL, this, loadBasicShapes);
    m_pWindow = (GLFWwindow *)m_pRenderingSystem->CreateWindow(windowWidth, windowHeight, pWindowName);
    InitGUI();
}

void BaseGLApp::InitGUI()
{
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(m_pWindow, true);
    const char *glsl_version = "#version 130";
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void BaseGLApp::OnFrame(double deltaTime)
{
    OnFrameChild(deltaTime);
}

void BaseGLApp::OnFrameEnd()
{
    if (m_showGUI)
    {
        OnFrameGUI();
    }
}

bool BaseGLApp::OnKeyboard(int key, int action)
{
    bool handledByMe = true;

    switch (key)
    {
    case GLFW_KEY_SPACE:
        if (action == GLFW_PRESS)
        {
            m_showGUI = !m_showGUI;
        }
        break;

    case GLFW_KEY_Z:
        if (action == GLFW_PRESS)
        {
            m_isWireframe = !m_isWireframe;
            if (m_isWireframe)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
        }
        break;

    case GLFW_KEY_KP_4:
        m_pRenderingSystem->GetCurrentCamera()->SetTarget(glm::vec3(-1.0f, 0.0f, 0.0f));
        m_pRenderingSystem->GetCurrentCamera()->SetUp(glm::vec3(0.0f, 1.0f, 0.0f));
        break;

    case GLFW_KEY_KP_6:
        m_pRenderingSystem->GetCurrentCamera()->SetTarget(glm::vec3(1.0f, 0.0f, 0.0f));
        m_pRenderingSystem->GetCurrentCamera()->SetUp(glm::vec3(0.0f, 1.0f, 0.0f));
        break;

    case GLFW_KEY_KP_8:
        m_pRenderingSystem->GetCurrentCamera()->SetTarget(glm::vec3(0.0f, 0.0f, 1.0f));
        m_pRenderingSystem->GetCurrentCamera()->SetUp(glm::vec3(0.0f, 1.0f, 0.0f));
        break;

    case GLFW_KEY_KP_2:
        m_pRenderingSystem->GetCurrentCamera()->SetTarget(glm::vec3(0.0f, 0.0f, -1.0f));
        m_pRenderingSystem->GetCurrentCamera()->SetUp(glm::vec3(0.0f, 1.0f, 0.0f));
        break;

    case GLFW_KEY_R:
        m_pRenderingSystem->ReloadShaders();
        break;

    default:
        handledByMe = false;
        break;
    }

    return handledByMe;
}

bool BaseGLApp::OnMouseMove(int x, int y)
{
    return !m_leftMousePressed;
}

bool BaseGLApp::OnMouseButton(int button, int action, int mode, int x, int y)
{
    bool handledByMe = true;

    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
        m_leftMousePressed = (action == GLFW_PRESS);
        handledByMe = m_leftMousePressed && m_showGUI;
        break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
        m_midMousePressed = (action == GLFW_PRESS);
        break;

    default:
        handledByMe = false;
        break;
    }

    return handledByMe;
}

void BaseGLApp::OnFrameGUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    Scene *pScene = m_pRenderingSystem->GetScene();
    bool my_tool_active = false;

    ImGui::Begin("Test", &my_tool_active, ImGuiWindowFlags_MenuBar);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
