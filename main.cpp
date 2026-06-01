#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cstdint>
#include <unistd.h>
#include "bin/renderer.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

Renderer* pRenderer = NULL;

static void RenderSceneCB()
{
    pRenderer->RenderSceneCB();
}

static void KeyboardCB(u_char key, int mouse_x, int mouse_y)
{
    pRenderer->KeyboardCB(key, mouse_x, mouse_y);
}

static void SpecialKeyboardCB(int key, int mouse_x, int mouse_y)
{
    pRenderer->SpecialKeyboardCB(key, mouse_x, mouse_y);
}

static void PassiveMouseCB(int x, int y)
{
    pRenderer->PassiveMouseCB(x, y);
}

static void InitializeGlutCallbacks()
{
    glutDisplayFunc(RenderSceneCB);
    glutKeyboardFunc(KeyboardCB);
    glutSpecialFunc(SpecialKeyboardCB);
    glutPassiveMotionFunc(PassiveMouseCB);
}

int main(int argc, char **argv)
{
    srandom(getpid());

    glutInit(&argc, argv);

    glutInitContextVersion(3, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    int x = 1920 + 200; // offset for 1920 pixels to display the window on the second screen
    int y = 100;
    glutInitWindowPosition(x, y);
    int win = glutCreateWindow("Test");
    printf("window id: %d\n", win);

    // char game_mode_string[64];
    // snprintf(
    //     game_mode_string,
    //     sizeof(game_mode_string),
    //     "%dx%d@32",
    //     WINDOW_WIDTH,
    //     WINDOW_HEIGHT);

    // glutGameModeString(game_mode_string);
    // glutEnterGameMode();

    GLenum res = glewInit();

    if (res != GLEW_OK)
    {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    InitializeGlutCallbacks();

    pRenderer = new Renderer();

    if (!pRenderer->Init())
    {
        return 1;
    }

    glutMainLoop();

    return 0;
}