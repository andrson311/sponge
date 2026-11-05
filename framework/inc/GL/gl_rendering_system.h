#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "core_rendering_system.h"
#include "gl_forward_renderer.h"
#include "gl_scene.h"

class TerrainGrid
{
public:
    TerrainGrid(u_int indexCount) : m_indexCount(indexCount) {}
    void Render();

    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ibo = 0;
    u_int m_indexCount = 0;
};

class RenderingSystemGL : public CoreRenderingSystem
{
public:
    RenderingSystemGL(GameCallbacks *pGameCallbacks, bool loadBasicShapes);
    ~RenderingSystemGL() {};

    virtual void Shutdown();
    virtual void ReloadShaders();

    virtual Scene *CreateEmptyScene();
    virtual CoreModel *LoadModelInternal(const std::string &filename, const ModelLoadFlags &flags);
    virtual CoreModel *LoadMeshInternal(const std::string &filename);
    virtual Grid *CreateGridInternal(int width, int depth);
    virtual int LoadTexture2D(const std::string &filename, TextureConfig *pConfig);
    virtual int LoadTexture2D(const void *pData, int width, int height, TextureConfig *pConfig = NULL);
    virtual void UpdateTexture2D(int textureHandle, const void *pData);
    virtual int LoadCubemapTexture(const std::string &filename);

    virtual void Execute();
    virtual void ImGuiTextureWindow(const char *pTitle);
    virtual void *CreateTerrainGrid(int width, int height);
    virtual int GetTextureAPIHandle(int textureHandle);
    BaseTexture *GetTexture(int textureHandle);

    void OnKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
    void OnCursorPosCallback(GLFWwindow *window, double x, double y);
    void OnMouseButtonCallback(GLFWwindow *window, int button, int action, int mode);
    void GetMousePos(void *pWindow, int &x, int &y);

protected:
    virtual void *CreateWindowInternal(const char *pWindowName);
    virtual void SetCamera(CameraFPS *pCamera) { m_forwardRenderer.SetCamera(pCamera); };

private:
    void InitCallbacks();
    void SetDefaultGLState();

    GLFWwindow *m_pWindow = NULL;
    ForwardRenderer m_forwardRenderer;
    std::vector<BaseTexture *> m_textures;
    int m_numTextures = 0;
};
