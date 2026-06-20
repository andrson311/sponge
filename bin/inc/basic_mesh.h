#pragma once

#include <map>
#include <vector>
#include <string>
#include <GL/glew.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "texture.h"
#include "world_transform.h"
#include "mesh_common.h"
#include "material.h"
#include "util.h"

class BasicMesh : public MeshCommon
{
public:
    BasicMesh() {};
    ~BasicMesh();

    bool LoadMesh(const std::string &Filename, int AssimpFlags = ASSIMP_LOAD_FLAGS);

    void Render(IRenderCallbacks *pRenderCallbacks = NULL);
    void Render(u_int DrawIndex, u_int PrimID);
    void Render(u_int NumInstances, const glm::mat4 *WVPMats, const glm::mat4 *WorldMats);

    const Material &GetMaterial();
    PBRMaterial &GetPBRMaterial() { return m_Materials[0].PBRmaterial; }

    void GetLeadingVertex(u_int DrawIndex, u_int PrimID, aiVector3D &Vertex);
    void SetPBR(bool IsPBR) { m_isPBR = IsPBR; }

protected:
    void Clear();
    virtual void ReserveSpace(u_int NumVertices, u_int NumIndices);
    virtual void InitSingleMesh(u_int MeshIndex, const aiMesh *paiMesh);
    // virtual void InitSingleMeshOpt(u_int MeshIndex, const aiMesh *paiMesh);
    virtual void PopulateBuffers();
    virtual void PopulateBuffersNonDSA();
    // virtual void PopulateBuffersDSA();

    struct BasicMeshEntry
    {
        BasicMeshEntry()
        {
            NumIndices = 0;
            BaseVertex = 0;
            BaseIndex = 0;
            MaterialIndex = INVALID_MATERIAL;
        }

        u_int NumIndices;
        u_int BaseVertex;
        u_int BaseIndex;
        u_int MaterialIndex;
    };

    std::vector<BasicMeshEntry> m_Meshes;

    const aiScene *m_pScene = NULL;

    glm::mat4 m_GlobalInverseTransform;

    std::vector<u_int> m_Indices;

    enum BUFFER_TYPE
    {
        INDEX_BUFFER = 0,
        VERTEX_BUFFER = 1,
        WVP_MAT_BUFFER = 2,
        WORLD_MAT_BUFFER = 3,
        NUM_BUFFERS = 4
    };

    GLuint m_VAO = 0;
    GLuint m_Buffers[NUM_BUFFERS] = {0};

private:
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec2 TexCoords;
        glm::vec3 Normal;
    };

    bool InitFromScene(const aiScene *pScene, const std::string &Filename);
    void CountVerticesAndIndices(const aiScene *pScene, u_int &NumVertices, u_int &NumIndices);
    void InitAllMeshes(const aiScene *pScene);
    // void OptimizeMesh(int MeshIndex, std::vector<u_int> &Indices, std::vector<Vertex> &Vertices);
    bool InitMaterials(const aiScene *pScene, const std::string &Filename);
    void LoadTextures(const std::string &Dir, const aiMaterial *pMaterial, int index);

    void LoadDiffuseTexture(const std::string &Dir, const aiMaterial *pMaterial, int index);
    void LoadDiffuseTextureEmbedded(const aiTexture *paiTexture, int MaterialIndex);
    void LoadDiffuseTextureFromFile(const std::string &Dir, const aiString &Path, int MaterialIndex);

    void LoadSpecularTexture(const std::string &Dir, const aiMaterial *pMaterial, int index);
    void LoadSpecularTextureEmbedded(const aiTexture *paiTexture, int MaterialIndex);
    void LoadSpecularTextureFromFile(const std::string &Dir, const aiString &Path, int MaterialIndex);

    void LoadAlbedoTexture(const std::string &Dir, const aiMaterial *pMaterial, int index);
    void LoadAlbedoTextureEmbedded(const aiTexture *paiTexture, int MaterialIndex);
    void LoadAlbedoTextureFromFile(const std::string &Dir, const aiString &Path, int MaterialIndex);

    void LoadMetallicTexture(const std::string &Dir, const aiMaterial *pMaterial, int index);
    void LoadMetallicTextureEmbedded(const aiTexture *paiTexture, int MaterialIndex);
    void LoadMetallicTextureFromFile(const std::string &Dir, const aiString &Path, int MaterialIndex);

    void LoadRoughnessTexture(const std::string &Dir, const aiMaterial *pMaterial, int index);
    void LoadRoughnessTextureEmbedded(const aiTexture *paiTexture, int MaterialIndex);
    void LoadRoughnessTextureFromFile(const std::string &Dir, const aiString &Path, int MaterialIndex);

    void LoadColors(const aiMaterial *pMaterial, int index);

    void SetupRenderMaterialsPhong(u_int MeshIndex, u_int MaterialIndex, IRenderCallbacks *pRenderCallbacks);
    void SetupRenderMaterialsPBR();

    std::vector<Material> m_Materials;
    std::vector<Vertex> m_Vertices;

    Assimp::Importer m_Importer;

    bool m_isPBR = false;
};
