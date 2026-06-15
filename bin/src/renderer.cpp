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

    //dirLight.AmbientIntensity = 1.0f;
    //dirLight.DiffuseIntensity = 1.0f;
    //dirLight.WorldDirection = glm::vec3(-1.0f, 0.0f, 0.0f);

    pointLights[0].AmbientIntensity = 1.0f;
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

    if (pMesh1)
    {
        delete pMesh1;
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

    pMesh1 = new MeshSkinned();
    //if (!pMesh1->LoadMesh("assets/wine_barrel/wine_barrel_01_4k.obj"))
    if (!pMesh1->LoadMesh("assets/example/example1.glb"))
    //if (!pMesh1->LoadMesh("assets/bob/boblampclean.md5mesh"))
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

    StartTimeMillis = GetCurrentTimeMillis();

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

    WorldTrans &worldTransform = pMesh1->GetWorldTransform();

    worldTransform.SetRotation(-90.0f, 0.0f, 180.0f);
    worldTransform.SetPosition(0.0f, 0.0f, 0.0f);
    worldTransform.SetScale(0.1f);

    glm::mat4 World = worldTransform.GetMatrix();

    //dirLight.CalcLocalDirection(World);

    glm::mat4 WVP = Projection * View * World;
    pLightingTech->SetWVP(WVP);
    //pLightingTech->SetDirectionalLight(dirLight);

    pointLights[0].WorldPosition.x = 0.0f;
    pointLights[0].WorldPosition.y = 1.0f;
    pointLights[0].WorldPosition.z = 1.0f;
    pointLights[0].CalcLocalPosition(worldTransform);

    pointLights[1].WorldPosition.x = 10.0f;
    pointLights[1].WorldPosition.y = 1.0f;
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

    pLightingTech->SetMaterial(pMesh1->GetMaterial());

    glm::vec3 CameraLocalPos3f = worldTransform.WorldPosToLocalPos(pGameCamera->GetPos());
    pLightingTech->SetCameraLocalPos(CameraLocalPos3f);

    long long CurrentTimeMillis = GetCurrentTimeMillis();
    float AnimationTimeSec = ((float)(CurrentTimeMillis - StartTimeMillis)) / 1000.0f;

    std::vector<glm::mat4> Transforms;
    pMesh1->GetBoneTransforms(AnimationTimeSec, Transforms);

    for (u_int i = 0; i < Transforms.size(); i++)
    {
        pLightingTech->SetBoneTransform(i, Transforms[i]);
    }
    
    pMesh1->Render();

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
