#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#include "util.h"
#include "glfw.h"
#include "freetypeGL.h"

class AppFreetype
{
public:
    AppFreetype() {}
    virtual ~AppFreetype() {}

    void Init();
    void Run();
    void RenderSceneCB();
    void KeyboardCB(u_int key, int state);

private:
    void CreateWindow();

    GLFWwindow* window = NULL;
    FontRenderer m_fontRenderer;
};
