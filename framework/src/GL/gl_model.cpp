#include "core_rendering_system.h"
#include "gl_model.h"
#include "gl_engine_common.h"
#include "gl_ssbo_db.h"

bool usePVP = true; // programmable vertex pulling
bool useIndirectRender = false;

#define POSITION_LOCATION 0
#define TEX_COORD0_LOCATION 1
#define TEX_COORD1_LOCATION 2
#define NORMAL_LOCATION 3
#define TANGENT_LOCATION 4
#define BITANGENT_LOCATION 5
#define COLOR_LOCATION 6
#define BONE_ID_LOCATION 7
#define BONE_WEIGHT_LOCATION 8

static GLuint sWhiteTexture = -1;
static GLuint sWhiteNormalTexture = -1;

static void CreateDummyTexture(GLuint &tex, u_char r, u_char g, u_char b)
{
    if (tex == -1)
    {
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);

        u_char dummyPixel[3] = {r, g, b};

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, dummyPixel);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
}

static void CreateWhiteTexture()
{
    CreateDummyTexture(sWhiteTexture, 255, 255, 255);
}

static void CreateWhiteNormalTexture()
{
    CreateDummyTexture(sWhiteNormalTexture, 127, 127, 255);
}

static void BindWhiteTexture(GLenum textureUnit)
{
    assert(sWhiteTexture != -1);

    if (IsGLVersionHigher(4, 5))
    {
        glBindTextureUnit(textureUnit - GL_TEXTURE0, sWhiteTexture);
    }
    else
    {
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, sWhiteTexture);
    }
}

static void BindWhiteNormalTexture(GLenum textureUnit)
{
    assert(sWhiteNormalTexture != -1);

    if (IsGLVersionHigher(4, 5))
    {
        glBindTextureUnit(textureUnit - GL_TEXTURE0, sWhiteNormalTexture);
    }
    else
    {
        glActiveTexture(textureUnit);
        glBindTexture(GL_TEXTURE_2D, sWhiteNormalTexture);
    }
}

GLModel::GLModel()
{
    CreateWhiteTexture();
    CreateWhiteNormalTexture();
}

GLModel::GLModel(CoreRenderingSystem *pCoreRenderingSystem) : CoreModel(pCoreRenderingSystem)
{
    CreateWhiteTexture();
    CreateWhiteNormalTexture();
}

GLModel::~GLModel()
{
    if (m_buffers[0] != 0)
    {
        glDeleteBuffers(std::size(m_buffers), m_buffers);
    }

    if (m_VAO != 0)
    {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
}

void GLModel::ConvertToMesh(const char *pFilename)
{
    assert(m_vertexBufferSizeBytes > 0);
    assert(m_indexBufferSizeBytes > 0);

    MeshData mesh;

    mesh.m_totalVertices = (int)(m_vertexBufferSizeBytes / sizeof(Vertex));
    mesh.m_totalIndices = (int)m_indices.size();
    mesh.m_vertexSize = (int)sizeof(Vertex);

    size_t numMeshes = m_meshes.size();
    mesh.m_meshes.resize(numMeshes);
    mesh.m_boxes.resize(numMeshes);

    mesh.m_indexData.Size = m_indexBufferSizeBytes;
    mesh.m_indexData.pMem = (char *)glMapNamedBufferRange(m_buffers[INDEX_BUFFER], 0,
                                                          mesh.m_indexData.Size, GL_MAP_READ_BIT);

    mesh.m_vertexData.Size = m_vertexBufferSizeBytes;
    mesh.m_vertexData.pMem = (char *)glMapNamedBufferRange(m_buffers[VERTEX_BUFFER], 0,
                                                           mesh.m_vertexData.Size, GL_MAP_READ_BIT);

    for (size_t i = 0; i < numMeshes; i++)
    {
        printf("Converting meshes %d/%d\n", (int)i + 1, (int)numMeshes);
        Mesh m;

        // TODO: lod
        m.m_baseIndex = m_meshes[i].BaseIndex;
        m.m_baseVertex = m_meshes[i].BaseVertex;
        m.m_numIndices = m_meshes[i].NumIndices;
        m.m_numVertices = m_meshes[i].NumVertices;
        m.m_lodOffsets[0] = 0;
        m.m_lodOffsets[1] = m_meshes[i].NumIndices;

        mesh.m_meshes[i] = m;
    }

    SaveMeshData(pFilename, mesh);

    glUnmapNamedBuffer(m_buffers[INDEX_BUFFER]);
    glUnmapNamedBuffer(m_buffers[VERTEX_BUFFER]);
}

bool GLModel::LoadMesh(const std::string &filename)
{
    MeshData mesh;
    LoadMeshData(filename.c_str(), mesh);

    AllocBuffers();

    m_meshes.resize(mesh.m_meshes.size());

    for (int i = 0; i < m_meshes.size(); i++)
    {
        m_meshes[i].NumIndices = mesh.m_meshes[i].m_numIndices;
        m_meshes[i].NumVertices = mesh.m_meshes[i].m_numVertices;
        m_meshes[i].BaseVertex = mesh.m_meshes[i].m_baseVertex;
        m_meshes[i].BaseIndex = mesh.m_meshes[i].m_baseIndex;
        m_meshes[i].MaterialIndex = 0;
        m_meshes[i].Transformation = glm::mat4(1.0f);
    }

    m_indices.resize(mesh.m_totalIndices);
    memcpy(m_indices.data(), mesh.m_indexData.pMem, mesh.m_indexData.Size);

    std::vector<Vertex> vertices(mesh.m_totalVertices);
    memcpy(vertices.data(), mesh.m_vertexData.pMem, mesh.m_vertexData.Size);

    PopulateBuffers(vertices);
    m_materials.resize(1);
    mesh.Destroy();

    glBindVertexArray(0);

    return true;
}

void GLModel::AllocBuffers()
{
    if (IsGLVersionHigher(4, 5))
    {
        glCreateVertexArrays(1, &m_VAO);
        glCreateBuffers(std::size(m_buffers), m_buffers);
    }
    else
    {
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);
        glGenBuffers(std::size(m_buffers), m_buffers);
    }
}

void GLModel::PopulateBuffersSkinned(std::vector<SkinnedVertex> &vertices)
{
    PopulateBuffersInternal<SkinnedVertex>(vertices);
}

void GLModel::PopulateBuffers(std::vector<Vertex> &vertices)
{
    PopulateBuffersInternal<Vertex>(vertices);
}

void GLModel::InitGeometryPost()
{
    glBindVertexArray(0);
}

template <typename VertexType>
void GLModel::PopulateBuffersInternal(std::vector<VertexType> &vertices)
{
    m_vertexBufferSizeBytes = sizeof(VertexType) * vertices.size();
    m_indexBufferSizeBytes = sizeof(m_indices[0]) * m_indices.size();

    if (usePVP)
    {
        if (IsGLVersionHigher(4, 5))
        {
            PopulateBuffersPVP(vertices);
        }
        else
        {
            printf("Programmable vertex pulling but OpenGL version is less than 4.5\n");
            exit(1);
        }
    }
    else
    {
        if (IsGLVersionHigher(4, 5))
        {
            PopulateBuffersDSA(vertices);
        }
        else
        {
            PopulateBuffersNonDSA(vertices);
        }
    }
}

template <typename VertexType>
void GLModel::PopulateBuffersPVP(std::vector<VertexType> &vertices)
{
    glNamedBufferStorage(m_buffers[VERTEX_BUFFER], m_vertexBufferSizeBytes, vertices.data(), GL_MAP_READ_BIT);
    glNamedBufferStorage(m_buffers[INDEX_BUFFER], m_indexBufferSizeBytes, m_indices.data(), GL_MAP_READ_BIT);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_INDEX_VERTICES, m_buffers[VERTEX_BUFFER]);
    glVertexArrayElementBuffer(m_VAO, m_buffers[INDEX_BUFFER]);
}

template <typename VertexType>
void GLModel::PopulateBuffersNonDSA(std::vector<VertexType> &vertices)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[VERTEX_BUFFER]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_buffers[INDEX_BUFFER]);

    glBufferData(GL_ARRAY_BUFFER, m_vertexBufferSizeBytes, vertices.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferSizeBytes, &m_indices[0], GL_STATIC_DRAW);

    size_t numFloats = 0;

    int numElements = 3;
    glEnableVertexAttribArray(POSITION_LOCATION);
    glVertexAttribPointer(POSITION_LOCATION, numElements, GL_FLOAT, GL_FALSE,
                          sizeof(VertexType), (const void *)(numFloats * sizeof(float)));
    numFloats += numElements;

    numElements = 2;
    glEnableVertexAttribArray(TEX_COORD0_LOCATION);
    glVertexAttribPointer(TEX_COORD0_LOCATION, numElements, GL_FLOAT, GL_FALSE,
                          sizeof(VertexType), (const void *)(numFloats * sizeof(float)));
    numFloats += numElements;

    numElements = 2;
    glEnableVertexAttribArray(TEX_COORD1_LOCATION);
    glVertexAttribPointer(TEX_COORD1_LOCATION, numElements, GL_FLOAT, GL_FALSE,
                          sizeof(VertexType), (const void *)(numFloats * sizeof(float)));
    numFloats += numElements;

    numElements = 3;
    glEnableVertexAttribArray(NORMAL_LOCATION);
    glVertexAttribPointer(NORMAL_LOCATION, numElements, GL_FLOAT, GL_FALSE,
                          sizeof(VertexType), (const void *)(numFloats * sizeof(float)));
    numFloats += numElements;

    numElements = 3;
    glEnableVertexAttribArray(TANGENT_LOCATION);
    glVertexAttribPointer(TANGENT_LOCATION, numElements, GL_FLOAT, GL_FALSE,
                          sizeof(VertexType), (const void *)(numFloats * sizeof(float)));
    numFloats += numElements;

    numElements = 3;
    glEnableVertexAttribArray(BITANGENT_LOCATION);
    glVertexAttribPointer(BITANGENT_LOCATION, numElements, GL_FLOAT, GL_FALSE,
                          sizeof(VertexType), (const void *)(numFloats * sizeof(float)));
    numFloats += numElements;

    numElements = 4;
    glEnableVertexAttribArray(COLOR_LOCATION);
    glVertexAttribPointer(COLOR_LOCATION, numElements, GL_FLOAT, GL_FALSE,
                          sizeof(VertexType), (const void *)(numFloats * sizeof(float)));
    numFloats += numElements;

    if constexpr (std::is_same_v<VertexType, SkinnedVertex>)
    {
        numElements = MAX_NUM_BONES_PER_VERTEX;
        glEnableVertexAttribArray(BONE_ID_LOCATION);
        glVertexAttribPointer(BONE_ID_LOCATION, numElements, GL_INT, GL_FALSE,
                              sizeof(VertexType), (const void *)(numFloats * sizeof(float)));
        numFloats += numElements;

        numElements = MAX_NUM_BONES_PER_VERTEX;
        glEnableVertexAttribArray(BONE_WEIGHT_LOCATION);
        glVertexAttribPointer(BONE_WEIGHT_LOCATION, numElements, GL_FLOAT, GL_FALSE,
                              sizeof(VertexType), (const void *)(numFloats * sizeof(float)));
        numFloats += numElements;
    }
}

template <typename VertexType>
void GLModel::PopulateBuffersDSA(std::vector<VertexType> &vertices)
{
    glNamedBufferStorage(m_buffers[VERTEX_BUFFER], m_vertexBufferSizeBytes, vertices.data(), 0);
    glNamedBufferStorage(m_buffers[INDEX_BUFFER], m_indexBufferSizeBytes, m_indices.data(), 0);

    glVertexArrayVertexBuffer(m_VAO, 0, m_buffers[VERTEX_BUFFER], 0, sizeof(VertexType));
    glVertexArrayElementBuffer(m_VAO, m_buffers[INDEX_BUFFER]);

    size_t numFloats = 0;

    int numElements = 3;
    glEnableVertexArrayAttrib(m_VAO, POSITION_LOCATION);
    glVertexArrayAttribFormat(m_VAO, POSITION_LOCATION, numElements, GL_FLOAT, GL_FALSE,
                              (GLuint)(numFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, POSITION_LOCATION, 0);
    numFloats += numElements;

    numElements = 2;
    glEnableVertexArrayAttrib(m_VAO, TEX_COORD0_LOCATION);
    glVertexArrayAttribFormat(m_VAO, TEX_COORD0_LOCATION, numElements, GL_FLOAT, GL_FALSE,
                              (GLuint)(numFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, TEX_COORD0_LOCATION, 0);
    numFloats += numElements;

    numElements = 2;
    glEnableVertexArrayAttrib(m_VAO, TEX_COORD1_LOCATION);
    glVertexArrayAttribFormat(m_VAO, TEX_COORD1_LOCATION, numElements, GL_FLOAT, GL_FALSE,
                              (GLuint)(numFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, TEX_COORD1_LOCATION, 0);
    numFloats += numElements;

    numElements = 3;
    glEnableVertexArrayAttrib(m_VAO, NORMAL_LOCATION);
    glVertexArrayAttribFormat(m_VAO, NORMAL_LOCATION, numElements, GL_FLOAT, GL_FALSE,
                              (GLuint)(numFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, NORMAL_LOCATION, 0);
    numFloats += numElements;

    numElements = 3;
    glEnableVertexArrayAttrib(m_VAO, TANGENT_LOCATION);
    glVertexArrayAttribFormat(m_VAO, TANGENT_LOCATION, numElements, GL_FLOAT, GL_FALSE,
                              (GLuint)(numFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, TANGENT_LOCATION, 0);
    numFloats += numElements;

    numElements = 3;
    glEnableVertexArrayAttrib(m_VAO, BITANGENT_LOCATION);
    glVertexArrayAttribFormat(m_VAO, BITANGENT_LOCATION, numElements, GL_FLOAT, GL_FALSE,
                              (GLuint)(numFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, BITANGENT_LOCATION, 0);
    numFloats += numElements;

    numElements = 4;
    glEnableVertexArrayAttrib(m_VAO, COLOR_LOCATION);
    glVertexArrayAttribFormat(m_VAO, COLOR_LOCATION, numElements, GL_FLOAT, GL_FALSE,
                              (GLuint)(numFloats * sizeof(float)));
    glVertexArrayAttribBinding(m_VAO, COLOR_LOCATION, 0);
    numFloats += numElements;

    if constexpr (std::is_same_v<VertexType, SkinnedVertex>)
    {
        numElements = MAX_NUM_BONES_PER_VERTEX;
        glEnableVertexArrayAttrib(m_VAO, BONE_ID_LOCATION);
        glVertexArrayAttribFormat(m_VAO, BONE_ID_LOCATION, numElements, GL_INT, GL_FALSE,
                                  (GLuint)(numFloats * sizeof(float)));
        glVertexArrayAttribBinding(m_VAO, BONE_ID_LOCATION, 0);
        numFloats += numElements;

        numElements = MAX_NUM_BONES_PER_VERTEX;
        glEnableVertexArrayAttrib(m_VAO, BONE_WEIGHT_LOCATION);
        glVertexArrayAttribFormat(m_VAO, BONE_WEIGHT_LOCATION, numElements, GL_FLOAT, GL_FALSE,
                                  (GLuint)(numFloats * sizeof(float)));
        glVertexArrayAttribBinding(m_VAO, BONE_WEIGHT_LOCATION, 0);
        numFloats += numElements;
    }
}

void GLModel::Render(RenderCallbacks *pRenderCallbacks)
{
    assert(!useIndirectRender);

    glBindVertexArray(m_VAO);

    if (usePVP)
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER,
                         SSBO_INDEX_VERTICES, m_buffers[VERTEX_BUFFER]);
    }

    for (u_int i = 0; i < m_meshes.size(); i++)
    {
        RenderMesh(i, pRenderCallbacks);
    }

    glBindVertexArray(0);
}

void GLModel::RenderMesh(int meshIndex, RenderCallbacks *pRenderCallbacks)
{
    int materialIndex = m_meshes[meshIndex].MaterialIndex;
    assert((materialIndex >= 0) && (materialIndex < m_materials.size()));

    BindTextures(materialIndex);

    if (pRenderCallbacks)
    {
        bool hasSpecularTexture = m_materials[materialIndex].pTextures[TEX_TYPE_SPECULAR] != NULL;
        pRenderCallbacks->ControlSpecularExponentCB(hasSpecularTexture);
        pRenderCallbacks->DrawStartCB(meshIndex);
        pRenderCallbacks->SetMaterialCB(m_materials[materialIndex]);
        pRenderCallbacks->SetWorldMatrixCB(m_meshes[meshIndex].Transformation);
    }

    glDrawElementsBaseVertex(GL_TRIANGLES, m_meshes[meshIndex].NumIndices, GL_UNSIGNED_INT,
                             (void *)(sizeof(u_int) * m_meshes[meshIndex].BaseIndex),
                             m_meshes[meshIndex].BaseVertex);
}

void GLModel::BindTextures(int materialIndex)
{
    if (m_materials[materialIndex].pTextures[TEX_TYPE_BASE])
    {
        m_materials[materialIndex].pTextures[TEX_TYPE_BASE]->Bind(COLOR_TEXTURE_UNIT);
    }
    else
    {
        BindWhiteTexture(COLOR_TEXTURE_UNIT);
    }

    if (m_materials[materialIndex].pTextures[TEX_TYPE_SPECULAR])
    {
        m_materials[materialIndex].pTextures[TEX_TYPE_SPECULAR]->Bind(SPECULAR_EXPONENT_UNIT);
    }

    if (m_materials[materialIndex].pTextures[TEX_TYPE_EMISSIVE])
    {
        m_materials[materialIndex].pTextures[TEX_TYPE_EMISSIVE]->Bind(EMISSIVE_TEXTURE_UNIT);
    }
    else
    {
        BindWhiteTexture(EMISSIVE_TEXTURE_UNIT);
    }

    if (m_materials[materialIndex].pTextures[TEX_TYPE_ROUGHNESS])
    {
        m_materials[materialIndex].pTextures[TEX_TYPE_ROUGHNESS]->Bind(ROUGHNESS_TEXTURE_UNIT);
    }

    if (m_materials[materialIndex].pTextures[TEX_TYPE_CLEARCOAT])
    {
        m_materials[materialIndex].pTextures[TEX_TYPE_CLEARCOAT]->Bind(CLEARCOAT_TEXTURE_UNIT);
    }
    else
    {
        BindWhiteTexture(CLEARCOAT_TEXTURE_UNIT);
    }

    if (m_materials[materialIndex].pTextures[TEX_TYPE_CLEARCOAT_ROUGHNESS])
    {
        m_materials[materialIndex].pTextures[TEX_TYPE_CLEARCOAT_ROUGHNESS]->Bind(CLEARCOAT_ROUGHNESS_TEXTURE_UNIT);
    }
    else
    {
        BindWhiteTexture(CLEARCOAT_ROUGHNESS_TEXTURE_UNIT);
    }

    if (m_materials[materialIndex].pTextures[TEX_TYPE_CLEARCOAT_NORMAL])
    {
        m_materials[materialIndex].pTextures[TEX_TYPE_CLEARCOAT_NORMAL]->Bind(CLEARCOAT_NORMAL_TEXTURE_UNIT);
    }
    else
    {
        BindWhiteNormalTexture(CLEARCOAT_NORMAL_TEXTURE_UNIT);
    }

    if (m_materials[materialIndex].pTextures[TEX_TYPE_NORMAL])
    {
        m_materials[materialIndex].pTextures[TEX_TYPE_NORMAL]->Bind(NORMAL_TEXTURE_UNIT);
    }
    else if (m_pNormalMap)
    {
        m_pNormalMap->Bind(NORMAL_TEXTURE_UNIT);
    }

    if (m_pHeightMap)
    {
        m_pHeightMap->Bind(HEIGHT_TEXTURE_UNIT);
    }
}

void GLModel::Render(u_int drawIndex, u_int primID)
{
    glBindVertexArray(m_VAO);

    if (usePVP)
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_INDEX_VERTICES, m_buffers[VERTEX_BUFFER]);
    }

    int materialIndex = m_meshes[drawIndex].MaterialIndex;
    assert((materialIndex >= 0) && (materialIndex < m_materials.size()));

    if (m_materials[materialIndex].pTextures[TEX_TYPE_BASE])
    {
        m_materials[materialIndex].pTextures[TEX_TYPE_BASE]->Bind(COLOR_TEXTURE_UNIT);
    }

    if (m_materials[materialIndex].pTextures[TEX_TYPE_SPECULAR])
    {
        m_materials[materialIndex].pTextures[TEX_TYPE_SPECULAR]->Bind(SPECULAR_EXPONENT_UNIT);
    }

    glDrawElementsBaseVertex(GL_TRIANGLES, 3, GL_UNSIGNED_INT,
                             (void *)(sizeof(u_int) * (m_meshes[drawIndex].BaseIndex + primID * 3)),
                             m_meshes[drawIndex].BaseVertex);

    glBindVertexArray(0);
}

// used only by instancing
void GLModel::Render(u_int numInstances, const glm::mat4 *WVPs, const glm::mat4 *worlds)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[WVP_MAT_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * numInstances, WVPs, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_buffers[WORLD_MAT_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * numInstances, worlds, GL_DYNAMIC_DRAW);

    glBindVertexArray(m_VAO);

    if (usePVP)
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_INDEX_VERTICES, m_buffers[VERTEX_BUFFER]);
    }

    for (u_int i = 0; i < m_meshes.size(); i++)
    {
        int materialIndex = m_meshes[i].MaterialIndex;
        assert((materialIndex >= 0) && (materialIndex < m_materials.size()));

        if (m_materials[materialIndex].pTextures[TEX_TYPE_BASE])
        {
            m_materials[materialIndex].pTextures[TEX_TYPE_BASE]->Bind(COLOR_TEXTURE_UNIT);
        }

        if (m_materials[materialIndex].pTextures[TEX_TYPE_SPECULAR])
        {
            m_materials[materialIndex].pTextures[TEX_TYPE_SPECULAR]->Bind(SPECULAR_EXPONENT_UNIT);
        }

        glDrawElementsInstancedBaseVertex(GL_TRIANGLES, m_meshes[i].NumIndices,
                                          GL_UNSIGNED_INT, (void *)(sizeof(u_int) * m_meshes[i].BaseIndex),
                                          numInstances, m_meshes[i].BaseVertex);
    }

    glBindVertexArray(0);
}

void GLModel::RenderIndirect(const glm::mat4 &objectMatrix)
{
    assert(useIndirectRender);

    glBindVertexArray(m_VAO);

    if (usePVP)
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_INDEX_VERTICES, m_buffers[VERTEX_BUFFER]);
    }

    m_indirectRender.Render(objectMatrix);

    glBindVertexArray(0);
}

void GLModel::SetColorTexture(int textureHandle)
{
    BaseTexture *pTexture = NULL;

    if (textureHandle >= 0)
    {
        pTexture = m_pCoreRenderingSystem->GetTexture(textureHandle);
    }

    if (m_materials.size() == 0)
    {
        printf("SetColorTexture: no materials\n");
        assert(0);
    }

    if (m_materials[0].pTextures[TEX_TYPE_BASE])
    {
        delete m_materials[0].pTextures[TEX_TYPE_BASE];
    }

    m_materials[0].pTextures[TEX_TYPE_BASE] = (Texture *)pTexture;
}

void GLModel::SetNormalMap(int textureHandle)
{
    if (textureHandle < 0)
    {
        m_pNormalMap = NULL;
    }
    else
    {
        BaseTexture *pTexture = m_pCoreRenderingSystem->GetTexture(textureHandle);
        m_pNormalMap = (Texture *)pTexture;

        // hack
        if (m_materials.size() > 0)
        {
            if (m_materials[0].pTextures[TEX_TYPE_NORMAL])
            {
                delete m_materials[0].pTextures[TEX_TYPE_NORMAL];
            }

            m_materials[0].pTextures[TEX_TYPE_NORMAL] = m_pNormalMap;

            if (useIndirectRender)
            {
                m_indirectRender.RefreshMaterials(m_materials);
            }
        }
    }
}

void GLModel::SetHeightMap(int textureHandle)
{
    if (textureHandle < 0)
    {
        m_pHeightMap = NULL;
    }
    else
    {
        BaseTexture *pTexture = m_pCoreRenderingSystem->GetTexture(textureHandle);
        m_pHeightMap = (Texture *)pTexture;
    }
}

void GLModel::SetAmbientOcclusionMap(int textureHandle)
{
    if (textureHandle < 0)
    {
        m_pAOMap = NULL;
    }
    else
    {
        BaseTexture *pTexture = m_pCoreRenderingSystem->GetTexture(textureHandle);
        m_pAOMap = (Texture *)pTexture;
    }
}

Texture *GLModel::AllocTexture2D()
{
    return new Texture(GL_TEXTURE_2D);
}

void GLModel::DestroyTexture(Texture *pTexture)
{
    delete pTexture;
}
