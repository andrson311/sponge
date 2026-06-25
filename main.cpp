#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cstdint>
#include <unistd.h>
#include "app.h"
#include "app_terrain.h"

int main(int argc, char **argv)
{
    // App *app = new App();
    AppTerrain *app = new AppTerrain();

    app->Init();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    //glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    app->Run();

    delete app;

    return 0;
}