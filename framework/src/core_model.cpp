#include <algorithm>
#include <filesystem>
#include <assimp/GltfMaterial.h>
#include <meshoptimizer.h>

#include "core_rendering_system.h"
#include "core_model.h"

static bool useMeshOptimizer = false;
static bool MissingTextureDetection = false;

void CoreModel::DestroyModel()
{
    for (CoreMaterial &material : m_materials)
    {
        for (Texture *pTexture : material.pTextures)
        {
            if (pTexture)
            {
                DestroyTexture(pTexture);
            }
        }
    }
}

bool CoreModel::IsPBR() const
{
    if (m_materials.size() > 0)
    {
        return m_materials[0].m_isPBR;
    }

    return false;
}

bool CoreModel::LoadAssimpModel(const std::string &filename, const ModelLoadFlags &flags)
{
    m_filename = filename;
    AllocBuffers();

    u_int loadFlags = ASSIMP_LOAD_FLAGS;
    m_pScene = m_importer.ReadFile(filename.c_str(), loadFlags);

    if (m_pScene)
    {
        /*
        aiMatrix4x4 m = m_pScene->mRootNode->mTransformation;
        m_globalInverseTransform = glm::inverse(glm::mat4(
            m.a1, m.b1, m.c1, m.d1,
            m.a2, m.b2, m.c2, m.d2,
            m.a3, m.b3, m.c3, m.d3,
            m.a4, m.b4, m.c4, m.d4));
        */

        m_globalInverseTransform = glm::inverse(AssimpToGLM(m_pScene->mRootNode->mTransformation));

        return InitFromScene(m_pScene, filename);
    }
    else
    {
        printf("Error parsing '%s': '%s'\n", filename.c_str(), m_importer.GetErrorString());
    }

    return false;
}

bool CoreModel::InitFromScene(const aiScene *pScene, const std::string &filename)
{
    if (!InitGeometry(pScene, filename))
    {
        return false;
    }

    InitCameras(pScene);
    InitLights(pScene);

    return true;
}

bool CoreModel::InitGeometry(const aiScene *pScene, const std::string &filename)
{
    m_numAnimations = pScene->mNumAnimations;
    m_meshes.resize(pScene->mNumMeshes);
    m_materials.resize(pScene->mNumMaterials);
    m_meshNameToMeshIndex.clear();
    for (u_int i = 0; i < pScene->mNumMeshes; ++i)
    {
        const std::string name = pScene->mMeshes[i]->mName.C_Str();
        m_meshes[i].Name = name;
        if (!name.empty() && m_meshNameToMeshIndex.find(name) == m_meshNameToMeshIndex.end())
            m_meshNameToMeshIndex.emplace(name, static_cast<int>(i));
    }

    u_int numVertices = 0;
    u_int numIndices = 0;

    CountVerticesAndIndices(pScene, numVertices, numIndices);
    InitBuffers(pScene, numVertices, numIndices);

    if (!InitMaterials(pScene, filename))
    {
        return false;
    }

    CalculateMeshTransformations(pScene);
    InitGeometryPost();

    return glGetError() == 0;
}

void CoreModel::InitBuffers(const aiScene *pScene, u_int numVertices, u_int numIndices)
{
    u_int numAnimations = pScene->mNumAnimations;
    printf("Number of animations: %d\n", numAnimations);

    if (numAnimations > 0)
    {
        std::vector<SkinnedVertex> vertices;
        InitGeometryInternal<SkinnedVertex>(vertices, numVertices, numIndices);
        PopulateBuffersSkinned(vertices);
    }
    else
    {
        std::vector<Vertex> vertices;
        InitGeometryInternal<Vertex>(vertices, numVertices, numIndices);
        PopulateBuffers(vertices);
    }
}

template <typename VertexType>
void CoreModel::InitGeometryInternal(std::vector<VertexType> &vertices,
                                     int numVertices, int numIndices)
{
    ReserveSpace<VertexType>(vertices, numVertices, numIndices);
    InitAllMeshes<VertexType>(m_pScene, vertices);
}

void CoreModel::CountVerticesAndIndices(const aiScene *pScene,
                                        u_int &numVertices, u_int &numIndices)
{
    for (u_int i = 0; i < m_meshes.size(); i++)
    {
        m_meshes[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
        m_meshes[i].ValidFaces = CountValidFaces(*pScene->mMeshes[i]);
        // m_meshes[i].NumIndices = m_meshes[i].ValidFaces * 3;
        // m_meshes[i].NumVertices = pScene->mMeshes[i]->mNumVertices;
        // m_meshes[i].BaseVertex = numVertices;
        // m_meshes[i].BaseIndex = numIndices;

        numVertices += pScene->mMeshes[i]->mNumVertices;
        numIndices += m_meshes[i].NumIndices;
    }
}

u_int CoreModel::CountValidFaces(const aiMesh &mesh)
{
    u_int numValidFaces = 0;

    for (u_int i = 0; i < mesh.mNumFaces; i++)
    {
        if (mesh.mFaces[i].mNumIndices == 3)
        {
            numValidFaces++;
        }
    }

    return numValidFaces;
}

template <typename VertexType>
void CoreModel::ReserveSpace(std::vector<VertexType> &vertices,
                             u_int numVertices, u_int numIndices)
{
    vertices.reserve(numVertices);
    m_indices.reserve(numIndices);

    InitializeRequiredNodeMap(m_pScene->mRootNode);
}

template <typename VertexType>
void CoreModel::InitAllMeshes(const aiScene *pScene, std::vector<VertexType> &vertices)
{
    for (u_int i = 0; i < m_meshes.size(); i++)
    {
        const aiMesh *paiMesh = pScene->mMeshes[i];
        if (useMeshOptimizer)
        {
            InitSingleMeshOpt<VertexType>(vertices, i, paiMesh);
        }
        else
        {
            InitSingleMesh<VertexType>(vertices, i, paiMesh);
        }
    }
}

void CoreModel::CalculateMeshTransformations(const aiScene *pScene)
{
    glm::mat4 transformation = glm::mat4(1.0f);
    TraverseNodeHierarchy(transformation, pScene->mRootNode);
}

void CoreModel::TraverseNodeHierarchy(glm::mat4 parentTransformation, aiNode *pNode)
{
    glm::mat4 nodeTransformation = AssimpToGLM(pNode->mTransformation);
    glm::mat4 combinedTransformation = parentTransformation * nodeTransformation;

    if (pNode->mNumMeshes > 0)
    {
        for (int i = 0; i < (int)pNode->mNumMeshes; i++)
        {
            int meshIndex = pNode->mMeshes[i];
            m_meshes[meshIndex].Transformation = combinedTransformation;
            /*
            std::string nodeName(pNode->mName.C_Str());

            if (m_meshNameToMeshIndex.find(nodeName) == m_meshNameToMeshIndex.end())
            {
                m_meshNameToMeshIndex[nodeName] = meshIndex;
            }
            else
            {
                int oldMeshIndex = m_meshNameToMeshIndex[nodeName];
                if (meshIndex != oldMeshIndex)
                {
                    printf("Warning: node name '%s' already mapped to index %d, new index %d\n",
                           nodeName.c_str(), oldMeshIndex, meshIndex);
                    assert(0);
                }
            }
            */
        }
    }

    for (u_int i = 0; i < pNode->mNumChildren; i++)
    {
        TraverseNodeHierarchy(combinedTransformation, pNode->mChildren[i]);
    }
}

template <typename VertexType>
void CoreModel::InitSingleMesh(std::vector<VertexType> &vertices,
                               u_int meshIndex, const aiMesh *paiMesh)
{
    m_meshes[meshIndex].BaseVertex = (u_int)(vertices.size());
    m_meshes[meshIndex].BaseIndex = (u_int)(m_indices.size());

    u_int addedVertices = 0;
    u_int addedIndices = 0;

    // const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    for (u_int i = 0; i < paiMesh->mNumVertices; i++)
    {
        VertexType v;
        const aiVector3D &pos = paiMesh->mVertices[i];
        v.Position = AssimpToGLM(pos);

        m_minPos.x = std::min(m_minPos.x, v.Position.x);
        m_minPos.y = std::min(m_minPos.y, v.Position.y);
        m_minPos.z = std::min(m_minPos.z, v.Position.z);

        m_maxPos.x = std::max(m_maxPos.x, v.Position.x);
        m_maxPos.y = std::max(m_maxPos.y, v.Position.y);
        m_maxPos.z = std::max(m_maxPos.z, v.Position.z);

        if (paiMesh->mNormals)
        {
            v.Normal = AssimpToGLM(paiMesh->mNormals[i]);
        }
        else
        {
            v.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        if (paiMesh->HasTextureCoords(0))
        {
            v.TexCoords0 = glm::vec2(paiMesh->mTextureCoords[0][i].x, paiMesh->mTextureCoords[0][i].y);
        }
        else
        {
            v.TexCoords0 = glm::vec2(0.0f);
        }

        if (paiMesh->HasTextureCoords(1))
        {
            v.TexCoords1 = glm::vec2(paiMesh->mTextureCoords[1][i].x, paiMesh->mTextureCoords[1][i].y);
        }
        else
        {
            v.TexCoords1 = glm::vec2(0.0f);
        }

        if (m_flipTexCoordV)
        {
            v.TexCoords0.y = 1.0f - v.TexCoords0.y;
            v.TexCoords1.y = 1.0f - v.TexCoords1.y;
        }

        if (paiMesh->mTangents)
        {
            v.Tangent = AssimpToGLM(paiMesh->mTangents[i]);
        }
        else
        {
            v.Tangent = glm::vec3(0.0f);
        }

        if (paiMesh->mBitangents)
        {
            v.Bitangent = AssimpToGLM(paiMesh->mBitangents[i]);
        }
        else
        {
            v.Bitangent = glm::vec3(0.0f);
        }

        if (paiMesh->mColors[0])
        {
            v.Color = AssimpToGLM(*paiMesh->mColors[0]);
        }
        else
        {
            v.Color = glm::vec4(1.0f);
        }

        vertices.push_back(v);
        addedVertices++;
    }

    for (u_int i = 0; i < paiMesh->mNumFaces; i++)
    {
        const aiFace &face = paiMesh->mFaces[i];

        if (face.mNumIndices != 3)
        {
            printf("Warning! face %d had %d indices\n", i, face.mNumIndices);
            continue;
        }

        m_indices.push_back(face.mIndices[0]);
        m_indices.push_back(face.mIndices[1]);
        m_indices.push_back(face.mIndices[2]);
        addedIndices += 3;
    }

    m_meshes[meshIndex].NumVertices = addedVertices;
    m_meshes[meshIndex].NumIndices = addedIndices;

    if constexpr (std::is_same_v<VertexType, SkinnedVertex>)
    {
        LoadMeshBones(vertices, meshIndex, paiMesh);
    }
}

template <typename VertexType>
void CoreModel::InitSingleMeshOpt(std::vector<VertexType> &allVertices,
                                  u_int meshIndex, const aiMesh *paiMesh)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
    std::vector<VertexType> vertices(paiMesh->mNumVertices);

    for (u_int i = 0; i < paiMesh->mNumVertices; i++)
    {
        VertexType v;
        const aiVector3D &pos = paiMesh->mVertices[i];
        v.Position = AssimpToGLM(pos);

        m_minPos.x = std::min(m_minPos.x, v.Position.x);
        m_minPos.y = std::min(m_minPos.y, v.Position.y);
        m_minPos.z = std::min(m_minPos.z, v.Position.z);

        m_maxPos.x = std::max(m_maxPos.x, v.Position.x);
        m_maxPos.y = std::max(m_maxPos.y, v.Position.y);
        m_maxPos.z = std::max(m_maxPos.z, v.Position.z);

        if (paiMesh->mNormals)
        {
            v.Normal = AssimpToGLM(paiMesh->mNormals[i]);
        }
        else
        {
            v.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        const aiVector3D &pTexCoords0 = paiMesh->HasTextureCoords(0) ? paiMesh->mTextureCoords[0][i] : Zero3D;
        v.TexCoords0 = glm::vec2(pTexCoords0.x, pTexCoords0.y);

        const aiVector3D &pTexCoords1 = paiMesh->HasTextureCoords(1) ? paiMesh->mTextureCoords[1][i] : Zero3D;
        v.TexCoords1 = glm::vec2(pTexCoords1.x, pTexCoords1.y);

        if (paiMesh->mTangents)
        {
            v.Tangent = AssimpToGLM(paiMesh->mTangents[i]);
        }
        else
        {
            v.Tangent = glm::vec3(0.0f);
        }

        if (paiMesh->mBitangents)
        {
            v.Bitangent = AssimpToGLM(paiMesh->mBitangents[i]);
        }
        else
        {
            v.Bitangent = glm::vec3(0.0f);
        }

        if (paiMesh->mColors[0])
        {
            v.Color = AssimpToGLM(paiMesh->mColors[0][i]);
        }
        else
        {
            v.Color = glm::vec4(1.0f);
        }

        vertices[i] = v;
    }

    m_meshes[meshIndex].BaseVertex = (u_int)allVertices.size();
    m_meshes[meshIndex].BaseIndex = (u_int)m_indices.size();

    int numIndices = paiMesh->mNumFaces * 3;
    std::vector<u_int> indices;
    indices.reserve(numIndices);

    for (u_int i = 0; i < paiMesh->mNumFaces; i++)
    {
        const aiFace &face = paiMesh->mFaces[i];

        if (face.mNumIndices != 3)
        {
            printf("Warning! face %d had %d indices\n", i, face.mNumIndices);
            continue;
        }

        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }

    if constexpr (std::is_same_v<VertexType, SkinnedVertex>)
    {
        LoadMeshBones(vertices, meshIndex, paiMesh);
    }

    OptimizeMesh(meshIndex, indices, vertices, allVertices);
}

template <typename VertexType>
void CoreModel::OptimizeMesh(int meshIndex, std::vector<u_int> &indices,
                             std::vector<VertexType> &vertices, std::vector<VertexType> &allVertices)
{
    size_t numIndices = indices.size();
    size_t numVertices = vertices.size();
    if (numIndices == 0 || numVertices == 0)
        return;

    std::vector<u_int> remap(numIndices);
    size_t optVertexCount = meshopt_generateVertexRemap(remap.data(), indices.data(), numIndices,
                                                        vertices.data(), numVertices, sizeof(VertexType));
    std::vector<u_int> optIndices(numIndices);
    std::vector<VertexType> optVertices(optVertexCount);

    meshopt_remapIndexBuffer(optIndices.data(), indices.data(), numIndices, remap.data());
    meshopt_remapVertexBuffer(optVertices.data(), vertices.data(), numVertices,
                              sizeof(VertexType), remap.data());
    meshopt_optimizeVertexCache(optIndices.data(), optIndices.data(), numIndices, optVertexCount);
    meshopt_optimizeOverdraw(optIndices.data(), optIndices.data(), numIndices,
                             &(optVertices[0].Position.x), optVertexCount, sizeof(VertexType), 1.05f);
    meshopt_optimizeVertexFetch(optVertices.data(), optIndices.data(), numIndices,
                                optVertices.data(), optVertexCount, sizeof(VertexType));

    m_meshes[meshIndex].NumVertices = (u_int)optVertexCount;

    float threshold = 1.0f;
    if (threshold < 1.0f)
    {
        size_t targetIndexCount = (size_t)(numIndices * threshold);
        float targetError = 0.01f;
        std::vector<u_int> simplifiedIndices(optIndices.size());
        size_t optIndexCount = meshopt_simplify(simplifiedIndices.data(), optIndices.data(), numIndices,
                                                &optVertices[0].Position.x, optVertexCount,
                                                sizeof(VertexType), targetIndexCount, targetError);

        simplifiedIndices.resize(optIndexCount);
        m_indices.insert(m_indices.end(), simplifiedIndices.begin(), simplifiedIndices.end());
        m_meshes[meshIndex].NumIndices = (u_int)optIndexCount;
    }
    else
    {
        m_indices.insert(m_indices.end(), optIndices.begin(), optIndices.end());
        m_meshes[meshIndex].NumIndices = (u_int)numIndices;
    }

    // static int num_indices = 0;
    // num_indices += (int)numIndices;
    // static int opt_indices = 0;
    // opt_indices += (int)optIndexCount;
    // printf("Num indices %d\n", num_indices);
    // printf("Optimized number of indices %d\n", opt_indices);

    allVertices.insert(allVertices.end(), optVertices.begin(), optVertices.end());
}

bool CoreModel::InitMaterials(const aiScene *pScene, const std::string &filename)
{
    std::string dir = GetDirFromFilename(filename);

    for (u_int i = 0; i < pScene->mNumMaterials; i++)
    {
        const aiMaterial *pMaterial = pScene->mMaterials[i];
        SetMaterialType(pMaterial, m_materials[i]);
        LoadTextures(dir, pMaterial, i);
        LoadColors(pMaterial, i);
    }

    return true;
}

const CoreMaterial *CoreModel::GetMaterialForMesh(int meshIndex) const
{
    if (meshIndex >= m_meshes.size())
    {
        printf("Invalid mesh index %d, num meshes %d\n", meshIndex, (int)m_meshes.size());
        exit(1);
    }

    int materialIndex = m_meshes[meshIndex].MaterialIndex;
    const CoreMaterial *pMaterial = NULL;

    if ((materialIndex >= 0) && (materialIndex < m_materials.size()))
    {
        pMaterial = &m_materials[materialIndex];
    }

    return pMaterial;
}
int CoreModel::GetMeshIndex(const std::string &name)
{
    const auto it = m_meshNameToMeshIndex.find(name);
    return it != m_meshNameToMeshIndex.end() ? it->second : -1;
}

void CoreModel::GetVertexSizesInBytes(size_t &vertexSize, size_t &skinnedVertexSize)
{
    vertexSize = sizeof(Vertex);
    skinnedVertexSize = sizeof(SkinnedVertex);
}

void CoreModel::LoadTextures(const std::string &dir, const aiMaterial *pMaterial, int index)
{
    LoadDiffuseTexture(dir, pMaterial, index);
    LoadSpecularTexture(dir, pMaterial, index);
    LoadNormalTexture(dir, pMaterial, index);
    LoadMetalnessTexture(dir, pMaterial, index);
    LoadEmissiveTexture(dir, pMaterial, index);
    LoadEmissionColorTexture(dir, pMaterial, index);
    LoadNormalCameraTexture(dir, pMaterial, index);
    LoadRoughnessTexture(dir, pMaterial, index);
    LoadAmbientOcclusionTexture(dir, pMaterial, index);
    LoadClearCoatTexture(dir, pMaterial, index);
    LoadClearCoatRoughnessTexture(dir, pMaterial, index);
    LoadClearCoatNormalTexture(dir, pMaterial, index);
}

void CoreModel::LoadDiffuseTexture(const std::string &dir, const aiMaterial *pMaterial, int index)
{
    if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        LoadTexture(dir, pMaterial, index, aiTextureType_DIFFUSE, 0, TEX_TYPE_BASE);
    }
    else
    {
        aiString path;

        if (MissingTextureDetection)
        {
            path = ("no_texture.png");
        }
        else
        {
            path = ("white.png");
        }

        std::string defaultTextureDir("res/textures/");

        LoadTextureFromFile(defaultTextureDir, path, index, TEX_TYPE_BASE, false);
    }
}

void CoreModel::LoadSpecularTexture(const std::string &dir, const aiMaterial *pMaterial, int index)
{
    LoadTexture(dir, pMaterial, index,
                aiTextureType_SHININESS, 0, TEX_TYPE_SPECULAR);
}

void CoreModel::LoadNormalTexture(const std::string &dir, const aiMaterial *pMaterial, int index)
{
    LoadTexture(dir, pMaterial, index,
                aiTextureType_NORMALS, 0, TEX_TYPE_NORMAL);
}

void CoreModel::LoadMetalnessTexture(const std::string &dir, const aiMaterial *pMaterial, int index)
{
    LoadTexture(dir, pMaterial, index,
                aiTextureType_METALNESS, 0, TEX_TYPE_METALNESS);
}

void CoreModel::LoadEmissiveTexture(const std::string &dir, const aiMaterial *pMaterial, int index)
{
    LoadTexture(dir, pMaterial, index,
                aiTextureType_EMISSIVE, 0, TEX_TYPE_EMISSIVE);
}

void CoreModel::LoadEmissionColorTexture(const std::string &dir, const aiMaterial *pMaterial, int index)
{
    LoadTexture(dir, pMaterial, index,
                aiTextureType_EMISSION_COLOR, 0, TEX_TYPE_EMISSION_COLOR);
}

void CoreModel::LoadNormalCameraTexture(const std::string &dir, const aiMaterial *pMaterial, int index)
{
    LoadTexture(dir, pMaterial, index,
                aiTextureType_NORMAL_CAMERA, 0, TEX_TYPE_NORMAL_CAMERA);
}

void CoreModel::LoadRoughnessTexture(const std::string &dir, const aiMaterial *pMaterial, int index)
{
    LoadTexture(dir, pMaterial, index,
                aiTextureType_DIFFUSE_ROUGHNESS, 0, TEX_TYPE_ROUGHNESS);
}

void CoreModel::LoadAmbientOcclusionTexture(const std::string &dir, const aiMaterial *pMaterial, int index)
{
    LoadTexture(dir, pMaterial, index,
                aiTextureType_AMBIENT_OCCLUSION, 0, TEX_TYPE_AMBIENT_OCCLUSION);
}

void CoreModel::LoadClearCoatTexture(const std::string &dir, const aiMaterial *pMaterial, int index)
{
    LoadTexture(dir, pMaterial, index,
                aiTextureType_CLEARCOAT, 0, TEX_TYPE_CLEARCOAT);
}

void CoreModel::LoadClearCoatRoughnessTexture(const std::string &dir, const aiMaterial *pMaterial, int index)
{
    LoadTexture(dir, pMaterial, index,
                aiTextureType_CLEARCOAT, 1, TEX_TYPE_CLEARCOAT_ROUGHNESS);
}

void CoreModel::LoadClearCoatNormalTexture(const std::string &dir, const aiMaterial *pMaterial, int index)
{
    LoadTexture(dir, pMaterial, index,
                aiTextureType_CLEARCOAT, 2, TEX_TYPE_CLEARCOAT_NORMAL);
}

void CoreModel::LoadTexture(const std::string &dir, const aiMaterial *pMaterial, int index,
                            aiTextureType assimpType, int assimpTexIndex, TEXTURE_TYPE texType)
{
    assert(texType < TEX_TYPE_NUM);
    m_materials[index].pTextures[texType] = NULL;

    if (pMaterial->GetTextureCount(assimpType) > 0)
    {
        aiString path;

        if (pMaterial->GetTexture(assimpType, assimpTexIndex, &path) == AI_SUCCESS)
        {
            const aiTexture *paiTexture = m_pScene->GetEmbeddedTexture(path.C_Str());

            bool isSRGB = (texType == TEX_TYPE_BASE);

            if (paiTexture)
            {
                LoadTextureEmbedded(paiTexture, index, texType, isSRGB);
            }
            else
            {
                LoadTextureFromFile(dir, path, index, texType, isSRGB);
            }
        }
    }
}

void CoreModel::LoadTextureEmbedded(const aiTexture *paiTexture,
                                    int index, TEXTURE_TYPE texType, bool isSRGB)
{
    printf("Loaded embedded texture type '%s'\n", paiTexture->achFormatHint);
    m_materials[index].pTextures[texType] = AllocTexture2D();
    assert(paiTexture->mHeight == 0);
    int buffer_size = paiTexture->mWidth;
    m_materials[index].pTextures[texType]->Load(buffer_size, paiTexture->pcData, isSRGB);
}

void CoreModel::LoadTextureFromFile(const std::string &dir, const aiString &path,
                                    int index, TEXTURE_TYPE texType, bool isSRGB)
{
    std::string fullPath = GetFullPath(dir, path);

    m_materials[index].pTextures[texType] = AllocTexture2D();
    m_materials[index].pTextures[texType]->Load(fullPath.c_str(), isSRGB);

    printf("Loaded texture type %d from '%s'\n", texType, fullPath.c_str());
}

void CoreModel::LoadColors(const aiMaterial *pMaterial, int index)
{
    CoreMaterial &material = m_materials[index];
    material.m_name = pMaterial->GetName().C_Str();

    LoadColor(pMaterial, material.AmbientColor, AI_MATKEY_COLOR_AMBIENT, "ambient color");

    aiColor4D emissiveColor(0.0f, 0.0f, 0.0f, 0.0f);

    if (pMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor) == AI_SUCCESS)
    {
        printf("Loaded emissive color [%f %f %f]\n",
               emissiveColor.r, emissiveColor.g, emissiveColor.b);

        material.AmbientColor.r += emissiveColor.r;
        material.AmbientColor.g += emissiveColor.g;
        material.AmbientColor.b += emissiveColor.b;
        material.AmbientColor.a += emissiveColor.a;
        material.AmbientColor.a = std::min(material.AmbientColor.a, 1.0f);
    }
    else
    {
        material.AmbientColor = glm::vec4(1.0f);
    }

    LoadColor(pMaterial, material.DiffuseColor, AI_MATKEY_COLOR_DIFFUSE, "diffuse color");
    LoadColor(pMaterial, material.SpecularColor, AI_MATKEY_COLOR_SPECULAR, "specular color");
    LoadColor(pMaterial, material.BaseColor, AI_MATKEY_BASE_COLOR, "base color");
    LoadColor(pMaterial, material.EmissiveColor, AI_MATKEY_COLOR_EMISSIVE, "emissive color");

    float emissiveStrength = 1.0f;
    if (pMaterial->Get(AI_MATKEY_EMISSIVE_INTENSITY, emissiveStrength) == AI_SUCCESS)
    {
        material.EmissiveColor = material.EmissiveColor * glm::vec4(emissiveStrength,
                                                                    emissiveStrength,
                                                                    emissiveStrength,
                                                                    1.0f);
    }

    if (material.m_materialType && MaterialType_MetallicRoughness)
    {
        float metallicFactor;

        if (pMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metallicFactor) == AI_SUCCESS)
        {
            material.MetallicRoughnessNormalOcclusion.x = metallicFactor;
        }

        float roughnessFactor;

        if (pMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughnessFactor) == AI_SUCCESS)
        {
            material.MetallicRoughnessNormalOcclusion.y = roughnessFactor;
        }
    }

    float normalScale;
    if (pMaterial->Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), normalScale) == AI_SUCCESS)
    {
        material.MetallicRoughnessNormalOcclusion.z = normalScale;
    }

    float occlusionStrength;
    if (pMaterial->Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_LIGHTMAP, 0), occlusionStrength) == AI_SUCCESS)
    {
        material.MetallicRoughnessNormalOcclusion.w = occlusionStrength;
    }

    float alphaCutoff = 0.0f;
    if (pMaterial->Get(AI_MATKEY_GLTF_ALPHACUTOFF, alphaCutoff) == AI_SUCCESS)
    {
        material.EmissiveColor.a = alphaCutoff;
    }
    else
    {
        material.EmissiveColor.a = 0.5f;
    }

    float opaquenessThreshold = 0.05f;
    float opacity = 1.0f;
    if (pMaterial->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS)
    {
        material.m_transparencyFactor = glm::clamp(1.0f - opacity, 0.0f, 1.0f);
        if (material.m_transparencyFactor >= 1.0f - opaquenessThreshold)
        {
            material.m_transparencyFactor = 0.0f;
        }
    }

    aiColor4D transparentColor;
    if (pMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, transparentColor) == AI_SUCCESS)
    {
        float opacity = std::max(std::max(transparentColor.r, transparentColor.g), transparentColor.b);
        material.m_transparencyFactor = glm::clamp(opacity, 0.0f, 1.0f);
        if (material.m_transparencyFactor >= 1.0f - opaquenessThreshold)
        {
            material.m_transparencyFactor = 0.0f;
        }

        material.m_alphaTest = 0.5f;
    }

    ProcessClearCoat(index, pMaterial, material);

    if ((material.AmbientColor.r == 0.0f) && (material.AmbientColor.g == 0.0f) && (material.AmbientColor.b == 0.0f))
    {
        material.AmbientColor = material.DiffuseColor;
    }
}

void CoreModel::ProcessClearCoat(int index, const aiMaterial *pMaterial, CoreMaterial &material)
{
    bool useClearCoat = m_materials[index].pTextures[TEX_TYPE_CLEARCOAT] ||
                        m_materials[index].pTextures[TEX_TYPE_CLEARCOAT_NORMAL] ||
                        m_materials[index].pTextures[TEX_TYPE_CLEARCOAT_ROUGHNESS];

    float clearCoatFactor = 0.0f;
    if (pMaterial->Get(AI_MATKEY_CLEARCOAT_FACTOR, clearCoatFactor) == AI_SUCCESS)
    {
        material.ClearCoatTransmissionThickness.x = clearCoatFactor;
        useClearCoat = true;
    }

    float clearCoatRoughnessFactor = 0.0f;
    if (pMaterial->Get(AI_MATKEY_CLEARCOAT_ROUGHNESS_FACTOR, clearCoatRoughnessFactor) == AI_SUCCESS)
    {
        material.ClearCoatTransmissionThickness.y = clearCoatRoughnessFactor;
        useClearCoat = true;
    }

    if (useClearCoat)
    {
        material.m_materialType |= MaterialType_ClearCoat;
    }
}

void CoreModel::LoadColor(const aiMaterial *pMaterial, glm::vec4 &color,
                          const char *paiMatKey, int aiMatType, int aiMatIdx, const char *pName)
{
    aiColor4D aiColor(0.0f, 0.0f, 0.0f, 0.0f);

    if (pMaterial->Get(paiMatKey, aiMatType, aiMatIdx, aiColor) == AI_SUCCESS)
    {
        printf("Loaded %s color [%f %f %f]\n", pName, aiColor.r, aiColor.g, aiColor.b);
        color.r = aiColor.r;
        color.g = aiColor.g;
        color.b = aiColor.b;
        color.a = std::min(aiColor.a, 1.0f);
    }
    else
    {
        color = glm::vec4(1.0f);
    }
}

bool CoreModel::GetFullTransformation(const aiNode *pRootNode,
                                      const char *pName, glm::mat4 &transformation)
{
    transformation = glm::mat4(1.0f);

    const aiNode *pNode = pRootNode->FindNode(pName);

    if (!pNode)
    {
        printf("Warning! Cannot find a node for '%s'\n", pName);
        return false;
    }

    while (pNode)
    {
        glm::mat4 curTransformation = AssimpToGLM(pNode->mTransformation);
        transformation = curTransformation * transformation;
        pNode = pNode->mParent;
    }

    return true;
}

void CoreModel::InitCameras(const aiScene *pScene)
{
    printf("Loading %d cameras\n", pScene->mNumCameras);
    m_cameras.resize(pScene->mNumCameras);

    for (u_int i = 0; i < pScene->mNumCameras; i++)
    {
        InitSingleCamera(i, pScene);
    }
}

void CoreModel::InitSingleCamera(int index, const aiScene *pScene)
{
    const aiCamera *pCamera = pScene->mCameras[index];
    printf("Loading camera: '%s'\n", pCamera->mName.C_Str());

    glm::mat4 transformation;
    GetFullTransformation(pScene->mRootNode, pCamera->mName.C_Str(), transformation);

    aiMatrix4x4 aiCameraMatrix;
    pCamera->GetCameraMatrix(aiCameraMatrix);

    glm::mat4 basisChange(1.0f);

    basisChange[0][0] = 1.0f;
    basisChange[0][1] = 0.0f;
    basisChange[0][2] = 0.0f;

    basisChange[1][0] = 0.0f;
    basisChange[1][1] = 0.0f;
    basisChange[1][2] = -1.0f;

    basisChange[2][0] = 0.0f;
    basisChange[2][1] = 1.0f;
    basisChange[2][2] = 0.0f;

    glm::mat4 finalWorld = transformation; // basisChange * transformation;

    glm::vec4 pos4D(0.0f, 0.0f, 0.0f, 1.0f);
    pos4D = finalWorld * pos4D;
    glm::vec3 finalPos(pos4D.x, pos4D.y, pos4D.z);

    glm::vec4 target4D(0.0f, 0.0f, -1.0f, 0.0f);
    target4D = finalWorld * target4D;
    glm::vec3 finalTarget = glm::normalize(glm::vec3(target4D.x, target4D.y, target4D.z));

    glm::vec4 up4D(0.0f, 1.0f, 0.0f, 0.0f);
    up4D = finalWorld * up4D;
    glm::vec3 finalUp = glm::normalize(glm::vec3(up4D.x, up4D.y, up4D.z));

    PersProjInfo persProjInfo;
    persProjInfo.zNear = pCamera->mClipPlaneNear;
    persProjInfo.zFar = pCamera->mClipPlaneFar;

    int windowWidth, windowHeight;
    m_pCoreRenderingSystem->GetWindowSize(windowWidth, windowHeight);
    persProjInfo.Width = (float)windowWidth;
    persProjInfo.Height = (float)windowHeight;

    persProjInfo.FOV = glm::degrees(pCamera->mHorizontalFOV) / 2.0f;

    m_cameras[index].Init(finalPos, finalTarget, finalUp, persProjInfo);
}

void CoreModel::InitLights(const aiScene *pScene)
{
    for (int i = 0; i < (int)pScene->mNumLights; i++)
    {
        InitSingleLight(pScene, *pScene->mLights[i]);
    }
}

void CoreModel::InitSingleLight(const aiScene *pScene, const aiLight &light)
{
    switch (light.mType)
    {
    case aiLightSource_DIRECTIONAL:
        InitDirectionalLight(pScene, light);
        break;

    case aiLightSource_POINT:
        InitPointLight(pScene, light);
        break;

    case aiLightSource_SPOT:
        InitSpotLight(pScene, light);
        break;

    case aiLightSource_AMBIENT:
        printf("Ambient light is not implemented\n");
        exit(0);

    case aiLightSource_AREA:
        printf("Area light is not implemented");
        exit(0);

    default:
        printf("Light type is not defined\n");
        exit(0);
    }
}

void CoreModel::InitDirectionalLight(const aiScene *pScene, const aiLight &light)
{
    DirectionalLight l;
    l.Color = glm::vec3(1.0f);
    l.DiffuseIntensity = 1.0f;

    glm::mat4 transformation;
    GetFullTransformation(pScene->mRootNode, light.mName.C_Str(), transformation);

    /*
    glm::vec3 direction = AssimpToGLM(light.mDirection);
    glm::vec4 dir4D(direction, 0.0f);
    l.WorldDirection = dir4D;
    */

    glm::vec3 direction(transformation[0][2],
                        transformation[1][2],
                        transformation[2][2]);

    l.WorldDirection = glm::normalize(direction);

    glm::vec3 up = AssimpToGLM(light.mUp);
    glm::vec4 up4D(up, 0.0f);
    up4D = transformation * up4D;
    l.Up = glm::vec3(up4D);

    m_dirLights.push_back(l);
}

void CoreModel::InitPointLight(const aiScene *pScene, const aiLight &light)
{
    PointLight l;
    l.Color = glm::vec3(light.mColorDiffuse.r, light.mColorDiffuse.g, light.mColorDiffuse.b);
    l.DiffuseIntensity = 1.0f;

    glm::mat4 transformation;
    GetFullTransformation(pScene->mRootNode, light.mName.C_Str(), transformation);

    glm::vec3 position = AssimpToGLM(light.mPosition);
    glm::vec4 pos4D(position, 1.0f);
    pos4D = transformation * pos4D;
    l.WorldPosition = glm::vec3(pos4D);

    l.Attenuation.Constant = light.mAttenuationConstant;
    l.Attenuation.Linear = light.mAttenuationLinear;
    l.Attenuation.Exp = light.mAttenuationQuadratic;

    m_pointLights.push_back(l);
}

void CoreModel::InitSpotLight(const aiScene *pScene, const aiLight &light)
{
    SpotLight l;
    l.Color = glm::vec3(1.0f);
    l.DiffuseIntensity = 1.0f;

    glm::mat4 transformation;
    GetFullTransformation(pScene->mRootNode, light.mName.C_Str(), transformation);

    /*
    glm::vec3 direction = AssimpToGLM(light.mDirection);
    glm::vec4 dir4D(direction, 0.0f);
    dir4D = transformation * dir4D;
    l.WorldDirection = dir4D;
    */

    glm::vec3 direction(transformation[0][2],
                        transformation[1][2],
                        transformation[2][2]);

    l.WorldDirection = glm::normalize(direction);

    glm::vec3 up = AssimpToGLM(light.mUp);
    if (glm::length(up) == 0)
    {
        if ((l.WorldDirection == glm::vec3(0.0f, 1.0f, 0.0f)) ||
            (l.WorldDirection == glm::vec3(0.0f, -1.0f, 0.0f)))
        {
            l.Up = glm::vec3(1.0f, 0.0f, 0.0f);
        }
        else
        {
            l.Up = glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }
    else
    {
        glm::vec4 up4D(up, 0.0f);
        up4D = transformation * up4D;
        l.Up = glm::vec3(up4D);
    }

    glm::vec3 position = AssimpToGLM(light.mPosition);
    glm::vec4 pos4D(position, 1.0f);
    pos4D = transformation * pos4D;
    l.WorldPosition = glm::vec3(pos4D);

    l.Attenuation.Constant = light.mAttenuationConstant;
    l.Attenuation.Linear = light.mAttenuationLinear;
    l.Attenuation.Exp = light.mAttenuationQuadratic / 100.0;

    l.Cutoff = glm::degrees(light.mAngleOuterCone);

    m_spotLights.push_back(l);
}

void CoreModel::LoadMeshBones(std::vector<SkinnedVertex> &skinnedVertices,
                              u_int meshIndex, const aiMesh *pMesh)
{
    if (pMesh->mNumBones > MAX_BONES)
    {
        printf("The number of bones in the model (%d) is larger than the maximum supported (%d)\n",
               pMesh->mNumBones, MAX_BONES);
        assert(0);
    }

    for (u_int i = 0; i < pMesh->mNumBones; i++)
    {
        LoadSingleBone(skinnedVertices, meshIndex, pMesh->mBones[i]);
    }
}

void CoreModel::LoadSingleBone(std::vector<SkinnedVertex> &skinnedVertices,
                               u_int meshIndex, const aiBone *pBone)
{
    int boneID = GetBoneID(pBone);

    if (boneID == m_boneInfo.size())
    {
        BoneInfo bi(pBone->mOffsetMatrix);
        m_boneInfo.push_back(bi);
    }

    for (u_int i = 0; i < pBone->mNumWeights; i++)
    {
        const aiVertexWeight &vw = pBone->mWeights[i];
        u_int globalVertexID = m_meshes[meshIndex].BaseVertex + vw.mVertexId;

        if (globalVertexID < skinnedVertices.size())
        {
            skinnedVertices[globalVertexID].Bones.AddBoneData(boneID, vw.mWeight);
        }
    }

    MarkRequiredNodesForBone(pBone);
}

void CoreModel::MarkRequiredNodesForBone(const aiBone *pBone)
{
    std::string nodeName(pBone->mName.C_Str());
    const aiNode *pParent = NULL;

    do
    {
        auto it = m_requiredNodeMap.find(nodeName);

        if (it == m_requiredNodeMap.end())
        {
            printf("Cannot find bone %s in the hierarchy\n", nodeName.c_str());
            assert(0);
        }

        it->second.isRequired = true;
        pParent = it->second.pNode->mParent;

        if (pParent)
        {
            nodeName = std::string(pParent->mName.C_Str());
        }

    } while (pParent);
}

void CoreModel::InitializeRequiredNodeMap(const aiNode *pNode)
{
    std::string nodeName(pNode->mName.C_Str());
    NodeInfo info(pNode);
    m_requiredNodeMap[nodeName] = info;

    for (u_int i = 0; i < pNode->mNumChildren; i++)
    {
        InitializeRequiredNodeMap(pNode->mChildren[i]);
    }
}

int CoreModel::GetBoneID(const aiBone *pBone)
{
    int boneIndex = 0;
    std::string boneName(pBone->mName.C_Str());

    if (m_boneNameToIndexMap.find(boneName) == m_boneNameToIndexMap.end())
    {
        boneIndex = (int)m_boneNameToIndexMap.size();
        m_boneNameToIndexMap[boneName] = boneIndex;
    }
    else
    {
        boneIndex = m_boneNameToIndexMap[boneName];
    }

    return boneIndex;
}

u_int CoreModel::FindPosition(float animationTimeTicks, const aiNodeAnim *pNodeAnim)
{
    for (u_int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
    {
        float t = (float)pNodeAnim->mPositionKeys[i + 1].mTime;
        if (animationTimeTicks < t)
        {
            return i;
        }
    }

    return 0;
}

void CoreModel::CalcInterpolatedPosition(aiVector3D &out,
                                         float animationTimeTicks,
                                         const aiNodeAnim *pNodeAnim)
{
    if (pNodeAnim->mNumPositionKeys == 1)
    {
        out = pNodeAnim->mPositionKeys[0].mValue;
        return;
    }

    float maxTime = (float)pNodeAnim->mPositionKeys[pNodeAnim->mNumPositionKeys - 1].mTime;
    if (animationTimeTicks >= maxTime)
    {
        out = pNodeAnim->mPositionKeys[pNodeAnim->mNumPositionKeys - 1].mValue;
        return;
    }

    u_int positionIndex = FindPosition(animationTimeTicks, pNodeAnim);
    u_int nextPositionIndex = positionIndex + 1;

    assert(nextPositionIndex < pNodeAnim->mNumPositionKeys);

    float t1 = (float)pNodeAnim->mPositionKeys[positionIndex].mTime;
    float t2 = (float)pNodeAnim->mPositionKeys[nextPositionIndex].mTime;

    float deltaTime = t2 - t1;
    float factor = 0.0f;

    if (deltaTime > 0.0f)
    {
        factor = factor = (animationTimeTicks - t1) / deltaTime;
    }

    factor = glm::clamp(factor, 0.0f, 1.0f);
    const aiVector3D &start = pNodeAnim->mPositionKeys[positionIndex].mValue;
    const aiVector3D &end = pNodeAnim->mPositionKeys[nextPositionIndex].mValue;
    out = start + factor * (end - start);
}

u_int CoreModel::FindRotation(float animationTimeTicks, const aiNodeAnim *pNodeAnim)
{
    assert(pNodeAnim->mNumRotationKeys > 0);

    for (u_int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
    {
        float t = (float)pNodeAnim->mRotationKeys[i + 1].mTime;
        if (animationTimeTicks < t)
        {
            return i;
        }
    }

    return 0;
}

void CoreModel::CalcInterpolatedRotation(aiQuaternion &out,
                                         float animationTimeTicks,
                                         const aiNodeAnim *pNodeAnim)
{
    if (pNodeAnim->mNumRotationKeys == 1)
    {
        out = pNodeAnim->mRotationKeys[0].mValue;
        return;
    }

    float maxTime = (float)pNodeAnim->mRotationKeys[pNodeAnim->mNumRotationKeys - 1].mTime;
    if (animationTimeTicks >= maxTime)
    {
        out = pNodeAnim->mRotationKeys[pNodeAnim->mNumRotationKeys - 1].mValue;
        return;
    }

    u_int rotationIndex = FindRotation(animationTimeTicks, pNodeAnim);
    u_int nextRotationIndex = rotationIndex + 1;
    assert(nextRotationIndex < pNodeAnim->mNumRotationKeys);

    float t1 = (float)pNodeAnim->mRotationKeys[rotationIndex].mTime;
    float t2 = (float)pNodeAnim->mRotationKeys[nextRotationIndex].mTime;
    float deltaTIme = t2 - t1;
    float factor = 0.0f;

    if (deltaTIme > 0.0f)
    {
        factor = (animationTimeTicks - t1) / deltaTIme;
    }

    factor = glm::clamp(factor, 0.0f, 1.0f);
    const aiQuaternion &start = pNodeAnim->mRotationKeys[rotationIndex].mValue;
    const aiQuaternion &end = pNodeAnim->mRotationKeys[nextRotationIndex].mValue;
    aiQuaternion::Interpolate(out, start, end, factor);

    out.Normalize();
}

u_int CoreModel::FindScaling(float animationTimeTicks, const aiNodeAnim *pNodeAnim)
{
    assert(pNodeAnim->mNumScalingKeys > 0);

    for (u_int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
    {
        float t = (float)pNodeAnim->mScalingKeys[i + 1].mTime;
        if (animationTimeTicks < t)
        {
            return i;
        }
    }

    return 0;
}

void CoreModel::CalcInterpolatedScaling(aiVector3D &out,
                                        float animationTimeTicks,
                                        const aiNodeAnim *pNodeAnim)
{
    if (pNodeAnim->mNumScalingKeys == 1)
    {
        out = pNodeAnim->mScalingKeys[0].mValue;
        return;
    }

    float maxTime = (float)pNodeAnim->mScalingKeys[pNodeAnim->mNumScalingKeys - 1].mTime;
    if (animationTimeTicks >= maxTime)
    {
        out = pNodeAnim->mScalingKeys[pNodeAnim->mNumScalingKeys - 1].mValue;
        return;
    }

    u_int scalingIndex = FindScaling(animationTimeTicks, pNodeAnim);
    u_int nextScalingIndex = scalingIndex + 1;
    assert(nextScalingIndex < pNodeAnim->mNumScalingKeys);

    float t1 = (float)pNodeAnim->mScalingKeys[scalingIndex].mTime;
    float t2 = (float)pNodeAnim->mScalingKeys[nextScalingIndex].mTime;
    float deltaTime = t2 - t1;
    float factor = 0.0f;

    if (deltaTime > 0.0f)
    {
        factor = (animationTimeTicks - t1) / deltaTime;
    }

    factor = glm::clamp(factor, 0.0f, 1.0f);
    const aiVector3D &start = pNodeAnim->mScalingKeys[scalingIndex].mValue;
    const aiVector3D &end = pNodeAnim->mScalingKeys[nextScalingIndex].mValue;

    out = start + factor * (end - start);
}

void CoreModel::ReadNodeHierarchy(float animationTimeTicks, const aiNode *pNode,
                                  const glm::mat4 &parentTransform, const aiAnimation &animation)
{
    std::string nodeName(pNode->mName.data);
    glm::mat4 nodeTransformation = AssimpToGLM(pNode->mTransformation);
    const aiNodeAnim *pNodeAnim = FindNodeAnim(animation, nodeName);

    if (pNodeAnim)
    {
        LocalTransform transform;
        CalcLocalTransform(transform, animationTimeTicks, pNodeAnim);

        glm::mat4 scalingM = glm::scale(glm::mat4(1.0f),
                                        glm::vec3(transform.Scaling.x,
                                                  transform.Scaling.y,
                                                  transform.Scaling.z));

        glm::mat4 rotationM = glm::mat4_cast(glm::quat(transform.Rotation.w,
                                                       transform.Rotation.x,
                                                       transform.Rotation.y,
                                                       transform.Rotation.z));

        glm::mat4 translationM = glm::translate(glm::mat4(1.0f),
                                                glm::vec3(transform.Translation.x,
                                                          transform.Translation.y,
                                                          transform.Translation.z));

        nodeTransformation = translationM * rotationM * scalingM;
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransformation;

    if (m_boneNameToIndexMap.find(nodeName) != m_boneNameToIndexMap.end())
    {
        u_int boneIndex = m_boneNameToIndexMap[nodeName];
        m_boneInfo[boneIndex].FinalTransformation = m_globalInverseTransform *
                                                    globalTransformation *
                                                    m_boneInfo[boneIndex].OffsetMatrix;
    }

    for (u_int i = 0; i < pNode->mNumChildren; i++)
    {
        std::string childName(pNode->mChildren[i]->mName.data);
        std::map<std::string, NodeInfo>::iterator it = m_requiredNodeMap.find(childName);

        if (it == m_requiredNodeMap.end())
        {
            printf("Child %d cannot be found in the required node map\n", childName.c_str());
            assert(0);
        }

        if (it->second.isRequired)
        {
            ReadNodeHierarchy(animationTimeTicks, pNode->mChildren[i],
                              globalTransformation, animation);
        }
    }
}

void CoreModel::ReadNodeHierarchyBlended(float startTicks, float endTicks,
                                         const aiNode *pNode, const glm::mat4 &parentTransform,
                                         const aiAnimation &startAnimation, const aiAnimation &endAnimation,
                                         float blendFactor)
{
    std::string nodeName(pNode->mName.data);
    glm::mat4 nodeTransformation = AssimpToGLM(pNode->mTransformation);

    const aiNodeAnim *pStartNodeAnim = FindNodeAnim(startAnimation, nodeName);
    LocalTransform startTransform;

    if (pStartNodeAnim)
    {
        CalcLocalTransform(startTransform, startTicks, pStartNodeAnim);
    }

    const aiNodeAnim *pEndNodeAnim = FindNodeAnim(endAnimation, nodeName);
    LocalTransform endTransform;

    if (pEndNodeAnim)
    {
        CalcLocalTransform(endTransform, endTicks, pEndNodeAnim);
    }

    if (pStartNodeAnim && !pEndNodeAnim)
    {
        glm::mat4 scalingM = glm::scale(glm::mat4(1.0f),
                                        glm::vec3(startTransform.Scaling.x,
                                                  startTransform.Scaling.y,
                                                  startTransform.Scaling.z));
        glm::mat4 rotationM = glm::mat4_cast(glm::quat(startTransform.Rotation.w,
                                                       startTransform.Rotation.x,
                                                       startTransform.Rotation.y,
                                                       startTransform.Rotation.z));

        glm::mat4 translationM = glm::translate(glm::mat4(1.0f),
                                                glm::vec3(startTransform.Translation.x,
                                                          startTransform.Translation.y,
                                                          startTransform.Translation.z));

        nodeTransformation = translationM * rotationM * scalingM;
    }
    else if (!pStartNodeAnim && pEndNodeAnim)
    {
        glm::mat4 scalingM = glm::scale(glm::mat4(1.0f),
                                        glm::vec3(endTransform.Scaling.x,
                                                  endTransform.Scaling.y,
                                                  endTransform.Scaling.z));
        glm::mat4 rotationM = glm::mat4_cast(glm::quat(endTransform.Rotation.w,
                                                       endTransform.Rotation.x,
                                                       endTransform.Rotation.y,
                                                       endTransform.Rotation.z));

        glm::mat4 translationM = glm::translate(glm::mat4(1.0f),
                                                glm::vec3(endTransform.Translation.x,
                                                          endTransform.Translation.y,
                                                          endTransform.Translation.z));

        nodeTransformation = translationM * rotationM * scalingM;
    }
    else if (pStartNodeAnim && pEndNodeAnim)
    {
        aiVector3D blendedScaling = (1.0f - blendFactor) * startTransform.Scaling +
                                    endTransform.Scaling * blendFactor;

        glm::mat4 scalingM = glm::scale(glm::mat4(1.0f),
                                        glm::vec3(blendedScaling.x,
                                                  blendedScaling.y,
                                                  blendedScaling.z));

        /*
        glm::quat startQuat(startTransform.Rotation.w,
                            startTransform.Rotation.x,
                            startTransform.Rotation.y,
                            startTransform.Rotation.z);
        glm::quat endQuat(endTransform.Rotation.w,
                          endTransform.Rotation.x,
                          endTransform.Rotation.y,
                          endTransform.Rotation.z);

        glm::mat4 rotationM = glm::mat4_cast(glm::slerp(startQuat, endQuat, blendFactor));
        */

        const aiQuaternion &startQuat = startTransform.Rotation;
        const aiQuaternion &endQuat = endTransform.Rotation;
        aiQuaternion blendedQuat;
        aiQuaternion::Interpolate(blendedQuat, startQuat, endQuat, blendFactor);
        glm::mat4 rotationM = glm::mat4_cast(glm::quat(blendedQuat.w,
                                                       blendedQuat.x,
                                                       blendedQuat.y,
                                                       blendedQuat.z));

        aiVector3d blendedTranslation = (1.0f - blendFactor) * startTransform.Translation +
                                        endTransform.Translation * blendFactor;

        glm::mat4 translationM = glm::translate(glm::mat4(1.0f),
                                                glm::vec3(blendedTranslation.x,
                                                          blendedTranslation.y,
                                                          blendedTranslation.z));

        nodeTransformation = translationM * rotationM * scalingM;
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransformation;

    if (m_boneNameToIndexMap.find(nodeName) != m_boneNameToIndexMap.end())
    {
        u_int boneIndex = m_boneNameToIndexMap[nodeName];
        m_boneInfo[boneIndex].FinalTransformation = m_globalInverseTransform *
                                                    globalTransformation *
                                                    m_boneInfo[boneIndex].OffsetMatrix;
    }

    for (u_int i = 0; i < pNode->mNumChildren; i++)
    {
        std::string childName(pNode->mChildren[i]->mName.data);
        auto it = m_requiredNodeMap.find(childName);

        if (it == m_requiredNodeMap.end())
        {
            printf("Child %s cannot be found in the required node map\n", childName.c_str());
            assert(0);
        }

        if (it->second.isRequired)
        {
            ReadNodeHierarchyBlended(startTicks, endTicks,
                                     pNode->mChildren[i], globalTransformation,
                                     startAnimation, endAnimation, blendFactor);
        }
    }
}

void CoreModel::CalcLocalTransform(LocalTransform &transform,
                                   float ticks, const aiNodeAnim *pNodeAnim)
{
    CalcInterpolatedScaling(transform.Scaling, ticks, pNodeAnim);
    CalcInterpolatedRotation(transform.Rotation, ticks, pNodeAnim);
    CalcInterpolatedPosition(transform.Translation, ticks, pNodeAnim);
}

void CoreModel::GetBoneTransforms(float seconds,
                                  std::vector<glm::mat4> &transforms,
                                  u_int animationIndex)
{
    if (animationIndex >= m_pScene->mNumAnimations)
    {
        printf("Invalid animation index %d, max is %d\n",
               animationIndex, m_pScene->mNumAnimations);
        assert(0);
    }

    glm::mat4 identity(1.0f);
    float ticks = CalcAnimationTimeTicks(seconds, animationIndex);
    const aiAnimation &animation = *m_pScene->mAnimations[animationIndex];

    ReadNodeHierarchy(ticks, m_pScene->mRootNode, identity, animation);
    transforms.resize(m_boneInfo.size());

    for (u_int i = 0; i < m_boneInfo.size(); i++)
    {
        transforms[i] = m_boneInfo[i].FinalTransformation;
    }
}

void CoreModel::GetBoneTransformsBlended(float seconds, std::vector<glm::mat4> &blendedTransforms,
                                         u_int startAnimIndex, u_int endAnimIndex, float blendFactor)
{
    if (startAnimIndex >= m_pScene->mNumAnimations)
    {
        printf("Invalid start animation index %d, max is %d\n",
               startAnimIndex, m_pScene->mNumAnimations);
        assert(0);
    }

    if (endAnimIndex >= m_pScene->mNumAnimations)
    {
        printf("Invalid end animation index %d, max is %d\n",
               endAnimIndex, m_pScene->mNumAnimations);
        assert(0);
    }

    if ((blendFactor < 0.0f) || (blendFactor > 1.0f))
    {
        printf("Invalid blend factor %f\n", blendFactor);
        assert(0);
    }

    float startTicks = CalcAnimationTimeTicks(seconds, startAnimIndex);
    float endTicks = CalcAnimationTimeTicks(seconds, endAnimIndex);

    const aiAnimation &startAnimation = *m_pScene->mAnimations[startAnimIndex];
    const aiAnimation &endAnimation = *m_pScene->mAnimations[endAnimIndex];

    glm::mat4 identity(1.0f);
    ReadNodeHierarchyBlended(startTicks, endTicks, m_pScene->mRootNode,
                             identity, startAnimation, endAnimation, blendFactor);
    blendedTransforms.resize(m_boneInfo.size());

    for (u_int i = 0; i < m_boneInfo.size(); i++)
    {
        blendedTransforms[i] = m_boneInfo[i].FinalTransformation;
    }
}

float CoreModel::CalcAnimationTimeTicks(float seconds, u_int animationIndex)
{
    float tps = (float)(m_pScene->mAnimations[animationIndex]->mTicksPerSecond != 0
                            ? m_pScene->mAnimations[animationIndex]->mTicksPerSecond
                            : 25.0f);
    float ticks = seconds * tps;
    double duration = 0.0f;
    modf((double)m_pScene->mAnimations[animationIndex]->mDuration, &duration);

    if (duration <= 0.0f)
    {
        return 0.0f;
    }

    return std::fmod(ticks, duration);
}

const aiNodeAnim *CoreModel::FindNodeAnim(const aiAnimation &animation,
                                          const std::string &nodeName)
{
    for (u_int i = 0; i < animation.mNumChannels; i++)
    {
        const aiNodeAnim *pNodeAnim = animation.mChannels[i];

        if (std::string(pNodeAnim->mNodeName.data) == nodeName)
        {
            return pNodeAnim;
        }
    }

    return NULL;
}

bool CoreModel::IsAnimated() const
{
    bool ret = m_numAnimations > 0;

    if (ret && (NumBones() == 0))
    {
        printf("No bones detected\n");
        assert(0);
    }

    return ret;
}

void GetVertexSizesInBytes(size_t &vertexSize, size_t &skinnedVertexSize)
{
    CoreModel::GetVertexSizesInBytes(vertexSize, skinnedVertexSize);
}
