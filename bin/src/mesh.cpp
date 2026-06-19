#include "../inc/mesh.h"
#include <stdio.h>
#include <cassert>

#define POSITION_LOCATION 0
#define TEX_COORD_LOCATION 1
#define NORMAL_LOCATION 2

Mesh::~Mesh()
{
    Clear();
}

void Mesh::Clear()
{
    for (auto &mat : m_Materials)
    {
        delete mat.pTextures[TEX_TYPE_BASE];
        delete mat.pTextures[TEX_TYPE_SPECULAR];
        mat.pTextures[TEX_TYPE_BASE] = NULL;
        mat.pTextures[TEX_TYPE_SPECULAR] = NULL;
    }

    if (m_Buffers[0] != 0)
    {
        glDeleteBuffers(NUM_BUFFERS, m_Buffers);
    }

    if (m_VAO != 0)
    {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
}

bool Mesh::LoadMesh(const std::string &Filename, int AssimpFlags)
{
    Clear();

    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    glGenBuffers(NUM_BUFFERS, m_Buffers);

    m_pScene = m_Importer.ReadFile(Filename.c_str(), AssimpFlags);

    if (!m_pScene)
    {
        printf("Error parsing '%s': '%s'\n", Filename.c_str(), m_Importer.GetErrorString());
        return false;
    }

    aiMatrix4x4 m = m_pScene->mRootNode->mTransformation;
    m_GlobalInverseTransform = glm::inverse(glm::mat4(
        m.a1, m.b1, m.c1, m.d1,
        m.a2, m.b2, m.c2, m.d2,
        m.a3, m.b3, m.c3, m.d3,
        m.a4, m.b4, m.c4, m.d4));

    bool Ret = InitFromScene(m_pScene, Filename);
    glBindVertexArray(0);

    return Ret;
}

bool Mesh::InitFromScene(const aiScene *pScene, const std::string &Filename)
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

void Mesh::CountVerticesAndIndices(const aiScene *pScene, u_int &NumVertices, u_int &NumIndices)
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

void Mesh::ReserveSpace(u_int NumVertices, u_int NumIndices)
{
    m_Vertices.reserve(NumVertices);
    m_Indices.reserve(NumIndices);
}

void Mesh::InitAllMeshes(const aiScene *pScene)
{
    for (u_int i = 0; i < m_Meshes.size(); i++)
    {
        InitSingleMesh(i, pScene->mMeshes[i]);
    }
}

void Mesh::InitSingleMesh(u_int MeshIndex, const aiMesh *paiMesh)
{
    const aiVector3D Zero3D(0.0f, 0.0f, 0.0f);
    for (u_int i = 0; i < paiMesh->mNumVertices; i++)
    {
        const aiVector3D &pPos = paiMesh->mVertices[i];

        Vertex v;
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

void Mesh::PopulateBuffers()
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

bool Mesh::InitMaterials(const aiScene *pScene, const std::string &Filename)
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

void Mesh::LoadTextures(const std::string &Dir, const aiMaterial *pMaterial, int Index)
{
    LoadDiffuseTexture(Dir, pMaterial, Index);
    LoadSpecularTexture(Dir, pMaterial, Index);
}

void Mesh::LoadDiffuseTexture(const std::string &Dir, const aiMaterial *pMaterial, int MaterialIndex)
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

void Mesh::LoadDiffuseTextureEmbedded(const aiTexture *paiTexture, int MaterialIndex)
{
    printf("Embedded diffuse texture type '%s'\n", paiTexture->achFormatHint);
    m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE] = new Texture(GL_TEXTURE_2D);
    m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]->Load(paiTexture->mWidth, paiTexture->pcData, true);
}

void Mesh::LoadDiffuseTextureFromFile(const std::string &Dir, const aiString &Path, int MaterialIndex)
{
    std::string FullPath = GetFullPath(Dir, Path);
    m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE] = new Texture(GL_TEXTURE_2D, FullPath);

    if (!m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]->Load(true))
    {
        printf("Error loading diffuse texture '%s'\n", FullPath.c_str());
        exit(1);
    }
    else
    {
        printf("Loaded diffuse texture '%s'\n", FullPath.c_str());
    }
}

void Mesh::LoadSpecularTexture(const std::string &Dir, const aiMaterial *pMaterial, int MaterialIndex)
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

void Mesh::LoadSpecularTextureEmbedded(const aiTexture *paiTexture, int MaterialIndex)
{
    printf("Embedded specular texture type '%s'\n", paiTexture->achFormatHint);
    m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR] = new Texture(GL_TEXTURE_2D);
    m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR]->Load(paiTexture->mWidth, paiTexture->pcData, false);
}

void Mesh::LoadSpecularTextureFromFile(const std::string &Dir, const aiString &Path, int MaterialIndex)
{
    std::string FullPath = GetFullPath(Dir, Path);
    m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR] = new Texture(GL_TEXTURE_2D, FullPath);

    if (!m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR]->Load(false))
    {
        printf("Error loading specular texture '%s'\n", FullPath.c_str());
        exit(1);
    }
    else
    {
        printf("Loaded specular texture '%s'\n", FullPath.c_str());
    }
}

void Mesh::LoadColors(const aiMaterial *pMaterial, int index)
{
    aiColor4D AmbientColor(0.0f, 0.0f, 0.0f, 0.0f);
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

void Mesh::Render()
{
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

        glDrawElementsBaseVertex(
            GL_TRIANGLES,
            m_Meshes[i].NumIndices,
            GL_UNSIGNED_INT,
            (void *)(sizeof(u_int) * m_Meshes[i].BaseIndex),
            m_Meshes[i].BaseVertex);
    }

    glBindVertexArray(0);
}

void Mesh::Render(IRenderCallbacks *pRenderCallbacks)
{
    glBindVertexArray(m_VAO);

    for (u_int i = 0; i < m_Meshes.size(); i++)
    {
        u_int MaterialIndex = m_Meshes[i].MaterialIndex;
        assert(MaterialIndex < m_Materials.size());

        if (pRenderCallbacks)
        {
            pRenderCallbacks->DrawStartCB(i);
            pRenderCallbacks->SetMaterial(m_Materials[MaterialIndex]);
        }

        if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE])
        {
            m_Materials[MaterialIndex].pTextures[TEX_TYPE_BASE]->Bind(COLOR_TEXTURE_UNIT);
        }
        else if (pRenderCallbacks)
        {
            pRenderCallbacks->DisableDiffuseTexture();
        }

        if (m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR])
        {
            m_Materials[MaterialIndex].pTextures[TEX_TYPE_SPECULAR]->Bind(SPECULAR_TEXTURE_UNIT);

            if (pRenderCallbacks)
            {
                pRenderCallbacks->ControlSpecularExponent(true);
            }
        }
        else if (pRenderCallbacks)
        {
            pRenderCallbacks->ControlSpecularExponent(false);
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

void Mesh::Render(u_int DrawIndex, u_int PrimID)
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

void Mesh::Render(u_int NumInstances, const glm::mat4 *WVPMats, const glm::mat4 *WorldMats)
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

const Material &Mesh::GetMaterial()
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
