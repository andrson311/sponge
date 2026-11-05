#pragma once

#include "core_model.h"
#include "gl_indirect_render.h"

class GLModel : public CoreModel
{
public:
    GLModel();
    GLModel(CoreRenderingSystem *pCoreRenderingSystem);
    ~GLModel();

    virtual void ConvertToMesh(const char *pFilename);
    bool LoadMesh(const std::string &filename);

    void Render(RenderCallbacks *pRenderCallbacks);
    void Render(u_int drawIndex, u_int primID);
    void Render(u_int numInstances, const glm::mat4 *WVPs, const glm::mat4 *worlds);

    virtual void SetColorTexture(int textureHandle);
    virtual void SetNormalMap(int textureHandle);
    virtual void SetHeightMap(int textureHandle);
    virtual void SetAmbientOcclusionMap(int textureHandle);

    virtual Texture *AllocTexture2D();
    virtual void DestroyTexture(Texture *pTexture);

    void RenderIndirect(const glm::mat4 &objectMatrix);

    Texture *GetNormalMap() const { return m_pNormalMap; }
    Texture *GetHeightMap() const { return m_pHeightMap; }
    Texture *GetAOMap() const { return m_pAOMap; }

protected:
    virtual void AllocBuffers();
    virtual void PopulateBuffersSkinned(std::vector<SkinnedVertex> &vertices);
    virtual void PopulateBuffers(std::vector<Vertex> &vertices);
    virtual void InitGeometryPost();

private:
    template <typename VertexType>
    void PopulateBuffersInternal(std::vector<VertexType> &vertices);

    template <typename VertexType>
    void PopulateBuffersPVP(std::vector<VertexType> &vertices);

    template <typename VertexType>
    void PopulateBuffersNonDSA(std::vector<VertexType> &vertices);

    template <typename VertexType>
    void PopulateBuffersDSA(std::vector<VertexType> &vertices);

    void RenderMesh(int meshIndex, RenderCallbacks *pRenderCallbacks = NULL);
    void BindTextures(int materialIndex);

    GLuint m_VAO = 0;
    GLuint m_buffers[NUM_BUFFERS] = {0};

    size_t m_vertexBufferSizeBytes = 0;
    size_t m_indexBufferSizeBytes = 0;

    IndirectRender m_indirectRender;

    Texture *m_pNormalMap = NULL;
    Texture *m_pHeightMap = NULL;
    Texture *m_pAOMap = NULL;
};
