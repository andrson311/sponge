#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include "glfw.h"
#include "camera.h"

class AppBase
{
public:
    virtual bool KeyboardCB(int key, int action, int mods);
    virtual void MouseButtonCB(int button, int action, int mods, int x, int y) {}
    virtual void MouseMoveCB(int x, int y);
    void Run();

protected:
    AppBase();
    ~AppBase();

    void InitBaseApp() {};
    void DefaultCreateWindow(int windowWidth, int windowHeight, const char *pWindowName);
    void DefaultCreateCameraPers();
    void DefaultInitGUI();
    void DefaultInitCallbacks();
    void SetWindowShouldClose();
    virtual void RenderSceneCB(float dt) = 0;

    GLFWwindow *m_pWindow = NULL;
    int m_windowWidth = 0;
    int m_windowHeight = 0;
    Camera *m_pGameCamera = NULL;
    bool m_isPaused = false;
    bool m_isWireframe = false;
};
