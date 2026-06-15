#include <stdio.h>
#include "../inc/mesh_skinned.h"

#define POSITION_LOCATION 0
#define TEX_COORD_LOCATION 1
#define NORMAL_LOCATION 2
#define BONE_ID_LOCATION 3
#define BONE_WEIGHT_LOCATION 4

MeshSkinned::~MeshSkinned()
{
    Clear();
}

void MeshSkinned::Clear()
{
    if (m_Buffers[0] != 0)
    {
        glDeleteBuffers(std::size(m_Buffers), m_Buffers);
    }

    if (m_VAO != 0)
    {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
}

bool MeshSkinned::LoadMesh(const std::string &Filename)
{
    Clear();

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    glGenBuffers(std::size(m_Buffers), m_Buffers);

    bool Ret = false;

    pScene = Importer.ReadFile(Filename.c_str(), ASSIMP_LOAD_FLAGS);

    if (pScene)
    {
        m_GlobalInverseTransform = AssimpToGLM(pScene->mRootNode->mTransformation);
        m_GlobalInverseTransform = glm::inverse(m_GlobalInverseTransform);
        Ret = InitFromScene(pScene, Filename);

        if (pScene->mNumAnimations == 0)
        {
            printf("Animation: none found in '%s'\n", Filename.c_str());
        }
        else
        {
            printf("Animation: found %d animation(s) in '%s'\n", pScene->mNumAnimations, Filename.c_str());
            for (u_int i = 0; i < pScene->mNumAnimations; i++)
            {
                const aiAnimation *anim = pScene->mAnimations[i];
                printf("  [%d] name='%s' duration=%.2f ticks ticksPerSec=%.2f channels=%d\n",
                       i,
                       anim->mName.C_Str(),
                       anim->mDuration,
                       anim->mTicksPerSecond,
                       anim->mNumChannels);
            }
        }
    }
    else
    {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
    }

    glBindVertexArray(0);
    return Ret;
}

bool MeshSkinned::InitFromScene(const aiScene *pScene, const std::string &Filename)
{
    m_Meshes.resize(pScene->mNumMeshes);
    m_Materials.resize(pScene->mNumMaterials);

    u_int NumVertices = 0;
    u_int NumIndices = 0;

    CountVerticesAndIndices(pScene, NumVertices, NumIndices);
    ReserveSpace(NumVertices, NumIndices);
    InitAllMeshes(pScene);

    if (!InitMaterials(pScene, Filename))
    {
        return false;
    }

    PopulateBuffers();

    return true;
}

void MeshSkinned::CountVerticesAndIndices(const aiScene *pScene, u_int &NumVertices, u_int &NumIndices)
{
    for (u_int i = 0; i < m_Meshes.size(); i++)
    {
        m_Meshes[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
        m_Meshes[i].NumIndices = pScene->mMeshes[i]->mNumFaces * 3;
        m_Meshes[i].BaseVertex = NumVertices;
        m_Meshes[i].BaseIndex = NumIndices;

        NumVertices += pScene->mMeshes[i]->mNumVertices;
        NumIndices += m_Meshes[i].NumIndices;
    }
}

void MeshSkinned::ReserveSpace(u_int NumVertices, u_int NumIndices)
{
    m_Positions.reserve(NumVertices);
    m_Normals.reserve(NumVertices);
    m_TexCoords.reserve(NumVertices);
    m_Indices.reserve(NumIndices);
    m_Bones.resize(NumVertices);
}

void MeshSkinned::InitAllMeshes(const aiScene *pScene)
{
    for (u_int i = 0; i < m_Meshes.size(); i++)
    {
        InitSingleMesh(i, pScene->mMeshes[i]);
    }
}

void MeshSkinned::InitSingleMesh(u_int MeshIndex, const aiMesh *paiMesh)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    for (u_int i = 0; i < paiMesh->mNumVertices; i++)
    {
        const aiVector3D &pPos = paiMesh->mVertices[i];
        m_Positions.push_back(glm::vec3(pPos.x, pPos.y, pPos.z));

        if (paiMesh->mNormals)
        {
            const aiVector3D &pNormal = paiMesh->mNormals[i];
            m_Normals.push_back(glm::vec3(pNormal.x, pNormal.y, pNormal.z));
        }
        else
        {
            m_Normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        }

        const aiVector3D &pTexCoord = paiMesh->HasTextureCoords(0)
                                          ? paiMesh->mTextureCoords[0][i]
                                          : Zero3D;

        m_TexCoords.push_back(glm::vec2(pTexCoord.x, pTexCoord.y));
    }

    LoadMeshBones(MeshIndex, paiMesh);

    for (u_int i = 0; i < paiMesh->mNumFaces; i++)
    {
        const aiFace &pFace = paiMesh->mFaces[i];
        m_Indices.push_back(pFace.mIndices[0]);
        m_Indices.push_back(pFace.mIndices[1]);
        m_Indices.push_back(pFace.mIndices[2]);
    }
}

void MeshSkinned::LoadMeshBones(u_int MeshIndex, const aiMesh *pMesh)
{
    for (u_int i = 0; i < pMesh->mNumBones; i++)
    {
        LoadSingleBone(MeshIndex, pMesh->mBones[i]);
    }
}

void MeshSkinned::LoadSingleBone(u_int MeshIndex, const aiBone *pBone)
{
    int BoneId = GetBoneId(pBone);

    if (BoneId == m_BoneInfo.size())
    {
        BoneInfo bi(pBone->mOffsetMatrix);
        m_BoneInfo.push_back(bi);
    }

    for (u_int i = 0; i < pBone->mNumWeights; i++)
    {
        const aiVertexWeight &vw = pBone->mWeights[i];
        u_int GlobalVertexID = m_Meshes[MeshIndex].BaseVertex + vw.mVertexId;
        m_Bones[GlobalVertexID].AddBoneData(BoneId, vw.mWeight);
    }
}

int MeshSkinned::GetBoneId(const aiBone *pBone)
{
    int BoneIndex = 0;
    std::string BoneName(pBone->mName.C_Str());

    if (m_BoneNameToIndexMap.find(BoneName) == m_BoneNameToIndexMap.end())
    {
        BoneIndex = (int)m_BoneNameToIndexMap.size();
        m_BoneNameToIndexMap[BoneName] = BoneIndex;
    }
    else
    {
        BoneIndex = m_BoneNameToIndexMap[BoneName];
    }

    return BoneIndex;
}

bool MeshSkinned::InitMaterials(const aiScene *pScene, const std::string &Filename)
{
    std::string Dir = GetDirFromFilename(Filename);

    printf("Num materials: %d\n", pScene->mNumMaterials);

    for (u_int i = 0; i < pScene->mNumMaterials; i++)
    {
        const aiMaterial *pMaterial = pScene->mMaterials[i];
        LoadTextures(Dir, pMaterial, i);
        LoadColors(pMaterial, i);
    }

    return true;
}

void MeshSkinned::LoadTextures(const std::string &Dir, const aiMaterial *pMaterial, int index)
{
    LoadDiffuseTexture(Dir, pMaterial, index);
    LoadSpecularTexture(Dir, pMaterial, index);
}

void MeshSkinned::LoadDiffuseTexture(const std::string &Dir, const aiMaterial *pMaterial, int index)
{
    m_Materials[index].pDiffuse = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        aiString Path;
        if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path) == AI_SUCCESS)
        {
            const aiTexture *paiTexture = pScene->GetEmbeddedTexture(Path.C_Str());
            if (paiTexture)
            {
                printf("Embedded diffuse texture type '%s'\n", paiTexture->achFormatHint);
                m_Materials[index].pDiffuse = new Texture(GL_TEXTURE_2D);
                m_Materials[index].pDiffuse->Load(paiTexture->mWidth, paiTexture->pcData, true);
            }
            else
            {
                std::string FullPath = GetFullPath(Dir, Path);
                m_Materials[index].pDiffuse = new Texture(GL_TEXTURE_2D, FullPath);

                if (!m_Materials[index].pDiffuse->Load())
                {
                    printf("Error loading diffuse texture '%s'\n", FullPath.c_str());
                    exit(1);
                }
                else
                {
                    printf("Loaded diffuse texture '%s'\n", FullPath.c_str());
                }
            }
        }
    }
}

void MeshSkinned::LoadSpecularTexture(const std::string &Dir, const aiMaterial *pMaterial, int index)
{
    m_Materials[index].pSpecular = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0)
    {
        aiString Path;
        if (pMaterial->GetTexture(aiTextureType_SHININESS, 0, &Path) == AI_SUCCESS)
        {
            const aiTexture *paiTexture = pScene->GetEmbeddedTexture(Path.C_Str());
            if (paiTexture)
            {
                printf("Embedded specular texture type '%s'\n", paiTexture->achFormatHint);
                m_Materials[index].pSpecular = new Texture(GL_TEXTURE_2D);
                m_Materials[index].pSpecular->Load(paiTexture->mWidth, paiTexture->pcData, false);
            }
            else
            {
                std::string FullPath = GetFullPath(Dir, Path);
                m_Materials[index].pSpecular = new Texture(GL_TEXTURE_2D, FullPath);

                if (!m_Materials[index].pSpecular->Load())
                {
                    printf("Error loading specular texture '%s'\n", FullPath.c_str());
                    exit(1);
                }
                else
                {
                    printf("Loaded specular texture '%s'\n", FullPath.c_str());
                }
            }
        }
    }
}

void MeshSkinned::LoadColors(const aiMaterial *pMaterial, int index)
{
    int ShadingModel = 0;

    if (pMaterial->Get(AI_MATKEY_SHADING_MODEL, ShadingModel) == AI_SUCCESS)
    {
        printf("Shading model %d\n", ShadingModel);
    }

    aiColor4D AmbientColor(0.0f, 0.0f, 0.0f, 0.0f);
    if (pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, AmbientColor) == AI_SUCCESS)
    {
        printf("Loaded ambient color [%f %f %f]\n", AmbientColor.r, AmbientColor.g, AmbientColor.b);
        m_Materials[index].AmbientColor = glm::vec3(AmbientColor.r, AmbientColor.g, AmbientColor.b);
    }
    else
    {
        m_Materials[index].AmbientColor = glm::vec3(1.0f);
    }

    aiColor3D DiffuseColor(0.0f, 0.0f, 0.0f);
    if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, DiffuseColor) == AI_SUCCESS)
    {
        printf("Loaded diffuse color [%f %f %f]\n", DiffuseColor.r, DiffuseColor.g, DiffuseColor.b);
        m_Materials[index].DiffuseColor = glm::vec3(DiffuseColor.r, DiffuseColor.g, DiffuseColor.b);
    }

    aiColor3D SpecularColor(0.0f, 0.0f, 0.0f);
    if (pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, SpecularColor) == AI_SUCCESS)
    {
        printf("Loaded specular color [%f %f %f]\n", SpecularColor.r, SpecularColor.g, SpecularColor.b);
        m_Materials[index].SpecularColor = glm::vec3(SpecularColor.r, SpecularColor.g, SpecularColor.b);
    }
}

void MeshSkinned::PopulateBuffers()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_Positions[0]) * m_Positions.size(), m_Positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[TEXCOORD_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_TexCoords[0]) * m_TexCoords.size(), m_TexCoords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(TEX_COORD_LOCATION);
    glVertexAttribPointer(TEX_COORD_LOCATION, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[NORMAL_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_Normals[0]) * m_Normals.size(), m_Normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[BONE_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m_Bones[0]) * m_Bones.size(), m_Bones.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(BONE_ID_LOCATION);
    glVertexAttribIPointer(BONE_ID_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_INT,
                           sizeof(VertexBoneData), (const GLvoid *)0);
    glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
    glVertexAttribPointer(BONE_WEIGHT_LOCATION, MAX_NUM_BONES_PER_VERTEX, GL_FLOAT, GL_FALSE,
                          sizeof(VertexBoneData), (const GLvoid *)(MAX_NUM_BONES_PER_VERTEX * sizeof(int32_t)));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_Indices[0]) * m_Indices.size(), m_Indices.data(), GL_STATIC_DRAW);
}

void MeshSkinned::Render()
{
    glBindVertexArray(m_VAO);

    for (uint i = 0; i < m_Meshes.size(); i++)
    {
        uint MaterialIndex = m_Meshes[i].MaterialIndex;
        assert(MaterialIndex < m_Materials.size());

        if (m_Materials[MaterialIndex].pDiffuse)
            m_Materials[MaterialIndex].pDiffuse->Bind(COLOR_TEXTURE_UNIT);

        if (m_Materials[MaterialIndex].pSpecular)
            m_Materials[MaterialIndex].pSpecular->Bind(SPECULAR_TEXTURE_UNIT);

        glDrawElementsBaseVertex(GL_TRIANGLES,
                                 m_Meshes[i].NumIndices,
                                 GL_UNSIGNED_INT,
                                 (void *)(sizeof(uint) * m_Meshes[i].BaseIndex),
                                 m_Meshes[i].BaseVertex);
    }

    glBindVertexArray(0);
}

const Material &MeshSkinned::GetMaterial()
{
    for (uint i = 0; i < m_Materials.size(); i++)
    {
        if (m_Materials[i].AmbientColor != glm::vec3(0.0f))
        {
            return m_Materials[i];
        }
    }

    return m_Materials[0];
}

u_int MeshSkinned::FindPosition(float AnimationTimeTicks, const aiNodeAnim *pNodeAnim)
{
    for (u_int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
    {
        float t = (float)pNodeAnim->mPositionKeys[i + 1].mTime;
        if (AnimationTimeTicks < t)
        {
            return i;
        }
    }

    return 0;
}

void MeshSkinned::CalcInterpolatedPosition(aiVector3D &Out, float AnimationTimeTicks, const aiNodeAnim *pNodeAnim)
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
    float t2 = (float)pNodeAnim->mPositionKeys[NextPositionIndex].mTime;
    float DeltaTime = t2 - t1;
    float Factor = (AnimationTimeTicks - t1) / DeltaTime;

    assert(Factor >= 0.0f && Factor <= 1.0f);

    const aiVector3D &Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
    const aiVector3D &End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
    aiVector3D Delta = End - Start;

    Out = Start + Factor * Delta;
}

u_int MeshSkinned::FindRotation(float AnimationTimeTicks, const aiNodeAnim *pNodeAnim)
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

    return 0;
}

void MeshSkinned::CalcInterpolatedRotation(aiQuaternion &Out, float AnimationTimeTicks, const aiNodeAnim *pNodeAnim)
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
    float t2 = (float)pNodeAnim->mRotationKeys[NextRotationIndex].mTime;
    float DeltaTime = t2 - t1;
    float Factor = (AnimationTimeTicks - t1) / DeltaTime;

    assert(Factor >= 0.0f && Factor <= 1.0f);

    const aiQuaternion &StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
    const aiQuaternion &EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    Out.Normalize();
}

u_int MeshSkinned::FindScaling(float AnimationTimeTicks, const aiNodeAnim *pNodeAnim)
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

    return 0;
}

void MeshSkinned::CalcInterpolatedScaling(aiVector3D &Out, float AnimationTimeTicks, const aiNodeAnim *pNodeAnim)
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
    float t2 = (float)pNodeAnim->mScalingKeys[NextScalingIndex].mTime;
    float DeltaTime = t2 - t1;
    float Factor = (AnimationTimeTicks - (float)t1) / DeltaTime;

    //printf("Factor: %f\n", Factor);

    assert(Factor >= 0.0f && Factor <= 1.0f);

    const aiVector3D &Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
    const aiVector3D &End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
    aiVector3D Delta = End - Start;

    Out = Start + Factor * Delta;
}

void MeshSkinned::GetBoneTransforms(float TimeInSeconds, std::vector<glm::mat4> &Transforms)
{
    glm::mat4 Identity(1.0f);

    float TicksPerSecond = (float)(pScene->mAnimations[0]->mTicksPerSecond != 0
                                       ? pScene->mAnimations[0]->mTicksPerSecond
                                       : 25.0f);

    printf("TicksPerSecond: %f\n", TicksPerSecond);

    float TimeInTicks = TimeInSeconds * TicksPerSecond;

    printf("TimeInTicks: %f\n", TimeInTicks);

    float AnimationTimeTicks = fmod(TimeInTicks, (float)pScene->mAnimations[0]->mDuration);

    printf("(float)pScene->mAnimations[1]->mDuration: %f\n", (float)pScene->mAnimations[0]->mDuration);

    printf("AnimationTimeTicks: %f\n", AnimationTimeTicks);

    ReadNodeHierarchy(AnimationTimeTicks, pScene->mRootNode, Identity);
    Transforms.resize(m_BoneInfo.size());

    for (u_int i = 0; i < m_BoneInfo.size(); i++)
    {
        Transforms[i] = m_BoneInfo[i].FinalTransformation;
    }
}

void MeshSkinned::ReadNodeHierarchy(float AnimationTimeTicks, const aiNode *pNode, const glm::mat4 &ParentTransform)
{
    std::string NodeName(pNode->mName.data);
    const aiAnimation *pAnimation = pScene->mAnimations[0];
    glm::mat4 NodeTransformation = AssimpToGLM(pNode->mTransformation);
    const aiNodeAnim *pNodeAnim = FindNodeAnim(pAnimation, NodeName);

    if (pNodeAnim)
    {
        aiVector3D Scaling;
        CalcInterpolatedScaling(Scaling, AnimationTimeTicks, pNodeAnim);
        glm::mat4 ScalingM = glm::scale(glm::mat4(1.0f), glm::vec3(Scaling.x, Scaling.y, Scaling.z));

        aiQuaternion RotationQ;
        CalcInterpolatedRotation(RotationQ, AnimationTimeTicks, pNodeAnim);
        glm::quat GLMQuat(RotationQ.w, RotationQ.x, RotationQ.y, RotationQ.z);
        glm::mat4 RotationM = glm::mat4_cast(GLMQuat);

        aiVector3D Translation;
        CalcInterpolatedPosition(Translation, AnimationTimeTicks, pNodeAnim);
        glm::mat4 TranslationM = glm::translate(glm::mat4(1.0f), glm::vec3(Translation.x, Translation.y, Translation.z));

        NodeTransformation = TranslationM * RotationM * ScalingM;
    }

    glm::mat4 GlobalTransformation = ParentTransform * NodeTransformation;

    if (m_BoneNameToIndexMap.find(NodeName) != m_BoneNameToIndexMap.end())
    {
        u_int BoneIndex = m_BoneNameToIndexMap[NodeName];
        m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].OffsetMatrix;
    }

    for (u_int i = 0; i < pNode->mNumChildren; i++)
    {
        ReadNodeHierarchy(AnimationTimeTicks, pNode->mChildren[i], GlobalTransformation);
    }
}

const aiNodeAnim *MeshSkinned::FindNodeAnim(const aiAnimation *pAnimation, const std::string &NodeName)
{
    for (u_int i = 0; i < pAnimation->mNumChannels; i++)
    {
        const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

        if (std::string(pNodeAnim->mNodeName.data) == NodeName)
        {
            return pNodeAnim;
        }
    }
    
    return NULL;
}