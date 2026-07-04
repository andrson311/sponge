#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cstdint>
#include <unistd.h>
#include "app.h"
// #include "app_terrain.h"
// #include "app_freetype.h"
// #include "app_bezier.h"

int main(int argc, char **argv)
{
    App *app = new App();
    // AppTerrain *app = new AppTerrain();
    // AppFreetype *app = new AppFreetype();
    // app = new AppBezier();

    app->Init();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CLIP_DISTANCE0);

    app->Run();

    delete app;

    return 0;
}
