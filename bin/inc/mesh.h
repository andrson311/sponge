#pragma once

#include <map>
#include <vector>
#include <string>
#include <GL/glew.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "texture.h"
#include "world_transform.h"
#include "mesh_common.h"
#include "material.h"
#include "util.h"

class Mesh : public MeshCommon
{
public:
    Mesh() {};
    ~Mesh();

    bool LoadMesh(const std::string &Filename, int AssimpFlags = ASSIMP_LOAD_FLAGS);

    void Render();
    void Render(IRenderCallbacks* pRenderCallbacks);
    void Render(u_int DrawIndex, u_int PrimID);
    void Render(u_int NumInstances, const glm::mat4 *WVPMats, const glm::mat4 *WorldMats);

    const Material &GetMaterial();

protected:
    void Clear();
    virtual void ReserveSpace(u_int NumVertices, u_int NumIndices);
    virtual void InitSingleMesh(u_int MeshIndex, const aiMesh *paiMesh);
    virtual void PopulateBuffers();

    struct MeshEntry
    {
        MeshEntry()
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

    std::vector<MeshEntry> m_Meshes;

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
    struct Vertex {
        glm::vec3 Position;
        glm::vec2 TexCoords;
        glm::vec3 Normal;
    };

    bool InitFromScene(const aiScene* pScene, const std::string& Filename);
    void CountVerticesAndIndices(const aiScene* pScene, u_int& NumVertices, u_int& NumIndices);
    void InitAllMeshes(const aiScene* pScene);
    bool InitMaterials(const aiScene* pScene, const std::string& Filename);
    void LoadTextures(const std::string& Dir, const aiMaterial* pMaterial, int index);

    void LoadDiffuseTexture(const std::string& Dir, const aiMaterial* pMaterial, int index);
    void LoadDiffuseTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex);
    void LoadDiffuseTextureFromFile(const std::string& Dir, const aiString& Path, int MaterialIndex);

    void LoadSpecularTexture(const std::string& Dir, const aiMaterial* pMaterial, int index);
    void LoadSpecularTextureEmbedded(const aiTexture* paiTexture, int MaterialIndex);
    void LoadSpecularTextureFromFile(const std::string& Dir, const aiString& Path, int MaterialIndex);

    void LoadColors(const aiMaterial* pMaterial, int index);

    std::vector<Material> m_Materials;
    std::vector<Vertex> m_Vertices;

    Assimp::Importer m_Importer;
};
