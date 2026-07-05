#include "app_quad_tess.h"

void AppQuadTess::Init()
{
    DefaultCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Quad Tesselation Demo");
    DefaultInitCallbacks();

    InitCamera();
    InitMesh();
    InitShaders();

    DefaultInitGUI();

    glClearColor(135.0f / 255.0f, 206.0f / 255.0f, 235.0f / 255.0f, 0.0f);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glLineWidth(5.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void AppQuadTess::RenderSceneCB(float dt)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_quadTessTech.SetWVP(m_pGameCamera->GetViewProjMatrix());
    m_quadTessTech.SetLevels(m_outerLevelLeft,
                             m_outerLevelBottom,
                             m_outerLevelRight,
                             m_outerLevelTop,
                             m_innerLevelLeftRight,
                             m_innerLevelTopBottom);
    m_vertexBuffer.Render();
    RenderGUI();
}

void AppQuadTess::RenderGUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Quad Tessellation");

    ImGui::SliderFloat("Outer Left Tessellation Level", &this->m_outerLevelLeft, 0.0f, 50.0f);
    ImGui::SliderFloat("Outer Bottom Tessellation Level", &this->m_outerLevelBottom, 0.0f, 50.0f);
    ImGui::SliderFloat("Outer Right Tessellation Level", &this->m_outerLevelRight, 0.0f, 50.0f);
    ImGui::SliderFloat("Outer Top Tessellation Level", &this->m_outerLevelTop, 0.0f, 50.0f);

    ImGui::SliderFloat("Inner Left/Right Tessellation Level", &this->m_innerLevelLeftRight, 0.0f, 50.0f);
    ImGui::SliderFloat("Inner Top/Bottom Tessellation Level", &this->m_innerLevelTopBottom, 0.0f, 50.0f);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::End();

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(m_pWindow, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void AppQuadTess::InitCamera()
{
    glm::vec3 CameraPos(0.0f, 0.0f, 1.0f);
    glm::vec3 CameraTarget(0.0f, 0.0f, -1.0f);
    glm::vec3 Up(0.0f, 1.0f, 0.0f);

    float c = 3.5f;
    OrthoProjInfo orthoProjInfo;
    orthoProjInfo.l = -0.4f * c;
    orthoProjInfo.r = 0.4f * c;
    orthoProjInfo.b = -0.3f * c;
    orthoProjInfo.t = 0.3f * c;
    orthoProjInfo.n = 0.1f;
    orthoProjInfo.f = 100.0f;
    orthoProjInfo.Width = WINDOW_WIDTH;
    orthoProjInfo.Height = WINDOW_HEIGHT;

    m_pGameCamera = new Camera(orthoProjInfo, CameraPos, CameraTarget, Up);
    m_pGameCamera->SetSpeed(0.1f);
}

void AppQuadTess::InitShaders()
{
    if (!m_quadTessTech.Init())
    {
        printf("Error initializing the quad tesselation technique\n");
        exit(1);
    }

    m_quadTessTech.Enable();
    m_quadTessTech.SetColor(glm::vec4(1.0f, 1.0f, 0.0f, 0.0f));
}

void AppQuadTess::InitMesh()
{
    int numVertexElements = 2;
    m_vertexBuffer.Init(m_vertices, numVertexElements, GL_PATCHES);
}
