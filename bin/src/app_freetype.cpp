#include "app_freetype.h"

void AppFreetype::Init()
{
    CreateWindow();
    m_fontRenderer.InitFontRenderer(WINDOW_WIDTH, WINDOW_HEIGHT);
}

void AppFreetype::Run()
{
    while (!glfwWindowShouldClose(window))
    {
        RenderSceneCB();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void AppFreetype::RenderSceneCB()
{
    glClearColor(0.35f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static int x = 0;
    int y = x;

    m_fontRenderer.RenderText(FONT_TYPE_AMIRI, blue, red, 50, 30, "FreetypeGL!");
    m_fontRenderer.RenderText(FONT_TYPE_LIBERASTIKA, white, blue, 50, 150, "FreetypeGL!");
    m_fontRenderer.RenderText(FONT_TYPE_LOBSTER, black, white, 50, 280, "FreetypeGL!");
    m_fontRenderer.RenderText(FONT_TYPE_LUCKIEST_GUY, yellow, black, 50, 390, "FreetypeGL!");
    m_fontRenderer.RenderText(FONT_TYPE_OLD_STANDARD, orange1, yellow, 50, 500, "FreetypeGL!");
    m_fontRenderer.RenderText(FONT_TYPE_SOURCE_CODE_PRO, orange2, orange1, 50, 610, "FreetypeGL!");
    m_fontRenderer.RenderText(FONT_TYPE_SOURCE_SANS_PRO, green, orange2, 50, 720, "FreetypeGL!");
    m_fontRenderer.RenderText(FONT_TYPE_VERA, gray, green, 50, 840, "FreetypeGL!");
    m_fontRenderer.RenderText(FONT_TYPE_VERA_MOBD, purple, gray, 50, 950, "FreetypeGL!");
    m_fontRenderer.RenderText(FONT_TYPE_VERA_MOBI, cyan, purple, 50, 1090, "FreetypeGL!");
}

void AppFreetype::KeyboardCB(u_int key, int state)
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
        }
    }
}

void AppFreetype::CreateWindow()
{
    int major_ver = 0;
    int minor_ver = 0;
    bool is_full_screen = false;
    window = InitGLFW(major_ver, minor_ver, WINDOW_WIDTH, WINDOW_HEIGHT, is_full_screen, "FreetypeGL demo");
    glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
}
