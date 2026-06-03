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
}

Renderer::~Renderer()
{
    delete pGameCamera;
    delete pMesh;
}

bool Renderer::Init()
{
    glm::vec3 CameraPos(0.0f, 0.0f, -1.0f);
    glm::vec3 CameraTarget(0.0f, 0.0f, 1.0f);
    glm::vec3 CameraUp(0.0f, 1.0f, 0.0f);
    pGameCamera = new Camera(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        CameraPos,
        CameraTarget,
        CameraUp);
    
    pMesh = new Mesh();
    if (!pMesh->LoadMesh("assets/antique_ceramic_vase/antique_ceramic_vase_01_4k.obj"))
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

    float RotationAngle = 0.01f;

    WorldTrans& worldTransform = pMesh->GetWorldTransform();

    worldTransform.SetScale(2.0f);
    worldTransform.SetPosition(0.0f, 0.0f, 2.0f);
    worldTransform.Rotate(0.0f, RotationAngle, 0.0f);

    glm::mat4 World = worldTransform.GetMatrix();

    dirLight.CalcLocalDirection(World);

    glm::mat4 View = pGameCamera->GetMatrix();

    float aspectRatio = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

    glm::mat4 Projection = glm::perspective(
        glm::radians(FOV), aspectRatio, zNear, zFar);

    glm::mat4 WVP = Projection * View * World;
    pLightingTech->SetWVP(WVP);
    pLightingTech->SetDirectionalLight(dirLight);
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

void Renderer::KeyboardCB(u_char key, int mouse_x, int mouse_y)
{
    switch (key)
    {
    case 'q':
    case 27: // escape key code
        exit(0);
    }

    pGameCamera->OnKeyboard(key);
}

void Renderer::SpecialKeyboardCB(int key, int mouse_x, int mouse_y)
{
    pGameCamera->OnKeyboard(key);
}

void Renderer::PassiveMouseCB(int x, int y) {
    pGameCamera->OnMouse(x, y);
}
