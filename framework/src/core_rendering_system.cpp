#include <stdio.h>
#include <stdlib.h>

#include "core_rendering_system.h"
#include "core_scene.h"
#include "gl_rendering_system.h"

#define NUM_MODELS 1024

CoreRenderingSystem *g_pRenderingSystem = NULL;

CoreRenderingSystem::CoreRenderingSystem(GameCallbacks *pGameCallbacks, bool loadBasicShapes)
{
    if (pGameCallbacks)
    {
        m_pGameCallbacks = pGameCallbacks;
    }
    else
    {
        printf("%s:%d - using the default game callbacks\n", __FILE__, __LINE__);
        m_pGameCallbacks = &m_defaultGameCallbacks;
    }

    m_loadBasicShapes = loadBasicShapes;
    m_models.resize(NUM_MODELS, 0);
}

RenderingSystem *RenderingSystem::CreateRenderingSystem(RENDERING_SYSTEM renderingSystem,
                                                        GameCallbacks *pGameCallbacks,
                                                        bool loadBasicShapes)
{
    if (g_pRenderingSystem)
    {
        printf("%s:%d - rendering system already exists\n", __FILE__, __LINE__);
        exit(0);
    }

    switch (renderingSystem)
    {
    case RENDERING_SYSTEM_GL:
        g_pRenderingSystem = new RenderingSystemGL(pGameCallbacks, loadBasicShapes);
        break;

    default:
        printf("%s:%d - rendering system %d not supported\n",
               __FILE__, __LINE__, renderingSystem);
        exit(0);
    }

    return g_pRenderingSystem;
}

void *CoreRenderingSystem::CreateWindow(int width, int height, const char *pWindowName)
{
    m_windowWidth = width;
    m_windowHeight = height;

    void *pWindow = CreateWindowInternal(pWindowName);

    if (m_loadBasicShapes)
    {
        InitializeBasicShapes();
    }

    return pWindow;
}

void CoreRenderingSystem::InitializeBasicShapes()
{
    ModelLoadFlags defaultFlags;
    CoreModel *pModel = LoadModelInternal("res/sphere/sphere.obj", defaultFlags);
    m_shapeToModel["sphere"] = pModel;

    pModel = LoadModelInternal("res/box/box.obj", defaultFlags);
    m_shapeToModel["cube"] = pModel;

    pModel = LoadModelInternal("res/quad/quad.obj", defaultFlags);
    m_shapeToModel["square"] = pModel;
}

Scene *CoreRenderingSystem::CreateSceneInternal(const std::string &filename,
                                                const ModelLoadFlags &flags)
{
    CoreScene *pScene = (CoreScene *)CreateEmptyScene();
    pScene->LoadScene(filename, flags);
    return pScene;
}

Scene *CoreRenderingSystem::CreateDefaultScene()
{
    CoreScene *pScene = (CoreScene *)CreateEmptyScene();
    pScene->InitializeDefault();
    return pScene;
}

void CoreRenderingSystem::SetScene(Scene *pScene)
{
    m_pScene = (CoreScene *)pScene;
    m_pCamera = m_pScene->GetCurrentCamera();
    SetCamera(m_pCamera);
}

Scene *CoreRenderingSystem::GetScene()
{
    return m_pScene;
}

Model *CoreRenderingSystem::LoadModel(const std::string &filename)
{
    ModelLoadFlags defaultFlags;
    return LoadModel(filename, defaultFlags);
}

Model *CoreRenderingSystem::LoadModel(const std::string &filename,
                                      const ModelLoadFlags &flags)
{
    if (m_numModels == m_models.size())
    {
        printf("%s:%d - out of models space\n", __FILE__, __LINE__);
        exit(0);
    }

    CoreModel *pModel = LoadModelInternal(filename, flags);

    if (pModel)
    {
        m_models[m_numModels] = pModel;
        m_numModels++;
        pModel->SetName(filename);
    }
    else
    {
        printf("%s:%d - error loading '%s'\n",
               __FILE__, __LINE__, filename.c_str());
        exit(0);
    }

    return pModel;
}

Model *CoreRenderingSystem::LoadMesh(const std::string &filename)
{
    if (m_numModels == m_models.size())
    {
        printf("%s:%d - out of models space\n", __FILE__, __LINE__);
        exit(0);
    }

    CoreModel *pModel = LoadMeshInternal(filename);

    if (pModel)
    {
        m_models[m_numModels] = pModel;
        m_numModels++;
        pModel->SetName(filename);
    }
    else
    {
        printf("%s:%d - error loading '%s'\n",
               __FILE__, __LINE__, filename.c_str());
        exit(0);
    }

    return pModel;
}

Grid *CoreRenderingSystem::CreateGrid(int width, int depth)
{
    if (m_numModels == m_models.size())
    {
        printf("%s:%d - out of models space\n", __FILE__, __LINE__);
        exit(0);
    }

    Grid *pGrid = CreateGridInternal(width, depth);

    return pGrid;
}

Model *CoreRenderingSystem::GetModel(const std::string &basicShape)
{
    if (m_shapeToModel.find(basicShape) == m_shapeToModel.end())
    {
        printf("%s:%d - cannot find basic shape %s\n", __FILE__, __LINE__, basicShape.c_str());
        exit(0);
    }
    
    Model* pModel = m_shapeToModel[basicShape];
    return pModel;
}