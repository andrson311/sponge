#include <stdio.h>
#include "skinned_mesh.h"
#include "meshoptimizer.h"

#define POSITION_LOCATION 0
#define TEX_COORD_LOCATION 1
#define NORMAL_LOCATION 2
#define BONE_ID_LOCATION 3
#define BONE_WEIGHT_LOCATION 4

SkinnedMesh::~SkinnedMesh()
{
    Clear();
}

void SkinnedMesh::ReserveSpace(u_int NumVertices, u_int NumIndices)
{
    BasicMesh::ReserveSpace(NumVertices, NumIndices);
    InitializeRequiredNodeMap(m_pScene->mRootNode);
}

void SkinnedMesh::InitSingleMesh(u_int MeshIndex, const aiMesh *paiMesh)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    SkinnedVertex v;

    for (u_int i = 0; i < paiMesh->mNumVertices; i++)
    {
        const aiVector3D &pPos = paiMesh->mVertices[i];
        v.Position = glm::vec3(pPos.x, pPos.y, pPos.z);

        if (paiMesh->mNormals)
        {
            const aiVector3D &pNormal = paiMesh->mNormals[i];
            v.Normal = glm::vec3(pNormal.x, pNormal.y, pNormal.z);
        }
        else
        {
            aiVector3D Normal(0.0f, 1.0f, 0.0f);
            v.Normal = glm::vec3(Normal.x, Normal.y, Normal.z);
        }

        const aiVector3D &pTexCoord = paiMesh->HasTextureCoords(0)
                                          ? paiMesh->mTextureCoords[0][i]
                                          : Zero3D;
        v.TexCoords = glm::vec2(pTexCoord.x, pTexCoord.y);
        m_skinnedVertices.push_back(v);
    }

    for (u_int i = 0; i < paiMesh->mNumFaces; i++)
    {
        const aiFace &Face = paiMesh->mFaces[i];
        m_Indices.push_back(Face.mIndices[0]);
        m_Indices.push_back(Face.mIndices[1]);
        m_Indices.push_back(Face.mIndices[2]);
    }

    LoadMeshBones(MeshIndex, paiMesh, m_skinnedVertices, m_Meshes[MeshIndex].BaseVertex);
}

void SkinnedMesh::InitSingleMeshOpt(u_int MeshIndex, const aiMesh *paiMesh)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
    SkinnedVertex v;

    std::vector<SkinnedVertex> SkinnedVertices(paiMesh->mNumVertices);

    for (u_int i = 0; i < paiMesh->mNumVertices; i++)
    {
        const aiVector3D &pPos = paiMesh->mVertices[i];
        v.Position = glm::vec3(pPos.x, pPos.y, pPos.z);

        if (paiMesh->mNormals)
        {
            const aiVector3D &pNormal = paiMesh->mNormals[i];
            v.Normal = glm::vec3(pNormal.x, pNormal.y, pNormal.z);
        }
        else
        {
            aiVector3D Normal(0.0f, 1.0f, 0.0f);
            v.Normal = glm::vec3(Normal.x, Normal.y, Normal.z);
        }

        const aiVector3D &pTexCoord = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : Zero3D;
        v.TexCoords = glm::vec2(pTexCoord.x, pTexCoord.y);

        SkinnedVertices[i] = v;
    }

    m_Meshes[MeshIndex].BaseVertex = (u_int)m_skinnedVertices.size();
    m_Meshes[MeshIndex].BaseIndex = (u_int)m_Indices.size();

    int NumIndices = paiMesh->mNumFaces * 3;

    std::vector<u_int> Indices;
    Indices.resize(NumIndices);

    // Populate the index buffer
    for (u_int i = 0; i < paiMesh->mNumFaces; i++)
    {
        const aiFace &Face = paiMesh->mFaces[i];
        Indices[i * 3 + 0] = Face.mIndices[0];
        Indices[i * 3 + 1] = Face.mIndices[1];
        Indices[i * 3 + 2] = Face.mIndices[2];
    }

    LoadMeshBones(MeshIndex, paiMesh, SkinnedVertices, 0);

    OptimizeMesh(MeshIndex, Indices, SkinnedVertices);
}

void SkinnedMesh::OptimizeMesh(int MeshIndex, std::vector<u_int> &Indices, std::vector<SkinnedVertex> &SkinnedVertices)
{
    size_t NumIndices = Indices.size();

    std::vector<unsigned int> remap(NumIndices);
    size_t OptVertexCount = meshopt_generateVertexRemap(remap.data(), Indices.data(), Indices.size(),
                                                        SkinnedVertices.data(), Indices.size(), sizeof(SkinnedVertex));

    std::vector<u_int> OptIndices;
    std::vector<SkinnedVertex> OptVertices;
    OptIndices.resize(NumIndices);
    OptVertices.resize(OptVertexCount);

    meshopt_remapIndexBuffer(OptIndices.data(), Indices.data(), Indices.size(), remap.data());
    meshopt_remapVertexBuffer(OptVertices.data(), SkinnedVertices.data(), SkinnedVertices.size(),
                              sizeof(SkinnedVertex), remap.data());
    meshopt_optimizeVertexCache(OptIndices.data(), OptIndices.data(), Indices.size(), OptVertexCount);
    meshopt_optimizeOverdraw(OptIndices.data(), OptIndices.data(), Indices.size(),
                             &(OptVertices[0].Position.x), OptVertexCount, sizeof(SkinnedVertex), 1.05f);
    meshopt_optimizeVertexFetch(OptVertices.data(), OptIndices.data(), NumIndices, OptVertices.data(),
                                OptVertexCount, sizeof(SkinnedVertex));

    float Threshold = 1.0f;
    size_t TargetIndexCount = (size_t)(NumIndices * Threshold);
    float TargetError = 1.0f;
    std::vector<u_int> IndicesLod(OptIndices.size());
    size_t OptIndexCount = meshopt_simplify(&IndicesLod[0], OptIndices.data(), OptIndices.size(),
                                            &OptVertices[0].Position.x, OptVertexCount, sizeof(SkinnedVertex), TargetIndexCount, TargetError);

    OptIndices = IndicesLod;
    OptIndices.resize(OptIndexCount);

    m_Indices.insert(m_Indices.end(), OptIndices.begin(), OptIndices.end());

    m_skinnedVertices.insert(m_skinnedVertices.end(), OptVertices.begin(), OptVertices.end());

    m_Meshes[MeshIndex].NumIndices = (u_int)OptIndexCount;
}

void SkinnedMesh::LoadMeshBones(u_int MeshIndex, const aiMesh *pMesh,
                                std::vector<SkinnedVertex> &SkinnedVertices, int BaseVertex)
{
    if (pMesh->mNumBones > MAX_BONES)
    {
        printf("The number of bones in the model (%d) is larger than the maximum supported (%d)\n", pMesh->mNumBones, MAX_BONES);
        assert(0);
    }

    for (u_int i = 0; i < pMesh->mNumBones; i++)
    {
        LoadSingleBone(MeshIndex, pMesh->mBones[i], SkinnedVertices, BaseVertex);
    }
}

void SkinnedMesh::LoadSingleBone(u_int MeshIndex, const aiBone *pBone,
                                 std::vector<SkinnedVertex> &SkinnedVertices, int BaseVertex)
{
    int BoneId = GetBoneId(pBone);

    if (BoneId == m_boneInfo.size())
    {
        BoneInfo bi(pBone->mOffsetMatrix);
        m_boneInfo.push_back(bi);
    }

    for (u_int i = 0; i < pBone->mNumWeights; i++)
    {
        const aiVertexWeight &vw = pBone->mWeights[i];
        u_int GlobalVertexID = BaseVertex + pBone->mWeights[i].mVertexId;
        SkinnedVertices[GlobalVertexID].Bones.AddBoneData(BoneId, vw.mWeight);
    }

    MarkRequiredNodesForBone(pBone);
}

void SkinnedMesh::MarkRequiredNodesForBone(const aiBone *pBone)
{
    std::string NodeName(pBone->mName.C_Str());

    const aiNode *pParent = NULL;

    do
    {
        std::map<std::string, NodeInfo>::iterator it = m_requiredNodeMap.find(NodeName);

        if (it == m_requiredNodeMap.end())
        {
            printf("Cannot find bone %s in the hierarchy\n", NodeName.c_str());
            assert(0);
        }

        it->second.isRequired = true;
        pParent = it->second.pNode->mParent;

        if (pParent)
        {
            NodeName = std::string(pParent->mName.C_Str());
        }

    } while (pParent);
}

void SkinnedMesh::InitializeRequiredNodeMap(const aiNode *pNode)
{
    std::string NodeName(pNode->mName.C_Str());

    NodeInfo info(pNode);

    m_requiredNodeMap[NodeName] = info;

    for (u_int i = 0; i < pNode->mNumChildren; i++)
    {
        InitializeRequiredNodeMap(pNode->mChildren[i]);
    }
}

int SkinnedMesh::GetBoneId(const aiBone *pBone)
{
    int BoneIndex = 0;
    std::string BoneName(pBone->mName.C_Str());

    if (m_boneNameToIndexMap.find(BoneName) == m_boneNameToIndexMap.end())
    {
        BoneIndex = (int)m_boneNameToIndexMap.size();
        m_boneNameToIndexMap[BoneName] = BoneIndex;
    }
    else
    {
        BoneIndex = m_boneNameToIndexMap[BoneName];
    }

    return BoneIndex;
}

void SkinnedMesh::PopulateBuffers()
{
    if (IsGLVersionHigher(4, 5))
    {
        PopulateBuffersDSA();
    }
    else
    {
        PopulateBuffersNonDSA();
    }
}

void SkinnedMesh::PopulateBuffersNonDSA()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[VERTEX_BUFFER]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);

    glBufferData(GL_ARRAY_BUFFER, sizeof(m_skinnedVertices[0]) * m_skinnedVertices.size(),
                 &m_skinnedVertices[0], GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_Indices[0]) * m_Indices.size(), &m_Indices[0], GL_STATIC_DRAW);

    size_t NumFloats = 0;

    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (const void *)(NumFloats * sizeof(float)));
    NumFloats += 3;

    glEnableVertexAttribArray(TEX_COORD_LOCATION);
    glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (const void *)(NumFloats * sizeof(float)));
    NumFloats += 2;

    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (const void *)(NumFloats * sizeof(float)));
    NumFloats += 3;

    glEnableVertexAttribArray(BONE_ID_LOCATION);
    glVertexAttribIPointer(BONE_ID_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_INT, sizeof(SkinnedVertex), (const void *)(NumFloats * sizeof(float)));
    NumFloats += MAX_NUM_BONES_PER_VERTEX;

    glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
    glVertexAttribPointer(BONE_WEIGHT_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, sizeof(SkinnedVertex), (const void *)(NumFloats * sizeof(float)));
}

void SkinnedMesh::PopulateBuffersDSA()
{
    glNamedBufferStorage(m_Buffers[VERTEX_BUFFER], sizeof(m_skinnedVertices[0]) * m_skinnedVertices.size(), m_skinnedVertices.data(), 0);
    glNamedBufferStorage(m_Buffers[INDEX_BUFFER], sizeof(m_Indices[0]) * m_Indices.size(), m_Indices.data(), GL_DYNAMIC_STORAGE_BIT);

    glVertexArrayVertexBuffer(m_VAO, 0, m_Buffers[VERTEX_BUFFER], 0, sizeof(SkinnedVertex));
    glVertexArrayElementBuffer(m_VAO, m_Buffers[INDEX_BUFFER]);

    size_t NumFloats = 0;

    glEnableVertexArrayAttrib(m_VAO, POSITION_LOCATION);
    glVertexArrayAttribFormat(m_VAO, POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, (GLuint)(NumFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, POSITION_LOCATION, 0);
    NumFloats += 3;

    glEnableVertexArrayAttrib(m_VAO, TEX_COORD_LOCATION);
    glVertexArrayAttribFormat(m_VAO, TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, (GLuint)(NumFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, TEX_COORD_LOCATION, 0);
    NumFloats += 2;

    glEnableVertexArrayAttrib(m_VAO, NORMAL_LOCATION);
    glVertexArrayAttribFormat(m_VAO, NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, (GLuint)(NumFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, NORMAL_LOCATION, 0);
    NumFloats += 3;

    glEnableVertexArrayAttrib(m_VAO, BONE_ID_LOCATION);
    glVertexArrayAttribIFormat(m_VAO, BONE_ID_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_INT, (GLuint)(NumFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, BONE_ID_LOCATION, 0);

    NumFloats += MAX_NUM_BONES_PER_VERTEX;

    glEnableVertexArrayAttrib(m_VAO, BONE_WEIGHT_LOCATION);
    glVertexArrayAttribFormat(m_VAO, BONE_WEIGHT_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, (GLuint)(NumFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, BONE_WEIGHT_LOCATION, 0);
}

u_int SkinnedMesh::FindPosition(float AnimationTimeTicks, const aiNodeAnim *pNodeAnim)
{
    for (u_int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
    {
        float t = (float)pNodeAnim->mPositionKeys[i + 1].mTime;
        if (AnimationTimeTicks < t)
        {
            return i;
        }
    }

    return pNodeAnim->mNumPositionKeys - 2;
}

void SkinnedMesh::CalcInterpolatedPosition(aiVector3D &Out, float AnimationTimeTicks, const aiNodeAnim *pNodeAnim)
{
    if (pNodeAnim->mNumPositionKeys == 1)
    {
        Out = pNodeAnim->mPositionKeys[0].mValue;
        return;
    }

    u_int PositionIndex = FindPosition(AnimationTimeTicks, pNodeAnim);
    u_int NextPositionIndex = PositionIndex + 1;
    assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);

    float t1 = (float)pNodeAnim->mPositionKeys[PositionIndex].mTime;

    if (t1 > AnimationTimeTicks)
    {
        Out = pNodeAnim->mPositionKeys[PositionIndex].mValue;
    }
    else
    {
        float t2 = (float)pNodeAnim->mPositionKeys[NextPositionIndex].mTime;
        float DeltaTime = t2 - t1;

        float Factor = (AnimationTimeTicks - t1) / DeltaTime;
        Factor = glm::clamp(Factor, 0.0f, 1.0f);
        assert(Factor >= 0.0f && Factor <= 1.0f);

        const aiVector3D &Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
        const aiVector3D &End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
        aiVector3D Delta = End - Start;
        Out = Start + Factor * Delta;
    }
}

u_int SkinnedMesh::FindRotation(float AnimationTimeTicks, const aiNodeAnim *pNodeAnim)
{
    assert(pNodeAnim->mNumRotationKeys > 0);

    for (u_int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
    {
        float t = (float)pNodeAnim->mRotationKeys[i + 1].mTime;

        if (AnimationTimeTicks < t)
        {
            return i;
        }
    }

    return pNodeAnim->mNumRotationKeys - 2;
}

void SkinnedMesh::CalcInterpolatedRotation(aiQuaternion &Out, float AnimationTimeTicks, const aiNodeAnim *pNodeAnim)
{
    if (pNodeAnim->mNumRotationKeys == 1)
    {
        Out = pNodeAnim->mRotationKeys[0].mValue;
        return;
    }

    u_int RotationIndex = FindRotation(AnimationTimeTicks, pNodeAnim);
    u_int NextRotationIndex = RotationIndex + 1;
    assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);

    float t1 = (float)pNodeAnim->mRotationKeys[RotationIndex].mTime;
    if (t1 > AnimationTimeTicks)
    {
        Out = pNodeAnim->mRotationKeys[RotationIndex].mValue;
    }
    else
    {
        float t2 = (float)pNodeAnim->mRotationKeys[NextRotationIndex].mTime;
        float DeltaTime = t2 - t1;

        float Factor = (AnimationTimeTicks - t1) / DeltaTime;
        Factor = glm::clamp(Factor, 0.0f, 1.0f);
        assert(Factor >= 0.0f && Factor <= 1.0f);

        const aiQuaternion &StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
        const aiQuaternion &EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
        aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    }

    Out.Normalize();
}

u_int SkinnedMesh::FindScaling(float AnimationTimeTicks, const aiNodeAnim *pNodeAnim)
{
    assert(pNodeAnim->mNumScalingKeys > 0);

    for (u_int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
    {
        float t = (float)pNodeAnim->mScalingKeys[i + 1].mTime;

        if (AnimationTimeTicks < t)
        {
            return i;
        }
    }

    return pNodeAnim->mNumScalingKeys - 2;
}

void SkinnedMesh::CalcInterpolatedScaling(aiVector3D &Out, float AnimationTimeTicks, const aiNodeAnim *pNodeAnim)
{
    if (pNodeAnim->mNumScalingKeys == 1)
    {
        Out = pNodeAnim->mScalingKeys[0].mValue;
        return;
    }

    u_int ScalingIndex = FindScaling(AnimationTimeTicks, pNodeAnim);
    u_int NextScalingIndex = ScalingIndex + 1;
    assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);

    float t1 = (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime;
    if (t1 > AnimationTimeTicks)
    {
        Out = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
    }
    else
    {
        float t2 = (float)pNodeAnim->mScalingKeys[NextScalingIndex].mTime;
        float DeltaTime = t2 - t1;

        float Factor = (AnimationTimeTicks - t1) / DeltaTime;
        Factor = glm::clamp(Factor, 0.0f, 1.0f);
        assert(Factor >= 0.0f && Factor <= 1.0f);

        const aiVector3D &Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
        const aiVector3D &End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
        aiVector3D Delta = End - Start;

        Out = Start + Factor * Delta;
    }
}

void SkinnedMesh::ReadNodeHierarchy(float AnimationTimeTicks, const aiNode *pNode,
                                    const glm::mat4 &ParentTransform, const aiAnimation &Animation)
{
    std::string NodeName(pNode->mName.data);
    glm::mat4 NodeTransformation = AssimpToGLM(pNode->mTransformation);
    const aiNodeAnim *pNodeAnim = FindNodeAnim(Animation, NodeName);

    if (pNodeAnim)
    {
        LocalTransform Transform;
        CalcLocalTransform(Transform, AnimationTimeTicks, pNodeAnim);

        // aiVector3D Scaling;
        // CalcInterpolatedScaling(Scaling, AnimationTimeTicks, pNodeAnim);
        glm::mat4 ScalingM = glm::scale(glm::mat4(1.0f), glm::vec3(Transform.Scaling.x, Transform.Scaling.y, Transform.Scaling.z));

        // aiQuaternion RotationQ;
        // CalcInterpolatedRotation(RotationQ, AnimationTimeTicks, pNodeAnim);
        glm::quat GLMQuat(Transform.Rotation.w, Transform.Rotation.x, Transform.Rotation.y, Transform.Rotation.z);
        glm::mat4 RotationM = glm::mat4_cast(GLMQuat);

        // aiVector3D Translation;
        // CalcInterpolatedPosition(Translation, AnimationTimeTicks, pNodeAnim);
        glm::mat4 TranslationM = glm::translate(glm::mat4(1.0f), glm::vec3(Transform.Translation.x, Transform.Translation.y, Transform.Translation.z));

        NodeTransformation = TranslationM * RotationM * ScalingM;
    }

    glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;

    if (m_boneNameToIndexMap.find(NodeName) != m_boneNameToIndexMap.end())
    {
        u_int BoneIndex = m_boneNameToIndexMap[NodeName];
        m_boneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_boneInfo[BoneIndex].OffsetMatrix;
    }

    for (u_int i = 0; i < pNode->mNumChildren; i++)
    {
        std::string ChildName(pNode->mChildren[i]->mName.data);
        std::map<std::string, NodeInfo>::iterator it = m_requiredNodeMap.find(ChildName);

        if (it == m_requiredNodeMap.end())
        {
            printf("Child %s cannot be found in the required node map\n", ChildName.c_str());
            assert(0);
        }

        if (it->second.isRequired)
        {
            ReadNodeHierarchy(AnimationTimeTicks, pNode->mChildren[i], GlobalTransformation, Animation);
        }
    }
}

void SkinnedMesh::ReadNodeHierarchyBlended(float StartAnimationTimeTicks, float EndAnimationTimeTicks,
                                           const aiNode *pNode, const glm::mat4 &ParentTransform,
                                           const aiAnimation &StartAnimation, const aiAnimation &EndAnimation, float BlendFactor)
{
    std::string NodeName(pNode->mName.data);

    glm::mat4 NodeTransformation = AssimpToGLM(pNode->mTransformation);

    const aiNodeAnim *pStartNodeAnim = FindNodeAnim(StartAnimation, NodeName);
    LocalTransform StartTransform;

    if (pStartNodeAnim)
    {
        CalcLocalTransform(StartTransform, StartAnimationTimeTicks, pStartNodeAnim);
    }

    const aiNodeAnim *pEndNodeAnim = FindNodeAnim(EndAnimation, NodeName);
    LocalTransform EndTransform;

    if ((pStartNodeAnim && !pEndNodeAnim) || (!pStartNodeAnim && pEndNodeAnim))
    {
        printf("On the node %s there is an animation node for only one of the start/end animations\n", NodeName.c_str());
        exit(0);
    }

    if (pEndNodeAnim)
    {
        CalcLocalTransform(EndTransform, EndAnimationTimeTicks, pEndNodeAnim);
    }

    if (pStartNodeAnim && pEndNodeAnim)
    {
        // interpolate scaling
        aiVector3D BlendedScaling = (1.0f - BlendFactor) * StartTransform.Scaling + EndTransform.Scaling * BlendFactor;
        glm::mat4 ScalingM = glm::scale(glm::mat4(1.0f), glm::vec3(BlendedScaling.x, BlendedScaling.y, BlendedScaling.z));

        // interpolate rotation
        // aiQuaternion BlendedRotation;
        // aiQuaternion::Interpolate(BlendedRotation, StartTransform.Rotation, EndTransform.Rotation, BlendFactor);
        // glm::quat GLMQuat(BlendedRotation.w, BlendedRotation.x, BlendedRotation.y, BlendedRotation.z);
        // glm::mat4 RotationM = glm::mat4_cast(GLMQuat);

        glm::quat StartQuat(StartTransform.Rotation.w, StartTransform.Rotation.x,
                            StartTransform.Rotation.y, StartTransform.Rotation.z);
        glm::quat EndQuat(EndTransform.Rotation.w, EndTransform.Rotation.x,
                          EndTransform.Rotation.y, EndTransform.Rotation.z);
        glm::quat GLMQuat = glm::slerp(StartQuat, EndQuat, BlendFactor);
        glm::mat4 RotationM = glm::mat4_cast(GLMQuat);

        // interpolate translation
        aiVector3D BlendedTranslation = (1.0f - BlendFactor) * StartTransform.Translation + EndTransform.Translation * BlendFactor;
        glm::mat4 TranslationM = glm::translate(glm::mat4(1.0f), glm::vec3(BlendedTranslation.x, BlendedTranslation.y, BlendedTranslation.z));

        // combine all
        NodeTransformation = TranslationM * RotationM * ScalingM;
    }

    glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;

    if (m_boneNameToIndexMap.find(NodeName) != m_boneNameToIndexMap.end())
    {
        u_int BoneIndex = m_boneNameToIndexMap[NodeName];
        m_boneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_boneInfo[BoneIndex].OffsetMatrix;
    }

    for (u_int i = 0; i < pNode->mNumChildren; i++)
    {
        std::string ChildName(pNode->mChildren[i]->mName.data);
        std::map<std::string, NodeInfo>::iterator it = m_requiredNodeMap.find(ChildName);

        if (it == m_requiredNodeMap.end())
        {
            printf("Child %s cannot be found in the required node map\n", ChildName.c_str());
            assert(0);
        }

        if (it->second.isRequired)
        {
            ReadNodeHierarchyBlended(StartAnimationTimeTicks, EndAnimationTimeTicks,
                                     pNode->mChildren[i], GlobalTransformation,
                                     StartAnimation, EndAnimation, BlendFactor);
        }
    }
}

void SkinnedMesh::CalcLocalTransform(LocalTransform &Transform, float AnimationTimeTicks, const aiNodeAnim *pNodeAnim)
{
    CalcInterpolatedScaling(Transform.Scaling, AnimationTimeTicks, pNodeAnim);
    CalcInterpolatedRotation(Transform.Rotation, AnimationTimeTicks, pNodeAnim);
    CalcInterpolatedPosition(Transform.Translation, AnimationTimeTicks, pNodeAnim);
}

void SkinnedMesh::GetBoneTransforms(float TimeInSeconds, std::vector<glm::mat4> &Transforms, u_int AnimationIndex)
{
    if (AnimationIndex >= m_pScene->mNumAnimations)
    {
        printf("Invalid animation index %d, max is %d\n", AnimationIndex, m_pScene->mNumAnimations);
        assert(0);
    }

    glm::mat4 Identity(1.0f);

    float AnimationTimeTicks = CalcAnimationTimeTicks(TimeInSeconds, AnimationIndex);
    const aiAnimation &Animation = *m_pScene->mAnimations[AnimationIndex];

    ReadNodeHierarchy(AnimationTimeTicks, m_pScene->mRootNode, Identity, Animation);
    Transforms.resize(m_boneInfo.size());

    for (u_int i = 0; i < m_boneInfo.size(); i++)
    {
        Transforms[i] = m_boneInfo[i].FinalTransformation;
    }
}

void SkinnedMesh::GetBoneTransformsBlended(float TimeInSeconds, std::vector<glm::mat4> &BlendedTransforms,
                                           u_int StartAnimIndex, u_int EndAnimIndex, float BlendFactor)
{
    if (StartAnimIndex >= m_pScene->mNumAnimations)
    {
        printf("Invalid start animation index %d, max is %d\n", StartAnimIndex, m_pScene->mNumAnimations);
        assert(0);
    }

    if (EndAnimIndex >= m_pScene->mNumAnimations)
    {
        printf("Invalid end animation index %d, max is %d\n", EndAnimIndex, m_pScene->mNumAnimations);
        assert(0);
    }

    if ((BlendFactor < 0.0f) || (BlendFactor > 1.0f))
    {
        printf("Invalid blend factor %f\n", BlendFactor);
        assert(0);
    }

    float StartAnimationTimeTicks = CalcAnimationTimeTicks(TimeInSeconds, StartAnimIndex);
    float EndAnimationTimeTicks = CalcAnimationTimeTicks(TimeInSeconds, EndAnimIndex);

    const aiAnimation &StartAnimation = *m_pScene->mAnimations[StartAnimIndex];
    const aiAnimation &EndAnimation = *m_pScene->mAnimations[EndAnimIndex];

    glm::mat4 Identity(1.0f);
    ReadNodeHierarchyBlended(StartAnimationTimeTicks, EndAnimationTimeTicks,
                             m_pScene->mRootNode, Identity,
                             StartAnimation, EndAnimation, BlendFactor);

    BlendedTransforms.resize(m_boneInfo.size());

    for (u_int i = 0; i < m_boneInfo.size(); i++)
    {
        BlendedTransforms[i] = m_boneInfo[i].FinalTransformation;
    }
}

float SkinnedMesh::CalcAnimationTimeTicks(float TimeInSeconds, u_int AnimationIndex)
{
    float TicksPerSecond = (float)(m_pScene->mAnimations[AnimationIndex]->mTicksPerSecond != 0)
                               ? m_pScene->mAnimations[AnimationIndex]->mTicksPerSecond
                               : 25.0f;

    float TimeInTicks = TimeInSeconds * TicksPerSecond;

    double Duration = 0.0;
    double fraction = modf((double)m_pScene->mAnimations[AnimationIndex]->mDuration, &Duration);
    float AnimationTimeTicks = fmod(TimeInTicks, Duration);
    return AnimationTimeTicks;
}

const aiNodeAnim *SkinnedMesh::FindNodeAnim(const aiAnimation &Animation, const std::string &NodeName)
{
    for (u_int i = 0; i < Animation.mNumChannels; i++)
    {
        const aiNodeAnim *pNodeAnim = Animation.mChannels[i];
        if (std::string(pNodeAnim->mNodeName.data) == NodeName)
        {
            return pNodeAnim;
        }
    }

    return NULL;
}
