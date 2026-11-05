#include <iostream>
#include "glfw.h"

static int glMajorVersion = 0;
static int glMinorVersion = 0;

int GetGLMajorVersion()
{
    return glMajorVersion;
}

int GetGLMinorVersion()
{
    return glMinorVersion;
}

int IsGLVersionHigher(int MajorVer, int MinorVer)
{
    return ((glMajorVersion >= MajorVer) && (glMinorVersion >= MinorVer));
}

static void InitGlew()
{
    glewExperimental = GL_TRUE;
    GLenum res = glewInit();

    // GLEW_ERROR_NO_GLX_DISPLAY only concerns GLX extensions (e.g. on Wayland); core functions are loaded
    if (res != GLEW_OK && res != GLEW_ERROR_NO_GLX_DISPLAY)
    {
        fprintf(stderr, "GLEW init error %u: %s\n", res, glewGetErrorString(res));
    }
}

GLFWwindow *InitGLFW(int major_ver, int minor_ver, int width, int height, bool is_full_screen, const char *title)
{
    if (glfwInit() != 1)
    {
        printf("Error initializing GLFW");
        exit(1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major_ver);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor_ver);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor *monitor = is_full_screen ? glfwGetPrimaryMonitor() : NULL;
    GLFWwindow *window = glfwCreateWindow(width, height, title, monitor, NULL);

    if (!window)
    {
        const char *pDesc = NULL;
        int error_code = glfwGetError(&pDesc);

        fprintf(stderr, "Error creating window: %s", pDesc);
        exit(1);
    }

    glfwMakeContextCurrent(window);

    glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);

    InitGlew();
    glfwSwapInterval(1);
    return window;
}
