#include "../inc/renderer.h"
#include "../inc/util.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Renderer::Renderer()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    dirLight.AmbientIntensity = 0.5;
    dirLight.DiffuseIntensity = 2.0f;
    dirLight.WorldDirection = glm::vec3(-1.0f, 0.0f, 0.0f);

    pointLights[0].DiffuseIntensity = 1.0f;
    pointLights[0].Color = glm::vec3(1.0f);
    pointLights[0].Attenuation.Linear = 0.2f;
    pointLights[0].Attenuation.Exp = 0.0f;

    pointLights[1].DiffuseIntensity = 1.0f;
    pointLights[1].Color = glm::vec3(1.0f);
    pointLights[1].Attenuation.Linear = 0.0f;
    pointLights[1].Attenuation.Exp = 0.2f;
}

Renderer::~Renderer()
{
    if (pGameCamera)
    {
        delete pGameCamera;
    }

    if (pMesh)
    {
        delete pMesh;
    }

    if (pLightingTech)
    {
        delete pLightingTech;
    }
}

bool Renderer::Init()
{
    glm::vec3 CameraPos(0.0f, 5.0f, -8.0f);
    glm::vec3 CameraTarget(0.0f, -0.5f, 1.0f);
    glm::vec3 CameraUp(0.0f, 1.0f, 0.0f);
    pGameCamera = new Camera(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        CameraPos,
        CameraTarget,
        CameraUp);

    pMesh = new Mesh();
    if (!pMesh->LoadMesh("assets/box_terrain/box_terrain.obj"))
    {
        return false;
    }

    pLightingTech = new LightingTechnique();
    if (!pLightingTech->Init())
    {
        return false;
    }

    pLightingTech->Enable();
    pLightingTech->SetTextureUnit(COLOR_TEXTURE_UNIT_INDEX);
    pLightingTech->SetSpecularExponentTextureUnit(SPECULAR_TEXTURE_UNIT_INDEX);

    return true;
}

void Renderer::RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pGameCamera->OnRender();

    WorldTrans &worldTransform = pMesh->GetWorldTransform();

    worldTransform.SetRotation(0.0f, 0.0f, 0.0f);
    worldTransform.SetPosition(0.0f, 0.0f, 10.0f);

    glm::mat4 World = worldTransform.GetMatrix();

    dirLight.CalcLocalDirection(World);

    glm::mat4 View = pGameCamera->GetMatrix();

    float aspectRatio = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

    glm::mat4 Projection = glm::perspective(
        glm::radians(FOV), aspectRatio, zNear, zFar);

    glm::mat4 WVP = Projection * View * World;
    pLightingTech->SetWVP(WVP);
    pLightingTech->SetDirectionalLight(dirLight);

    counter += 0.01f;
    pointLights[0].WorldPosition.x = -8.0f;
    pointLights[0].WorldPosition.y = sinf(counter) * 4 + 4;
    pointLights[0].WorldPosition.z = 0.0f;
    pointLights[0].CalcLocalPosition(worldTransform);

    pointLights[1].WorldPosition.x = 8.0f;
    pointLights[1].WorldPosition.y = sinf(counter) * 4 + 4;
    pointLights[1].WorldPosition.z = 0.0f;
    pointLights[1].CalcLocalPosition(worldTransform);

    pLightingTech->SetPointLights(2, pointLights);

    pLightingTech->SetMaterial(pMesh->GetMaterial());

    glm::mat4 CameraToLocalTranslation = worldTransform.GetReversedTranslationMatrix();
    glm::mat4 CameraToLocalRotation = worldTransform.GetReversedRotationMatrix();
    glm::mat4 CameraToLocalTransformation = CameraToLocalRotation * CameraToLocalTranslation;
    glm::vec4 CameraWorldPos = glm::vec4(pGameCamera->GetPos(), 1.0f);
    glm::vec4 CameraLocalPos = CameraToLocalTransformation * CameraWorldPos;
    glm::vec3 CameraLocalPos3f(CameraLocalPos);

    pLightingTech->SetCameraLocalPos(CameraLocalPos3f);

    pMesh->Render();

    glutPostRedisplay();
    glutSwapBuffers();
}

#define ATTEN_STEP 0.01f

void Renderer::KeyboardCB(u_char key, int mouse_x, int mouse_y)
{
    switch (key)
    {
    case 'q':
    case 27: // escape key code
        exit(0);

    case 'a':
        pointLights[0].Attenuation.Linear += ATTEN_STEP;
        pointLights[1].Attenuation.Linear += ATTEN_STEP;
        break;

    case 'z':
        pointLights[0].Attenuation.Linear -= ATTEN_STEP;
        pointLights[1].Attenuation.Linear -= ATTEN_STEP;
        break;

    case 's':
        pointLights[0].Attenuation.Exp += ATTEN_STEP;
        pointLights[1].Attenuation.Exp += ATTEN_STEP;
        break;

    case 'x':
        pointLights[0].Attenuation.Exp -= ATTEN_STEP;
        pointLights[1].Attenuation.Exp -= ATTEN_STEP;
        break;
    }

    printf("Linear %f Exp %f\n", pointLights[0].Attenuation.Linear, pointLights[0].Attenuation.Exp);
    pGameCamera->OnKeyboard(key);
}

void Renderer::SpecialKeyboardCB(int key, int mouse_x, int mouse_y)
{
    pGameCamera->OnKeyboard(key);
}

void Renderer::PassiveMouseCB(int x, int y)
{
    pGameCamera->OnMouse(x, y);
}
