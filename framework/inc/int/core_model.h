#pragma once

#include <map>
#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "core_material.h"
#include "camera.h"
#include "lighting.h"
#include "scene.h"
#include "model.h"
#include "core_mesh.h"
#include "gl_basic_mesh_entry.h"
#include "util.h"

void GetVertexSizesInBytes(size_t &VertexSize, size_t &SkinnedVertexSize);

class RenderCallbacks
{
public:
    virtual void DrawStartCB(u_int DrawIndex) = 0;
    virtual void ControlSpecularExponentCB(bool IsEnabled) = 0;
    virtual void SetMaterialCB(const CoreMaterial &material) = 0;
    virtual void SetWorldMatrixCB(const glm::mat4 &World) = 0;
};

class CoreRenderingSystem;

class CoreModel : public Model
{
public:
    CoreModel() {}
    CoreModel(CoreRenderingSystem *pCoreRenderingSystem) { m_pCoreRenderingSystem = pCoreRenderingSystem; }

    bool IsPBR() const;
    void DestroyModel();

    bool LoadAssimpModel(const std::string &filename)
    {
        ModelLoadFlags defaultFlags;
        return LoadAssimpModel(filename, defaultFlags);
    }
    bool LoadAssimpModel(const std::string &filename, const ModelLoadFlags &flags);

    const std::vector<CameraFPS> &GetCameras() const { return m_cameras; }

    u_int NumBones() const { return (u_int)m_boneNameToIndexMap.size(); }
    void GetBoneTransforms(float animationTimeSec, std::vector<glm::mat4> &transforms, u_int animationIndex = 0);
    void GetBoneTransformsBlended(float animationTimeSec, std::vector<glm::mat4> &transforms,
                                  u_int startAnimIndex, u_int endAnimIndex, float blendFactor);

    const std::vector<DirectionalLight> &GetDirLights() const { return m_dirLights; }
    const std::vector<SpotLight> &GetSpotLights() const { return m_spotLights; }
    const std::vector<PointLight> &GetPointLights() const { return m_pointLights; }

    void SetPointLight(int i, const PointLight &light)
    {
        if (i < m_pointLights.size())
        {
            m_pointLights[i] = light;
        }
    }

    void SetDirLight(int i, const DirectionalLight &light)
    {
        if (i < m_dirLights.size())
        {
            printf("Directional light %d set with ambient intensity: %f\n", light.AmbientIntensity);
            m_dirLights[i] = light;
        }
    }

    void SetTextureScale(float scale) { m_textureScale = scale; }
    bool IsAnimated() const;
    const CoreMaterial *GetMaterialForMesh(int meshIndex) const;

    virtual void SetColorTexture(int textureHandle) { assert(0); }
    virtual void SetNormalMap(int textureHandle) { assert(0); }
    virtual void SetHeightMap(int textureHandle) { assert(0); }
    virtual void SetAmbientOcclusionMap(int textureHandle) { assert(0); }
    virtual int GetMeshIndex(const std::string &name);
    virtual size_t GetNumMeshes() const { return m_meshes.size(); }
    static void GetVertexSizesInBytes(size_t &VertexSize, size_t &SkinnedVertexSize);

protected:
    virtual void AllocBuffers() = 0;
    virtual Texture *AllocTexture2D() = 0;
    virtual void DestroyTexture(Texture *pTexture) = 0;

    enum BUFFER_TYPE
    {
        INDEX_BUFFER = 0,
        VERTEX_BUFFER = 1,
        WVP_MAT_BUFFER = 2,
        WORLD_MAT_BUFFER = 3,
        NUM_BUFFERS = 4
    };

#define MAX_NUM_BONES_PER_VERTEX 4

    struct VertexBoneData
    {
        u_int BoneIDs[MAX_NUM_BONES_PER_VERTEX] = {0};
        float Weights[MAX_NUM_BONES_PER_VERTEX] = {0.0f};
        int index = 0;

        VertexBoneData() {}

        void AddBoneData(u_int BoneID, float Weight)
        {
            for (int i = 0; i < index; i++)
            {
                if (BoneIDs[i] == BoneID)
                {
                    return;
                }
            }

            if (Weight == 0.0f)
            {
                return;
            }

            if (index >= MAX_NUM_BONES_PER_VERTEX)
            {
                return;
                assert(0);
            }

            BoneIDs[index] = BoneID;
            Weights[index] = Weight;

            index++;
        }
    };

    struct Vertex
    {
        glm::vec3 Position;
        glm::vec2 TexCoords0;
        glm::vec2 TexCoords1;
        glm::vec3 Normal;
        glm::vec3 Tangent;
        glm::vec3 Bitangent;
        glm::vec4 Color;
    };

    struct SkinnedVertex : Vertex
    {
        VertexBoneData Bones;
    };

    virtual void InitGeometryPost() = 0;

    std::vector<BasicMeshEntry> m_meshes;
    std::vector<CoreMaterial> m_materials;
    std::map<std::string, int> m_meshNameToMeshIndex;

    std::vector<u_int> m_indices;
    CoreRenderingSystem *m_pCoreRenderingSystem = NULL;
    std::string m_filename;

private:
    template <typename VertexType>
    void ReserveSpace(std::vector<VertexType> &Vertices,
                      u_int NumVertices, u_int NumIndices);

    template <typename VertexType>
    void InitSingleMesh(std::vector<VertexType> &Vertices,
                        u_int MeshIndex, const aiMesh *paiMesh);

    template <typename VertexType>
    void InitSingleMeshOpt(std::vector<VertexType> &Vertices,
                           u_int MeshIndex, const aiMesh *paiMesh);

    virtual void PopulateBuffersSkinned(std::vector<SkinnedVertex> &Vertices) = 0;
    virtual void PopulateBuffers(std::vector<Vertex> &Vertices) = 0;

    u_int CountValidFaces(const aiMesh &mesh);

    bool InitFromScene(const aiScene *pScene, const std::string &filename);
    bool InitGeometry(const aiScene *pScene, const std::string &filename);
    void InitBuffers(const aiScene *pScene,
                     u_int NumVertices, u_int NumIndices);

    template <typename VertexType>
    void InitGeometryInternal(std::vector<VertexType> &Vertices,
                              int NumVertices, int NumIndices);

    void InitLights(const aiScene *pScene);
    void InitSingleLight(const aiScene *pScene, const aiLight &light);
    void InitDirectionalLight(const aiScene *pScene, const aiLight &light);
    void InitPointLight(const aiScene *pScene, const aiLight &light);
    void InitSpotLight(const aiScene *pScene, const aiLight &light);

    void CountVerticesAndIndices(const aiScene *pScene,
                                 u_int &NumVertices, u_int &NumIndices);

    template <typename VertexType>
    void InitAllMeshes(const aiScene *pScene, std::vector<VertexType> &Vertices);

    template <typename VertexType>
    void OptimizeMesh(int MeshIndex, std::vector<u_int> &Indices,
                      std::vector<VertexType> &Vertices, std::vector<VertexType> &AllVertices);

    void CalculateMeshTransformations(const aiScene *pScene);
    void TraverseNodeHierarchy(glm::mat4 ParentTransformation, aiNode *pNode);

    bool InitMaterials(const aiScene *pScene, const std::string &filename);

    void LoadTextures(const std::string &dir, const aiMaterial *pMaterial, int index);
    void LoadDiffuseTexture(const std::string &dir, const aiMaterial *pMaterial, int index);
    void LoadSpecularTexture(const std::string &dir, const aiMaterial *pMaterail, int index);
    void LoadNormalTexture(const std::string &dir, const aiMaterial *pMaterial, int index);
    void LoadMetalnessTexture(const std::string &dir, const aiMaterial *pMaterial, int index);
    void LoadEmissiveTexture(const std::string &dir, const aiMaterial *pMaterial, int index);
    void LoadEmissionColorTexture(const std::string &dir, const aiMaterial *pMaterial, int index);
    void LoadNormalCameraTexture(const std::string &dir, const aiMaterial *pMaterial, int index);
    void LoadRoughnessTexture(const std::string &dir, const aiMaterial *pMaterial, int index);
    void LoadAmbientOcclusionTexture(const std::string &dir, const aiMaterial *pMaterial, int index);
    void LoadClearCoatTexture(const std::string &dir, const aiMaterial *pMaterial, int index);
    void LoadClearCoatRoughnessTexture(const std::string &dir, const aiMaterial *pMaterial, int index);
    void LoadClearCoatNormalTexture(const std::string &dir, const aiMaterial *pMaterial, int index);
    
    void LoadTexture(const std::string &dir, const aiMaterial *pMaterial, int MaterialIndex,
                     aiTextureType AssimpType, int AssimpTexIndex, TEXTURE_TYPE texType);
    void LoadTextureEmbedded(const aiTexture *paiTexture, int MaterialIndex,
                             TEXTURE_TYPE texType, bool IsSRGB);
    void LoadTextureFromFile(const std::string &dir, const aiString &path, int index,
                             TEXTURE_TYPE texType, bool IsSRGB);

    void LoadColors(const aiMaterial *pMaterial, int index);
    void ProcessClearCoat(int index, const aiMaterial *pMaterial, CoreMaterial &material);
    void LoadColor(const aiMaterial *pMaterial, glm::vec4 &color,
                   const char *paiMatKey, int aiMatType, int aiMatIndex, const char *pName);

    bool GetFullTransformation(const aiNode *pRootNode, const char *pName, glm::mat4 &transformation);
    void InitCameras(const aiScene *pScene);
    void InitSingleCamera(int index, const aiScene *pScene);

    const aiScene *m_pScene = NULL;
    glm::mat4 m_globalInverseTransform;

    Assimp::Importer m_importer;

    std::vector<CameraFPS> m_cameras;
    std::vector<DirectionalLight> m_dirLights;
    std::vector<PointLight> m_pointLights;
    std::vector<SpotLight> m_spotLights;
    float m_textureScale = 1.0f;
    u_int m_numAnimations = 0;

    glm::vec3 m_minPos = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    glm::vec3 m_maxPos = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    bool m_flipTexCoordV = false;

    //std::map<std::string, int> m_meshNameToMeshIndex;

    // skeletal animation stuff
    void LoadMeshBones(std::vector<SkinnedVertex> &SkinnedVertices, u_int MeshIndex, const aiMesh *paiMesh);
    void LoadSingleBone(std::vector<SkinnedVertex> &SkinnedVertices, u_int MeshIndex, const aiBone *pBone);
    int GetBoneID(const aiBone *pBone);

    void CalcInterpolatedScaling(aiVector3D &out, float animationTime, const aiNodeAnim *pNodeAnim);
    void CalcInterpolatedRotation(aiQuaternion &out, float animationTime, const aiNodeAnim *pNodeAnim);
    void CalcInterpolatedPosition(aiVector3D &out, float animationTime, const aiNodeAnim *pNodeAnim);

    u_int FindScaling(float animationTime, const aiNodeAnim *pNodeAnim);
    u_int FindRotation(float animationTime, const aiNodeAnim *pNodeAnim);
    u_int FindPosition(float animationTime, const aiNodeAnim *pNodeAnim);

    const aiNodeAnim *FindNodeAnim(const aiAnimation &animation, const std::string &nodeName);
    void ReadNodeHierarchy(float animationTime, const aiNode *pNode,
                           const glm::mat4 &parentTransform, const aiAnimation &animation);
    void ReadNodeHierarchyBlended(float startAnimationTimeTicks, float endAnimationTimeTicks,
                                  const aiNode *pNode, const glm::mat4 &parentTransform,
                                  const aiAnimation &startAnimation, const aiAnimation &endAnimation,
                                  float blendFactor);
    void MarkRequiredNodesForBone(const aiBone *pBone);
    void InitializeRequiredNodeMap(const aiNode *pNode);
    float CalcAnimationTimeTicks(float timeInSeconds, u_int animationIndex);

    struct LocalTransform
    {
        aiVector3D Scaling;
        aiQuaternion Rotation;
        aiVector3D Translation;
    };

    void CalcLocalTransform(LocalTransform &transform, float animationTimeTicks, const aiNodeAnim *pNodeAnim);

    std::map<std::string, u_int> m_boneNameToIndexMap;

    struct BoneInfo
    {
        glm::mat4 OffsetMatrix;
        glm::mat4 FinalTransformation;

        BoneInfo(const aiMatrix4x4 &offset)
        {
            OffsetMatrix = AssimpToGLM(offset);
            FinalTransformation = glm::mat4(0.0f);
        }
    };

    std::vector<BoneInfo> m_boneInfo;

    struct NodeInfo
    {
        NodeInfo() {}
        NodeInfo(const aiNode *n) { pNode = n; }

        const aiNode *pNode = NULL;
        bool isRequired = false;
    };

    std::map<std::string, NodeInfo> m_requiredNodeMap;
};
