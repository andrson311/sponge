#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "rendering_system.h"

class BaseGLApp : public GameCallbacks
{
public:
    BaseGLApp(int windowWidth, int windowHeight, const char *pWindowName);
    ~BaseGLApp() {}

    virtual void OnFrame(double deltaTime);
    virtual void OnFrameEnd();
    virtual bool OnKeyboard(int key, int action);
    virtual bool OnMouseMove(int x, int y);
    virtual bool OnMouseButton(int button, int action, int mode, int x, int y);

protected:
    virtual void OnFrameChild(double deltaTime) {};
    virtual void OnFrameGUI();

    bool m_showGUI = false;
    RenderingSystem *m_pRenderingSystem = NULL;
    GLFWwindow *m_pWindow = NULL;

private:
    void InitGUI();

    bool m_isWireframe = false;
    bool m_leftMousePressed = false;
    bool m_midMousePressed = false;
};
