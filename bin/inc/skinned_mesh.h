#pragma once

#include <cassert>
#include <map>
#include <vector>
#include <string>
#include <GL/glew.h>

#include "mesh_common.h"
#include "basic_mesh.h"
#include "texture.h"
#include "world_transform.h"
#include "material.h"
#include "util.h"

class SkinnedMesh : public BasicMesh
{
public:
    SkinnedMesh() {}
    ~SkinnedMesh();

    u_int NumBones() const
    {
        return (u_int)m_boneNameToIndexMap.size();
    }

    void GetBoneTransforms(float AnimationTimeSec, std::vector<glm::mat4> &Transforms, u_int AnimationIndex = 0);
    void GetBoneTransformsBlended(float AnimationTimeSec,
                                  std::vector<glm::mat4> &Transforms, u_int StartAnimIndex, u_int EndAnimIndex, float BlendFactor);

private:
#define MAX_NUM_BONES_PER_VERTEX 4

    virtual void ReserveSpace(u_int NumVertices, u_int NumIndices);
    virtual void InitSingleMesh(u_int MeshIndex, const aiMesh *paiMesh);

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

            if (index == MAX_NUM_BONES_PER_VERTEX)
            {
                return;
                assert(0);
            }

            BoneIDs[index] = BoneID;
            Weights[index] = Weight;

            index++;
        }
    };

    struct SkinnedVertex
    {
        glm::vec3 Position;
        glm::vec2 TexCoords;
        glm::vec3 Normal;
        VertexBoneData Bones;
    };

    // virtual void InitSingleMeshOpt(u_int MeshIndex, const aiMesh *paiMesh);
    // void OptimizeMesh(int MeshIndex, std::vector<u_int> &Indices, std::vector<SkinnedVertex> &Vertices);

    virtual void PopulateBuffers();
    void PopulateBuffersNonDSA();
    // void PopulateBuffersDSA();

    void LoadMeshBones(u_int MeshIndex, const aiMesh *paiMesh, std::vector<SkinnedVertex> &SkinnedVertices, int BaseVertex);
    void LoadSingleBone(u_int MeshIndex, const aiBone *pBone, std::vector<SkinnedVertex> &SkinnedVertices, int BaseVertex);
    int GetBoneId(const aiBone *pBone);

    void CalcInterpolatedScaling(aiVector3D &Out, float AnimationTime, const aiNodeAnim *pNodeAnim);
    void CalcInterpolatedRotation(aiQuaternion &Out, float AnimationTime, const aiNodeAnim *pNodeAnim);
    void CalcInterpolatedPosition(aiVector3D &Out, float AnimationTime, const aiNodeAnim *pNodeAnim);

    u_int FindScaling(float AnimationTime, const aiNodeAnim *pNodeAnim);
    u_int FindRotation(float AnimationTime, const aiNodeAnim *pNodeAnim);
    u_int FindPosition(float AnimationTime, const aiNodeAnim *pNodeAnim);

    const aiNodeAnim *FindNodeAnim(const aiAnimation &Animation, const std::string &NodeName);

    void ReadNodeHierarchy(float AnimationTime,
                           const aiNode *pNode, const glm::mat4 &ParentTransform, const aiAnimation &Animation);
    void ReadNodeHierarchyBlended(float StartAnimationTimeTicks, float EndAnimationTimeTicks,
                                  const aiNode *pNode, const glm::mat4 &ParentTransform,
                                  const aiAnimation &StartAnimation, const aiAnimation &EndAnimation, float BlendFactor);
    void MarkRequiredNodesForBone(const aiBone *pBone);
    void InitializeRequiredNodeMap(const aiNode *pNode);
    float CalcAnimationTimeTicks(float TimeInSeconds, u_int AnimationIndex);

    struct LocalTransform
    {
        aiVector3D Scaling;
        aiQuaternion Rotation;
        aiVector3D Translation;
    };

    void CalcLocalTransform(LocalTransform &Transform, float AnimationTimeTicks, const aiNodeAnim *pNodeAnim);

    std::vector<SkinnedVertex> m_skinnedVertices;

    GLuint m_boneBuffer = 0;

    std::map<std::string, u_int> m_boneNameToIndexMap;

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
