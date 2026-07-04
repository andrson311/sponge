#include <stdio.h>
#include <cassert>
#include <meshoptimizer.h>
#include "basic_mesh.h"

#define POSITION_LOCATION 0
#define TEX_COORD_LOCATION 1
#define NORMAL_LOCATION 2

BasicMesh::~BasicMesh()
{
    Clear();
}

void BasicMesh::Clear()
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

bool BasicMesh::LoadMesh(const std::string &Filename, int AssimpFlags)
{
    Clear();

    if (IsGLVersionHigher(4, 5))
    {
        glCreateVertexArrays(1, &m_VAO);
        glCreateBuffers(std::size(m_Buffers), m_Buffers);
    }
    else
    {
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);
        glGenBuffers(std::size(m_Buffers), m_Buffers);
    }

    bool Ret = false;

    m_pScene = m_Importer.ReadFile(Filename.c_str(), AssimpFlags);

    if (m_pScene)
    {
        aiMatrix4x4 m = m_pScene->mRootNode->mTransformation;
        m_GlobalInverseTransform = glm::inverse(glm::mat4(
            m.a1, m.b1, m.c1, m.d1,
            m.a2, m.b2, m.c2, m.d2,
            m.a3, m.b3, m.c3, m.d3,
            m.a4, m.b4, m.c4, m.d4));

        Ret = InitFromScene(m_pScene, Filename);
    }
    else
    {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), m_Importer.GetErrorString());
    }

    if (!IsGLVersionHigher(4, 5))
    {
        glBindVertexArray(0);
    }

    return Ret;
}

bool BasicMesh::InitFromScene(const aiScene *pScene, const std::string &Filename)
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

void BasicMesh::CountVerticesAndIndices(const aiScene *pScene, u_int &NumVertices, u_int &NumIndices)
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

void BasicMesh::ReserveSpace(u_int NumVertices, u_int NumIndices)
{
    m_Vertices.reserve(NumVertices);
    m_Indices.reserve(NumIndices);
}

void BasicMesh::InitAllMeshes(const aiScene *pScene)
{
    for (u_int i = 0; i < m_Meshes.size(); i++)
    {
        // InitSingleMesh(i, pScene->mMeshes[i]);
        InitSingleMeshOpt(i, pScene->mMeshes[i]);
    }
}

void BasicMesh::InitSingleMesh(u_int MeshIndex, const aiMesh *paiMesh)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    Vertex v;

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
            v.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        const aiVector3D &pTexCoord = paiMesh->HasTextureCoords(0)
                                          ? paiMesh->mTextureCoords[0][i]
                                          : Zero3D;

        v.TexCoords = glm::vec2(pTexCoord.x, pTexCoord.y);
        m_Vertices.push_back(v);
    }

    for (u_int i = 0; i < paiMesh->mNumFaces; i++)
    {
        const aiFace &Face = paiMesh->mFaces[i];
        m_Indices.push_back(Face.mIndices[0]);
        m_Indices.push_back(Face.mIndices[1]);
        m_Indices.push_back(Face.mIndices[2]);
    }
}

void BasicMesh::InitSingleMeshOpt(u_int MeshIndex, const aiMesh *paiMesh)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

    Vertex v;
    std::vector<Vertex> Vertices(paiMesh->mNumVertices);

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

        Vertices[i] = v;
    }

    m_Meshes[MeshIndex].BaseVertex = (u_int)m_Vertices.size();
    m_Meshes[MeshIndex].BaseIndex = (u_int)m_Indices.size();

    int NumIndices = paiMesh->mNumFaces * 3;
    std::vector<u_int> Indices;
    Indices.resize(NumIndices);

    for (u_int i = 0; i < paiMesh->mNumFaces; i++)
    {
        const aiFace &Face = paiMesh->mFaces[i];
        Indices[i * 3 + 0] = Face.mIndices[0];
        Indices[i * 3 + 1] = Face.mIndices[1];
        Indices[i * 3 + 2] = Face.mIndices[2];
    }

    OptimizeMesh(MeshIndex, Indices, Vertices);
}

void BasicMesh::OptimizeMesh(int MeshIndex, std::vector<u_int> &Indices, std::vector<Vertex> &Vertices)
{
    size_t NumIndices = Indices.size();
    size_t NumVertices = Vertices.size();

    for (size_t i = 0; i < NumIndices; i++)
        if (Indices[i] >= NumVertices)
            printf("bad index %u at pos %zu (NumVertices=%zu, mesh=%d)\n", Indices[i], i, NumVertices, MeshIndex);

    // create a remap table
    std::vector<u_int> remap(NumIndices);
    size_t OptVertexCount = meshopt_generateVertexRemap(
        remap.data(),
        Indices.data(),
        NumIndices,
        Vertices.data(),
        NumVertices,
        sizeof(Vertex));

    // allocate local index and vertex arrays
    std::vector<u_int> OptIndices;
    std::vector<Vertex> OptVertices;
    OptIndices.resize(NumIndices);
    OptVertices.resize(OptVertexCount);

    // optimization #1: remove duplicate vertices
    meshopt_remapIndexBuffer(OptIndices.data(), Indices.data(), NumIndices, remap.data());
    meshopt_remapVertexBuffer(OptVertices.data(), Vertices.data(), NumVertices, sizeof(Vertex), remap.data());

    // optimization #2: improve the locality of the vertices
    meshopt_optimizeVertexCache(OptIndices.data(), OptIndices.data(), NumIndices, OptVertexCount);

    // optimization #3: reduce pixel overdraw
    meshopt_optimizeOverdraw(OptIndices.data(), OptIndices.data(),
                             NumIndices, &(OptVertices[0].Position.x),
                             OptVertexCount, sizeof(Vertex), 1.05f);

    // optimization #4: optimize access to the vertex buffer
    meshopt_optimizeVertexFetch(OptVertices.data(), OptIndices.data(),
                                NumIndices, OptVertices.data(),
                                OptVertexCount, sizeof(Vertex));

    // optimization #5: create simplified version of the model
    float Threshold = 0.5f;
    size_t TargetIndexCount = (size_t)(NumIndices * Threshold);

    float TargetError = 0.2f;
    std::vector<u_int> SimplifiedIndices(OptIndices.size());
    size_t OptIndexCount = meshopt_simplify(SimplifiedIndices.data(), OptIndices.data(),
                                            NumIndices, &OptVertices[0].Position.x,
                                            OptVertexCount, sizeof(Vertex),
                                            TargetIndexCount, TargetError);

    static int num_indices = 0;
    num_indices += (int)NumIndices;
    static int opt_indices = 0;
    opt_indices += (int)OptIndexCount;
    printf("Num indices %d\n", num_indices);
    printf("Optimized number of indices %d\n", opt_indices);

    SimplifiedIndices.resize(OptIndexCount);
    m_Indices.insert(m_Indices.end(), SimplifiedIndices.begin(), SimplifiedIndices.end());
    m_Vertices.insert(m_Vertices.end(), OptVertices.begin(), OptVertices.end());
    m_Meshes[MeshIndex].NumIndices = (u_int)OptIndexCount;
}

bool BasicMesh::InitMaterials(const aiScene *pScene, const std::string &Filename)
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

void BasicMesh::LoadTextures(const std::string &Dir, const aiMaterial *pMaterial, int Index)
{
    LoadDiffuseTexture(Dir, pMaterial, Index);
    LoadSpecularTexture(Dir, pMaterial, Index);

    // PBR
    LoadAlbedoTexture(Dir, pMaterial, Index);
    LoadMetallicTexture(Dir, pMaterial, Index);
    LoadRoughnessTexture(Dir, pMaterial, Index);
}

void BasicMesh::LoadDiffuseTexture(const std::string &Dir, const aiMaterial *pMaterial, int MaterialIndex)
{
    m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE] = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        aiString Path;
        if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path) == AI_SUCCESS)
        {
            const aiTexture *paiTexture = m_pScene->GetEmbeddedTexture(Path.C_Str());
            if (paiTexture)
            {
                LoadDiffuseTextureEmbedded(paiTexture, MaterialIndex);
            }
            else
            {
                LoadDiffuseTextureFromFile(Dir, Path, MaterialIndex);
            }
        }
    }
}

void BasicMesh::LoadDiffuseTextureEmbedded(const aiTexture *paiTexture, int MaterialIndex)
{
    printf("Embedded diffuse texture type '%s'\n", paiTexture->achFormatHint);
    m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE] = new Texture(GL_TEXTURE_2D);
    m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]->Load(paiTexture->mWidth, paiTexture->pcData, true);
}

void BasicMesh::LoadDiffuseTextureFromFile(const std::string &Dir, const aiString &Path, int MaterialIndex)
{
    std::string FullPath = GetFullPath(Dir, Path);
    m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE] = new Texture(GL_TEXTURE_2D, FullPath.c_str());

    if (!m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]->Load(true))
    {
        printf("Error loading diffuse texture '%s'\n", FullPath.c_str());
        exit(0);
    }
    else
    {
        printf("Loaded diffuse texture '%s' at index %d\n", FullPath.c_str(), MaterialIndex);
    }
}

void BasicMesh::LoadSpecularTexture(const std::string &Dir, const aiMaterial *pMaterial, int MaterialIndex)
{
    m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR] = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_SHININESS) > 0)
    {
        aiString Path;
        if (pMaterial->GetTexture(aiTextureType_SHININESS, 0, &Path) == AI_SUCCESS)
        {
            const aiTexture *paiTexture = m_pScene->GetEmbeddedTexture(Path.C_Str());
            if (paiTexture)
            {
                LoadSpecularTextureEmbedded(paiTexture, MaterialIndex);
            }
            else
            {
                LoadSpecularTextureFromFile(Dir, Path, MaterialIndex);
            }
        }
    }
}

void BasicMesh::LoadSpecularTextureEmbedded(const aiTexture *paiTexture, int MaterialIndex)
{
    printf("Embedded specular texture type '%s'\n", paiTexture->achFormatHint);
    m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR] = new Texture(GL_TEXTURE_2D);
    m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR]->Load(paiTexture->mWidth, paiTexture->pcData, false);
}

void BasicMesh::LoadSpecularTextureFromFile(const std::string &Dir, const aiString &Path, int MaterialIndex)
{
    std::string FullPath = GetFullPath(Dir, Path);
    m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR] = new Texture(GL_TEXTURE_2D, FullPath.c_str());

    if (!m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR]->Load(false))
    {
        printf("Error loading specular texture '%s'\n", FullPath.c_str());
        exit(0);
    }
    else
    {
        printf("Loaded specular texture '%s'\n", FullPath.c_str());
    }
}

void BasicMesh::LoadAlbedoTexture(const std::string &Dir, const aiMaterial *pMaterial, int MaterialIndex)
{
    m_Materials[MaterialIndex].PBRmaterial.pAlbedo = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_BASE_COLOR) > 0)
    {
        aiString Path;
        if (pMaterial->GetTexture(aiTextureType_BASE_COLOR, 0, &Path) == AI_SUCCESS)
        {
            const aiTexture *paiTexture = m_pScene->GetEmbeddedTexture(Path.C_Str());
            if (paiTexture)
            {
                LoadAlbedoTextureEmbedded(paiTexture, MaterialIndex);
            }
            else
            {
                LoadAlbedoTextureFromFile(Dir, Path, MaterialIndex);
            }
        }
    }
}

void BasicMesh::LoadAlbedoTextureEmbedded(const aiTexture *paiTexture, int MaterialIndex)
{
    printf("Embedded albedo texture type '%s'\n", paiTexture->achFormatHint);
    m_Materials[MaterialIndex].PBRmaterial.pAlbedo = new Texture(GL_TEXTURE_2D);
    m_Materials[MaterialIndex].PBRmaterial.pAlbedo->Load(paiTexture->mWidth, paiTexture->pcData, true);
}

void BasicMesh::LoadAlbedoTextureFromFile(const std::string &Dir, const aiString &Path, int MaterialIndex)
{
    std::string FullPath = GetFullPath(Dir, Path);
    m_Materials[MaterialIndex].PBRmaterial.pAlbedo = new Texture(GL_TEXTURE_2D, FullPath.c_str());

    if (!m_Materials[MaterialIndex].PBRmaterial.pAlbedo->Load(true))
    {
        printf("Error loading albedo texture '%s'\n", FullPath.c_str());
        exit(0);
    }
    else
    {
        printf("Loaded albedo texture '%s'\n", FullPath.c_str());
    }
}

void BasicMesh::LoadMetallicTexture(const std::string &Dir, const aiMaterial *pMaterial, int MaterialIndex)
{
    m_Materials[MaterialIndex].PBRmaterial.pMetallic = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_METALNESS) > 0)
    {
        aiString Path;
        if (pMaterial->GetTexture(aiTextureType_METALNESS, 0, &Path) == AI_SUCCESS)
        {
            const aiTexture *paiTexture = m_pScene->GetEmbeddedTexture(Path.C_Str());
            if (paiTexture)
            {
                LoadMetallicTextureEmbedded(paiTexture, MaterialIndex);
            }
            else
            {
                LoadMetallicTextureFromFile(Dir, Path, MaterialIndex);
            }
        }
    }
}

void BasicMesh::LoadMetallicTextureEmbedded(const aiTexture *paiTexture, int MaterialIndex)
{
    printf("Embedded metallic texture type '%s'\n", paiTexture->achFormatHint);
    m_Materials[MaterialIndex].PBRmaterial.pMetallic = new Texture(GL_TEXTURE_2D);
    m_Materials[MaterialIndex].PBRmaterial.pMetallic->Load(paiTexture->mWidth, paiTexture->pcData, false);
}

void BasicMesh::LoadMetallicTextureFromFile(const std::string &Dir, const aiString &Path, int MaterialIndex)
{
    std::string FullPath = GetFullPath(Dir, Path);
    m_Materials[MaterialIndex].PBRmaterial.pMetallic = new Texture(GL_TEXTURE_2D, FullPath.c_str());

    if (!m_Materials[MaterialIndex].PBRmaterial.pMetallic->Load(false))
    {
        printf("Error loading metalness texture '%s'\n", FullPath.c_str());
        exit(0);
    }
    else
    {
        printf("Loaded metalness texture '%s'\n", FullPath.c_str());
    }
}

void BasicMesh::LoadRoughnessTexture(const std::string &Dir, const aiMaterial *pMaterial, int MaterialIndex)
{
    m_Materials[MaterialIndex].PBRmaterial.pRoughness = NULL;

    if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0)
    {
        aiString Path;
        if (pMaterial->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &Path) == AI_SUCCESS)
        {
            const aiTexture *paiTexture = m_pScene->GetEmbeddedTexture(Path.C_Str());
            if (paiTexture)
            {
                LoadRoughnessTextureEmbedded(paiTexture, MaterialIndex);
            }
            else
            {
                LoadRoughnessTextureFromFile(Dir, Path, MaterialIndex);
            }
        }
    }
}

void BasicMesh::LoadRoughnessTextureEmbedded(const aiTexture *paiTexture, int MaterialIndex)
{
    printf("Embedded roughness texture type '%s'\n", paiTexture->achFormatHint);
    m_Materials[MaterialIndex].PBRmaterial.pRoughness = new Texture(GL_TEXTURE_2D);
    m_Materials[MaterialIndex].PBRmaterial.pRoughness->Load(paiTexture->mWidth, paiTexture->pcData, false);
}

void BasicMesh::LoadRoughnessTextureFromFile(const std::string &Dir, const aiString &Path, int MaterialIndex)
{
    std::string FullPath = GetFullPath(Dir, Path);
    m_Materials[MaterialIndex].PBRmaterial.pRoughness = new Texture(GL_TEXTURE_2D, FullPath.c_str());

    if (!m_Materials[MaterialIndex].PBRmaterial.pRoughness->Load(false))
    {
        printf("Error loading roughness texture '%s'\n", FullPath.c_str());
        exit(0);
    }
    else
    {
        printf("Loaded roughness texture '%s'\n", FullPath.c_str());
    }
}

void BasicMesh::LoadColors(const aiMaterial *pMaterial, int index)
{
    aiColor4D AmbientColor(0.0f, 0.0f, 0.0f, 0.0f);

    int ShadingModel = 0;
    if (pMaterial->Get(AI_MATKEY_SHADING_MODEL, ShadingModel) == AI_SUCCESS)
    {
        printf("Shading model %d\n", ShadingModel);
    }

    if (pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, AmbientColor) == AI_SUCCESS)
    {
        printf("Loaded ambient color [%f %f %f]\n", AmbientColor.r, AmbientColor.g, AmbientColor.b);
        m_Materials[index].AmbientColor = glm::vec4(AmbientColor.r, AmbientColor.g, AmbientColor.b, 1.0f);

        if (glm::length(glm::vec3(m_Materials[index].AmbientColor)) < 0.0001f)
        {
            m_Materials[index].AmbientColor = glm::vec4(1.0f);
        }
    }
    else
    {
        m_Materials[index].AmbientColor = glm::vec4(1.0f);
    }

    aiColor3D DiffuseColor(0.0f, 0.0f, 0.0f);
    if (pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, DiffuseColor) == AI_SUCCESS)
    {
        printf("Loaded diffuse color [%f %f %f]\n", DiffuseColor.r, DiffuseColor.g, DiffuseColor.b);
        m_Materials[index].DiffuseColor = glm::vec4(DiffuseColor.r, DiffuseColor.g, DiffuseColor.b, 1.0f);
    }
    else
    {
        m_Materials[index].DiffuseColor = glm::vec4(1.0f);
    }

    aiColor3D SpecularColor(0.0f, 0.0f, 0.0f);
    if (pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, SpecularColor) == AI_SUCCESS)
    {
        printf("Loaded specular color [%f %f %f]\n", SpecularColor.r, SpecularColor.g, SpecularColor.b);
        m_Materials[index].SpecularColor = glm::vec4(SpecularColor.r, SpecularColor.g, SpecularColor.b, 1.0f);
    }
    else
    {
        m_Materials[index].SpecularColor = glm::vec4(1.0f);
    }
}

void BasicMesh::PopulateBuffers()
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

void BasicMesh::PopulateBuffersNonDSA()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[VERTEX_BUFFER]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffers[INDEX_BUFFER]);

    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(m_Vertices[0]) * m_Vertices.size(), m_Vertices.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(m_Indices[0]) * m_Indices.size(), m_Indices.data(), GL_STATIC_DRAW);

    size_t NumFloats = 0;

    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION,
                          3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)(NumFloats * sizeof(float)));
    NumFloats += 3;

    glEnableVertexAttribArray(TEX_COORD_LOCATION);
    glVertexAttribPointer(TEX_COORD_LOCATION,
                          2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)(NumFloats * sizeof(float)));
    NumFloats += 2;

    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION,
                          3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)(NumFloats * sizeof(float)));
}

void BasicMesh::PopulateBuffersDSA()
{
    glNamedBufferStorage(m_Buffers[VERTEX_BUFFER], sizeof(m_Vertices[0]) * m_Vertices.size(), m_Vertices.data(), 0);
    glNamedBufferStorage(m_Buffers[INDEX_BUFFER], sizeof(m_Indices[0]) * m_Indices.size(), m_Indices.data(), 0);

    glVertexArrayVertexBuffer(m_VAO, 0, m_Buffers[VERTEX_BUFFER], 0, sizeof(Vertex));
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
}

void BasicMesh::Render(IRenderCallbacks *pRenderCallbacks)
{
    if (m_isPBR)
    {
        SetupRenderMaterialsPBR();
    }

    glBindVertexArray(m_VAO);

    for (u_int i = 0; i < m_Meshes.size(); i++)
    {
        u_int MaterialIndex = m_Meshes[i].MaterialIndex;
        assert(MaterialIndex < m_Materials.size());

        if (!m_isPBR)
        {
            SetupRenderMaterialsPhong(i, MaterialIndex, pRenderCallbacks);
        }

        glDrawElementsBaseVertex(
            GL_TRIANGLES,
            m_Meshes[i].NumIndices,
            GL_UNSIGNED_INT,
            (void *)(sizeof(u_int) * m_Meshes[i].BaseIndex),
            m_Meshes[i].BaseVertex);
    }

    glBindVertexArray(0);
}

void BasicMesh::SetupRenderMaterialsPhong(u_int MeshIndex, u_int MaterialIndex, IRenderCallbacks *pRenderCallbacks)
{
    if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE])
    {
        m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]->Bind(COLOR_TEXTURE_UNIT);
    }

    if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR])
    {
        m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR]->Bind(SPECULAR_TEXTURE_UNIT);

        if (pRenderCallbacks)
        {
            pRenderCallbacks->ControlSpecularExponent(true);
        }
    }
    else
    {
        if (pRenderCallbacks)
        {
            pRenderCallbacks->ControlSpecularExponent(false);
        }
    }

    if (pRenderCallbacks)
    {
        if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE])
        {
            pRenderCallbacks->DrawStartCB(MeshIndex);
            pRenderCallbacks->SetMaterial(m_Materials[MaterialIndex]);
        }
        else
        {
            pRenderCallbacks->DisableDiffuseTexture();
        }
    }
}

void BasicMesh::SetupRenderMaterialsPBR()
{
    int PBRMaterialIndex = 0;

    if (m_Materials[PBRMaterialIndex].PBRmaterial.pAlbedo)
    {
        m_Materials[PBRMaterialIndex].PBRmaterial.pAlbedo->Bind(ALBEDO_TEXTURE_UNIT);
    }

    if (m_Materials[PBRMaterialIndex].PBRmaterial.pRoughness)
    {
        m_Materials[PBRMaterialIndex].PBRmaterial.pRoughness->Bind(ROUGHNESS_TEXTURE_UNIT);
    }

    if (m_Materials[PBRMaterialIndex].PBRmaterial.pMetallic)
    {
        m_Materials[PBRMaterialIndex].PBRmaterial.pMetallic->Bind(METALLIC_TEXTURE_UNIT);
    }

    if (m_Materials[PBRMaterialIndex].PBRmaterial.pNormalMap)
    {
        m_Materials[PBRMaterialIndex].PBRmaterial.pNormalMap->Bind(NORMAL_TEXTURE_UNIT);
    }
}

void BasicMesh::Render(u_int DrawIndex, u_int PrimID)
{
    glBindVertexArray(m_VAO);

    u_int MaterialIndex = m_Meshes[DrawIndex].MaterialIndex;
    assert(MaterialIndex < m_Materials.size());

    if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE])
    {
        m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]->Bind(COLOR_TEXTURE_UNIT);
    }

    if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR])
    {
        m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR]->Bind(COLOR_TEXTURE_UNIT);
    }

    glDrawElementsBaseVertex(
        GL_TRIANGLES,
        3,
        GL_UNSIGNED_INT,
        (void *)(sizeof(u_int) * (m_Meshes[DrawIndex].BaseIndex + PrimID * 3)),
        m_Meshes[DrawIndex].BaseVertex);

    glBindVertexArray(0);
}

void BasicMesh::Render(u_int NumInstances, const glm::mat4 *WVPMats, const glm::mat4 *WorldMats)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[WVP_MAT_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * NumInstances, WVPMats, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_Buffers[WORLD_MAT_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * NumInstances, WorldMats, GL_DYNAMIC_DRAW);

    glBindVertexArray(m_VAO);

    for (u_int i = 0; i < m_Meshes.size(); i++)
    {
        u_int MaterialIndex = m_Meshes[i].MaterialIndex;
        assert(MaterialIndex < m_Materials.size());

        if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE])
        {
            m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]->Bind(COLOR_TEXTURE_UNIT);
        }

        if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR])
        {
            m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR]->Bind(SPECULAR_TEXTURE_UNIT);
        }

        glDrawElementsInstancedBaseVertex(
            GL_TRIANGLES,
            m_Meshes[i].NumIndices,
            GL_UNSIGNED_INT,
            (void *)(sizeof(u_int) * m_Meshes[i].BaseIndex),
            NumInstances,
            m_Meshes[i].BaseVertex);
    }

    glBindVertexArray(0);
}

const Material &BasicMesh::GetMaterial()
{
    for (u_int i = 0; i < m_Materials.size(); i++)
    {
        if (m_Materials[i].AmbientColor != glm::vec4(0.0f))
        {
            return m_Materials[i];
        }
    }

    if (m_Materials.empty())
    {
        printf("No materials\n");
        exit(1);
    }

    return m_Materials[0];
}

void BasicMesh::GetLeadingVertex(u_int DrawIndex, u_int PrimID, aiVector3D &Vertex)
{
    u_int MeshIndex = DrawIndex;

    assert(MeshIndex < m_pScene->mNumMeshes);
    const aiMesh *paiMesh = m_pScene->mMeshes[MeshIndex];

    assert(PrimID < paiMesh->mNumFaces);
    const aiFace &Face = paiMesh->mFaces[PrimID];

    u_int LeadingIndex = Face.mIndices[0];

    assert(LeadingIndex < paiMesh->mNumVertices);
    const aiVector3D &Pos = paiMesh->mVertices[LeadingIndex];
    Vertex = Pos;
}