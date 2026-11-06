#include <stdio.h>
#include <string.h>
#include <math.h>

#include "common_includes.h"

#define WINDOW_WIDTH  1920
#define WINDOW_HEIGHT 1080


class LightingTest : public GameCallbacks
{
public:

    virtual ~LightingTest()
    {        
    }


    void Init()
    {
        bool LoadBasicShapes = false;
        m_pRenderingSystem = RenderingSystem::CreateRenderingSystem(RENDERING_SYSTEM_GL, this, LoadBasicShapes);
        m_pRenderingSystem->CreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Lighting Test");

        m_pScene = m_pRenderingSystem->CreateEmptyScene();
        m_pScene->SetClearColor(glm::vec4(0.1f, 0.1f, 0.15f, 0.0f));
        m_pScene->SetCamera(glm::vec3(0.0f, 15.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        m_pScene->SetCameraZRange(0.1f, 1000.0f);
        m_pScene->GetConfig()->ForcePBRDisabled(true);
        m_pRenderingSystem->SetScene(m_pScene);
        
        InitLights();                

        InitObjects();
    }


    void Run()
    {
        m_pRenderingSystem->Execute();
    }

    void OnFrameEnd() override // TEMP-DEBUG
    {
        static int frame = 0;
        if (++frame % 60 == 0 && frame <= 600)
        {
            unsigned char px[4];
            int maxv[3] = {0, 0, 0};
            for (int y = 100; y < WINDOW_HEIGHT; y += 100)
                for (int x = 100; x < WINDOW_WIDTH; x += 100)
                {
                    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);
                    for (int c = 0; c < 3; c++)
                        if (px[c] > maxv[c]) maxv[c] = px[c];
                }
            glReadPixels(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, px);
            fprintf(stderr, "[DBG] frame %d center %d %d %d max %d %d %d\n", frame, px[0], px[1], px[2], maxv[0], maxv[1], maxv[2]);
        }
    }

    void OnFrame(long long DeltaTime) override
    {
        m_counter += 0.03f;
        m_pScene->GetDirLights()[0].WorldDirection = glm::vec3(sinf(m_counter), 0.0f, cosf(m_counter));
        m_pScene->GetPointLights()[0].WorldPosition = glm::vec3(1 - sinf(m_counter), 1.0f, 1.0f + cosf(m_counter));
    }

private:

    void InitLights()
    {
        m_dirLight.WorldDirection = glm::vec3(1.0f, -0.5f, 0.0f);
        m_dirLight.DiffuseIntensity = 1.0f;
        m_dirLight.AmbientIntensity = 0.3f;
        m_pScene->GetDirLights().push_back(m_dirLight);

        m_pointLight.DiffuseIntensity = 0.3f;
        m_pointLight.Color = glm::vec3(1.0f, 1.0f, 1.0f);

        m_pScene->GetPointLights().push_back(m_pointLight);
    }

    void InitObjects()
    {
        m_pTerrainModel = m_pRenderingSystem->LoadModel("res/iclone-7-raptoid-mascot/scene.gltf");
        m_pTerrainSceneObject = m_pScene->CreateSceneObject(m_pTerrainModel);
        m_pTerrainSceneObject->SetPosition(0.0f, 0.0f, -40.0f);
        m_pTerrainSceneObject->SetRotation(glm::vec3(0.0f, 0.0f, -90.0f));
        m_pTerrainSceneObject->SetScale(glm::vec3(0.1f));
        m_pScene->AddToRenderList(m_pTerrainSceneObject);
    }

    RenderingSystem* m_pRenderingSystem = NULL;
    Scene* m_pScene = NULL;
    Model* m_pTerrainModel = NULL;
    SceneObject* m_pTerrainSceneObject = NULL;
    DirectionalLight m_dirLight;
    PointLight m_pointLight;
    float m_counter = 0;    
};


void test_lighting()
{
    LightingTest App;
    App.Init();
    App.Run();
}

int main() {
    test_lighting();
}

