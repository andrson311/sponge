#include <iostream>
#include "../inc/glfw.h"

static void InitGlew()
{
    glewExperimental = GL_TRUE;
    GLenum res = glewInit();

    if (res != GLEW_OK)
    {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(res));
    }
}

GLFWwindow *InitGLFW(int width, int height, bool is_full_screen, const char *title)
{
    if (glfwInit() != 1)
    {
        printf("Error initializing GLFW");
        exit(1);
    }

    GLFWmonitor *monitor = is_full_screen ? glfwGetPrimaryMonitor() : NULL;
    GLFWwindow *window = glfwCreateWindow(width, height, title, monitor, NULL);

    if (!window)
    {
        const char* pDesc = NULL;
        int error_code = glfwGetError(&pDesc);

        fprintf(stderr, "Error creating window: %s", pDesc);
        exit(1);
    }

    glfwMakeContextCurrent(window);

    InitGlew();
    glfwSwapInterval(1);
    return window;
}
