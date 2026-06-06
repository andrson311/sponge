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

    spotLights[0].DiffuseIntensity = 1.0f;
    spotLights[0].Color = glm::vec3(1.0f);
    spotLights[0].Attenuation.Linear = 0.01f;
    spotLights[0].Cutoff = 20.0f;

    spotLights[1].DiffuseIntensity = 1.0f;
    spotLights[1].Color = glm::vec3(1.0f, 1.0f, 1.0f);
    spotLights[1].Attenuation.Linear = 0.01f;
    spotLights[1].Cutoff = 10.0f;
}

Renderer::~Renderer()
{
    if (pGameCamera)
    {
        delete pGameCamera;
    }

    if (pTerrain)
    {
        delete pTerrain;
    }

    if (pMesh1)
    {
        delete pMesh1;
    }

    if (pMesh2)
    {
        delete pMesh2;
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

    pTerrain = new Mesh();
    if (!pTerrain->LoadMesh("assets/box_terrain/box_terrain.obj"))
    {
        return false;
    }

    pMesh1 = new Mesh();
    if (!pMesh1->LoadMesh("assets/zombie/Zombie.obj"))
    {
        return false;
    }

    pMesh2 = new Mesh();
    if (!pMesh2->LoadMesh("assets/vintage_grandfather_clock/vintage_grandfather_clock_01_4k.obj"))
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

    glm::mat4 View = pGameCamera->GetMatrix();

    float aspectRatio = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

    glm::mat4 Projection = glm::perspective(
        glm::radians(FOV), aspectRatio, zNear, zFar);

    counter += 0.01f;

    WorldTrans &worldTransform = pTerrain->GetWorldTransform();

    worldTransform.SetRotation(0.0f, 0.0f, 0.0f);
    worldTransform.SetPosition(0.0f, 0.0f, 10.0f);

    glm::mat4 World = worldTransform.GetMatrix();

    //dirLight.CalcLocalDirection(World);

    glm::mat4 WVP = Projection * View * World;
    pLightingTech->SetWVP(WVP);
    //pLightingTech->SetDirectionalLight(dirLight);

    pointLights[0].WorldPosition.x = -10.0f;
    pointLights[0].WorldPosition.y = 2.0f;
    pointLights[0].WorldPosition.z = 0.0f;
    pointLights[0].CalcLocalPosition(worldTransform);

    pointLights[1].WorldPosition.x = 10.0f;
    pointLights[1].WorldPosition.y = sinf(counter) * 4 + 4;
    pointLights[1].WorldPosition.z = 0.0f;
    pointLights[1].CalcLocalPosition(worldTransform);

    pLightingTech->SetPointLights(2, pointLights);

    spotLights[0].WorldPosition = pGameCamera->GetPos();
    spotLights[0].WorldDirection = pGameCamera->GetTarget();
    spotLights[0].CalcLocalDirectionAndPosition(worldTransform);

    spotLights[1].WorldPosition = glm::vec3(0.0f, 1.0f, 0.0f);
    spotLights[1].WorldDirection = glm::vec3(0.0f, -1.0f, 0.0f);
    spotLights[1].CalcLocalDirectionAndPosition(worldTransform);

    pLightingTech->SetSpotLights(2, spotLights);

    pLightingTech->SetMaterial(pTerrain->GetMaterial());

    glm::vec3 CameraLocalPos3f = worldTransform.WorldPosToLocalPos(pGameCamera->GetPos());
    pLightingTech->SetCameraLocalPos(CameraLocalPos3f);

    pTerrain->Render();

    WorldTrans &mesh1WorldTransform = pMesh1->GetWorldTransform();

    mesh1WorldTransform.SetPosition(0.0f, 4.0f, 0.0f);

    World = mesh1WorldTransform.GetMatrix();
    WVP = Projection * View * World;
    pLightingTech->SetWVP(WVP);

    pointLights[0].CalcLocalPosition(mesh1WorldTransform);
    pointLights[1].CalcLocalPosition(mesh1WorldTransform);
    pLightingTech->SetPointLights(2, pointLights);

    spotLights[0].CalcLocalDirectionAndPosition(mesh1WorldTransform);
    spotLights[1].CalcLocalDirectionAndPosition(mesh1WorldTransform);
    pLightingTech->SetSpotLights(2, spotLights);

    pLightingTech->SetMaterial(pMesh1->GetMaterial());

    CameraLocalPos3f = mesh1WorldTransform.WorldPosToLocalPos(pGameCamera->GetPos());
    pLightingTech->SetCameraLocalPos(CameraLocalPos3f);

    pMesh1->Render();

    WorldTrans &mesh2WorldTransform = pMesh2->GetWorldTransform();
    mesh2WorldTransform.SetPosition(0.0f, 1.0f, 1.0f);

    World = mesh2WorldTransform.GetMatrix();
    WVP = Projection * View * World;

    pLightingTech->SetWVP(WVP);

    pointLights[0].CalcLocalPosition(mesh2WorldTransform);
    pointLights[1].CalcLocalPosition(mesh2WorldTransform);
    pLightingTech->SetPointLights(2, pointLights);

    spotLights[0].CalcLocalDirectionAndPosition(mesh2WorldTransform);
    spotLights[1].CalcLocalDirectionAndPosition(mesh2WorldTransform);
    pLightingTech->SetSpotLights(2, spotLights);

    pLightingTech->SetMaterial(pMesh2->GetMaterial());

    CameraLocalPos3f = mesh2WorldTransform.WorldPosToLocalPos(pGameCamera->GetPos());
    pLightingTech->SetCameraLocalPos(CameraLocalPos3f);

    pMesh2->Render();

    glutPostRedisplay();
    glutSwapBuffers();
}

#define ATTEN_STEP 0.01f
#define ANGLE_STEP 1.0f

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

    case 'd':
        spotLights[0].Cutoff += ANGLE_STEP;
        break;

    case 'c':
        spotLights[0].Cutoff -= ANGLE_STEP;
        break;

    case 'g':
        spotLights[1].Cutoff += ANGLE_STEP;
        break;

    case 'b':
        spotLights[1].Cutoff -= ANGLE_STEP;
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
