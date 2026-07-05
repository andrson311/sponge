#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cstdint>
#include <unistd.h>
// #include "app.h"
// #include "app_terrain.h"
// #include "app_freetype.h"
// #include "app_bezier.h"
#include "app_quad_tess.h"

int main(int argc, char **argv)
{
    AppQuadTess *app = new AppQuadTess();
    // AppTerrain *app = new AppTerrain();
    // AppFreetype *app = new AppFreetype();
    // app = new AppBezier();

    app->Init();

    app->Run();

    delete app;

    return 0;
}
