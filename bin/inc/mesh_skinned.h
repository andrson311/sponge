#pragma once

#include <cassert>
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

#include "mesh_common.h"
#include "texture.h"
#include "world_transform.h"
#include "material.h"
#include "util.h"

class MeshSkinned : public MeshCommon
{
public:
    MeshSkinned() {}
    ~MeshSkinned();

    bool LoadMesh(const std::string &Filename);
    void Render();
    unsigned int NumBones() const { return (unsigned int)m_BoneNameToIndexMap.size(); }
    WorldTrans &GetWorldTransform() { return m_worldTransform; }
    const Material &GetMaterial();
    void GetBoneTransforms(float AnimationTimeSec, std::vector<glm::mat4> &Transforms);

private:
#define MAX_NUM_BONES_PER_VERTEX 4

    void Clear();

    bool InitFromScene(const aiScene *pScene, const std::string &Filename);
    void CountVerticesAndIndices(const aiScene *pScene, unsigned int &NumVertices, unsigned int &NumIndices);
    void ReserveSpace(unsigned int NumVertices, unsigned int NumIndices);
    void InitAllMeshes(const aiScene *pScene);
    void InitSingleMesh(unsigned int MeshIndex, const aiMesh *paiMesh);
    bool InitMaterials(const aiScene *pScene, const std::string &Filename);
    void PopulateBuffers();

    void LoadTextures(const std::string &Dir, const aiMaterial *pMaterial, int index);
    void LoadDiffuseTexture(const std::string &Dir, const aiMaterial *pMaterial, int index);
    void LoadSpecularTexture(const std::string &Dir, const aiMaterial *pMaterial, int index);
    void LoadColors(const aiMaterial *pMaterial, int index);

    struct VertexBoneData
    {
        unsigned int BoneIDs[MAX_NUM_BONES_PER_VERTEX] = {0};
        float Weights[MAX_NUM_BONES_PER_VERTEX] = {0.0f};

        VertexBoneData() {}

        void AddBoneData(unsigned int BoneID, float Weight)
        {
            for (uint i = 0; i < std::size(BoneIDs); i++)
            {
                if (Weights[i] == 0.0f)
                {
                    BoneIDs[i] = BoneID;
                    Weights[i] = Weight;
                    return;
                }
            }

            assert(0);
        }
    };

    void LoadMeshBones(unsigned int MeshIndex, const aiMesh *paiMesh);
    void LoadSingleBone(unsigned int MeshIndex, const aiBone *pBone);
    int GetBoneId(const aiBone *pBone);
    void CalcInterpolatedScaling(aiVector3D &Out, float AnimationTime, const aiNodeAnim *pNodeAnim);
    void CalcInterpolatedRotation(aiQuaternion &Out, float AnimationTime, const aiNodeAnim *pNodeAnim);
    void CalcInterpolatedPosition(aiVector3D &Out, float AnimationTime, const aiNodeAnim *pNodeAnim);
    unsigned int FindScaling(float AnimationTime, const aiNodeAnim *pNodeAnim);
    unsigned int FindRotation(float AnimationTime, const aiNodeAnim *pNodeAnim);
    unsigned int FindPosition(float AnimationTime, const aiNodeAnim *pNodeAnim);
    const aiNodeAnim *FindNodeAnim(const aiAnimation *pAnimation, const std::string &NodeName);
    void ReadNodeHierarchy(float AnimationTime, const aiNode *pNode, const glm::mat4 &ParentTransform);

    enum BUFFER_TYPE
    {
        INDEX_BUFFER = 0,
        POS_VB = 1,
        TEXCOORD_VB = 2,
        NORMAL_VB = 3,
        BONE_VB = 4,
        NUM_BUFFERS = 5
    };

    WorldTrans m_worldTransform;
    GLuint m_VAO = 0;
    GLuint m_Buffers[NUM_BUFFERS] = {0};

    struct BasicMeshEntry
    {
        BasicMeshEntry()
        {
            NumIndices = 0;
            BaseVertex = 0;
            BaseIndex = 0;
            MaterialIndex = INVALID_MATERIAL;
        }

        unsigned int NumIndices;
        unsigned int BaseVertex;
        unsigned int BaseIndex;
        unsigned int MaterialIndex;
    };

    Assimp::Importer Importer;
    const aiScene *pScene = NULL;
    std::vector<BasicMeshEntry> m_Meshes;
    std::vector<Material> m_Materials;

    std::vector<glm::vec3> m_Positions;
    std::vector<glm::vec3> m_Normals;
    std::vector<glm::vec2> m_TexCoords;
    std::vector<unsigned int> m_Indices;
    std::vector<VertexBoneData> m_Bones;

    std::map<std::string, unsigned int> m_BoneNameToIndexMap;

    struct BoneInfo
    {
        glm::mat4 OffsetMatrix;
        glm::mat4 FinalTransformation;

        BoneInfo(const aiMatrix4x4 &Offset)
        {
            OffsetMatrix = AssimpToGLM(Offset);
            FinalTransformation = glm::mat4(0.0f);
        }
    };

    std::vector<BoneInfo> m_BoneInfo;
    glm::mat4 m_GlobalInverseTransform;
};
