#pragma once

#include "camera.h"
#include "scene.h"
#include "model.h"

class GameCallbacks
{
public:
    virtual void OnFrame(long long deltaTime) {} // time is in ms
    virtual void OnFrameEnd() {}

    virtual bool OnKeyboard(int key, int action) { return false; }
    virtual bool OnMouseMove(int x, int y) { return false; }
    virtual bool OnMouseButton(int button, int action, int mode,
                               int x, int y) { return false; }
};

enum RENDERING_SYSTEM
{
    RENDERING_SYSTEM_GL,
    RENDERING_SYSTEM_VK,
    RENDERING_SYSTEM_DX11,
    RENDERING_SYSTEM_DX12
};

enum TEXTURE_WRAP_MODE
{
    WRAP_MODE_CLAMP_TO_EDGE,
    WRAP_MODE_MIRRORED_REPEAT,
    WRAP_MODE_REPEAT,
    WRAP_MODE_CLAMP_TO_BORDER,
    NUM_WRAP_MODE
};

struct TextureConfig
{
    TEXTURE_WRAP_MODE m_wrapMode = WRAP_MODE_REPEAT;
    int m_numChannels = 0;
    bool m_isFloat = false;
    bool m_getMipmaps = true;
};

class RenderingSystem
{
public:
    static RenderingSystem *CreateRenderingSystem(RENDERING_SYSTEM renderingSystem,
                                                  GameCallbacks *pGameCallbacks, bool loadBasicShapes);

    virtual void *CreateWindow(int width, int height, const char *pWindowName) = 0;
    virtual void ReloadShaders() = 0;
    virtual void Shutdown() = 0;
    virtual void Execute() = 0;

    virtual Scene *CreateEmptyScene() = 0;
    Scene *CreateScene(const std::string &filename)
    {
        ModelLoadFlags defaultFlags;
        return CreateSceneInternal(filename, defaultFlags);
    }
    Scene *CreateScene(const std::string &filename, const ModelLoadFlags &flags)
    {
        return CreateSceneInternal(filename, flags);
    }
    virtual Scene *CreateDefaultScene() = 0;
    virtual void SetScene(Scene *pScene) = 0;
    virtual Scene *GetScene() = 0;

    virtual Model *LoadModel(const std::string &filename) = 0;
    virtual Model *LoadMesh(const std::string &filename) = 0;

    virtual Grid *CreateGrid(int width, int depth) = 0;

    virtual int LoadTexture2D(const std::string &filename, TextureConfig *pConfig = NULL) = 0;
    virtual int LoadTexture2D(const void *pData, int width, int height, TextureConfig *pConfig = NULL) = 0;
    virtual void UpdateTexture2D(int textureHandle, const void *pData) = 0;

    virtual int LoadCubemapTexture(const std::string &filename) = 0;

    virtual void *CreateTerrainGrid(int width, int depth) = 0;

    virtual void GetWindowSize(int &width, int &height) const = 0;
    virtual double GetElapsedTime() const = 0;
    virtual CameraFPS *GetCurrentCamera() = 0;
    virtual int GetTextureAPIHandle(int textureHandle) = 0;

protected:
    virtual Scene *CreateSceneInternal(const std::string &filename, const ModelLoadFlags &flags) = 0;
};
