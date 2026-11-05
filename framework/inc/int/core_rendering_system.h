#pragma once

#include "camera.h"
#include "rendering_system.h"
#include "core_model.h"
#include "core_scene.h"

class CoreRenderingSystem : public RenderingSystem
{
public:
    virtual Scene *CreateDefaultScene();
    virtual void *CreateWindow(int width, int height, const char *pWindowName);

    virtual Model *LoadModel(const std::string &filename);
    Model *LoadModel(const std::string &filename, const ModelLoadFlags &flags);
    virtual Model *LoadMesh(const std::string &filename);

    virtual Grid *CreateGrid(int width, int depth);

    virtual Model *GetModel(const std::string &basicShape);
    virtual void SetScene(Scene *pScene);
    virtual Scene *GetScene();

    virtual BaseTexture *GetTexture(int textureHandle) = 0;

    virtual void GetWindowSize(int &width, int &height) const
    {
        width = m_windowWidth;
        height = m_windowHeight;
    }

    virtual double GetElapsedTime() const { return m_elapsedTime; }
    virtual CameraFPS *GetCurrentCamera() { return m_pCamera; }
    virtual void ImGuiTextureWindow(const char *pTitle) = 0;

protected:
    CoreRenderingSystem(GameCallbacks *pGameCallbacks, bool loadBasicShapes);
    ~CoreRenderingSystem() = default;

    virtual void *CreateWindowInternal(const char *pWindowName) = 0;
    virtual Scene *CreateSceneInternal(const std::string &filename, const ModelLoadFlags &flags);
    virtual CoreModel *LoadModelInternal(const std::string &filename, const ModelLoadFlags &flags) = 0;
    virtual CoreModel *LoadMeshInternal(const std::string &filename) = 0;
    virtual Grid *CreateGridInternal(int width, int depth) = 0;

    virtual void SetCamera(CameraFPS *pCamera) = 0;

    double m_elapsedTime = 0;
    int m_windowWidth = 0;
    int m_windowHeight = 0;
    CameraFPS *m_pCamera = NULL;
    GameCallbacks *m_pGameCallbacks = NULL;
    GameCallbacks m_defaultGameCallbacks;
    CoreScene *m_pScene = NULL;

private:
    void InitializeBasicShapes();

    std::vector<CoreModel *> m_models;
    std::map<std::string, CoreModel *> m_shapeToModel;
    int m_numModels = 0;
    bool m_loadBasicShapes = false;
};
