#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "bin/inc/math_3d.h"

// config parameters
glm::vec3 CameraPos(1.0f, 0.0f, 5.0f);
glm::vec3 CameraTarget(0.0f, 0.0f, 1.0f);
glm::vec3 LightDir(0.0f, 0.0f, 1.0f);

float zNear = 1.0f;
float zFar = 100.0f;
float FOV = 90.0f;
float WindowWidth = 1280;
float WindowHeight = 720;

int main(int argc, char *argv[])
{
    PersProjInfo persProjInfo;
    persProjInfo.FOV = FOV;
    persProjInfo.Width = WindowWidth;
    persProjInfo.Height = WindowHeight;
    persProjInfo.zNear = zNear;
    persProjInfo.zFar = zFar;

    glm::vec3 Up(0.0f, 1.0f, 0.0f);
    glm::mat4 View = glm::lookAt(CameraPos, CameraTarget, Up);

    glm::vec3 LightPosWorld;
    OrthoProjInfo orthoProjInfo;
    CalcTightLightProjection(View, LightDir, persProjInfo, LightPosWorld, orthoProjInfo);
    printf("\nFinal orthographic projection params\n");
    orthoProjInfo.Print();
    
    return 0;
}
