#include "common_includes.h"

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000

void test_minimal()
{
    bool loadBasicShapes = false;
    RenderingSystem *pRenderingSystem = RenderingSystem::CreateRenderingSystem(RENDERING_SYSTEM_GL,
                                                                               NULL, loadBasicShapes);
    pRenderingSystem->CreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Minimal test");
    pRenderingSystem->SetScene(pRenderingSystem->CreateEmptyScene());
    pRenderingSystem->Execute();
}

int main()
{
    test_minimal();
}
