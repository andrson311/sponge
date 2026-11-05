#include "gl_ssbo_db.h"
#include "gl_indirect_render.h"

struct PerObjectData
{
    glm::mat4 WorldMatrix;
    glm::mat4 NormalMatrix;
    glm::ivec4 MaterialIndex = glm::ivec4(0);
};

struct DrawElementsIndirectCommand
{
    u_int Count = 0;
    u_int InstanceCount = 0;
    u_int FirstIndex = 0;
    int BaseVertex = 0;
    u_int BaseInstance = 0;
};

void IndirectRender::Init(const std::vector<BasicMeshEntry> &meshes,
                          const std::vector<CoreMaterial> &materials)
{
    assert(meshes.size() > 0);

    InitMeshes(meshes);
    InitDrawCmdsBuffer(meshes);
    AllocPerObjectBuffer(meshes);
    PrepareIndirectRenderMaterials(materials);
}

void IndirectRender::InitMeshes(const std::vector<BasicMeshEntry> &meshes)
{
    m_meshes.resize(meshes.size());

    for (int i = 0; i < meshes.size(); i++)
    {
        m_meshes[i].m_transformation = meshes[i].Transformation;
        m_meshes[i].m_materialIndex = meshes[i].MaterialIndex;
    }
}

void IndirectRender::InitDrawCmdsBuffer(const std::vector<BasicMeshEntry> &meshes)
{
    std::vector<DrawElementsIndirectCommand> drawCommands;
    drawCommands.resize(meshes.size());

    for (int i = 0; i < meshes.size(); i++)
    {
        DrawElementsIndirectCommand cmd;
        cmd.Count = meshes[i].NumIndices;
        cmd.InstanceCount = 1;
        cmd.FirstIndex = meshes[i].BaseIndex;
        cmd.BaseVertex = meshes[i].BaseVertex;
        cmd.BaseInstance = 0;

        drawCommands[i] = cmd;
    }

    glCreateBuffers(1, &m_drawCmdBuffer);
    glNamedBufferStorage(m_drawCmdBuffer,
                         sizeof(drawCommands[0]) * drawCommands.size(),
                         (const void *)drawCommands.data(), 0);
}

void IndirectRender::AllocPerObjectBuffer(const std::vector<BasicMeshEntry> &meshes)
{
    glCreateBuffers(1, &m_perObjectBuffer);
    glNamedBufferStorage(m_perObjectBuffer, sizeof(PerObjectData) * meshes.size(),
                         NULL, GL_DYNAMIC_STORAGE_BIT);
}

void IndirectRender::PrepareIndirectRenderMaterials(const std::vector<CoreMaterial> &materials)
{
    int numMaterials = (int)materials.size();
    m_materials.resize(numMaterials);

    for (int i = 0; i < numMaterials; i++)
    {
        m_materials[i].Color.AmbientColor = materials[i].AmbientColor;
        m_materials[i].Color.DiffuseColor = materials[i].DiffuseColor;
        m_materials[i].Color.SpecularColor = materials[i].SpecularColor;

        if (materials[i].pTextures[TEX_TYPE_BASE] &&
            (materials[i].pTextures[TEX_TYPE_BASE]->GetBindlessHandle() == -1))
        {
            printf("Diffuse texture exists but bindless handle is missing\n");
            exit(1);
        }

        GLuint64 diffuseMapBindlessHandle = materials[i].pTextures[TEX_TYPE_BASE]
                                                ? materials[i].pTextures[TEX_TYPE_BASE]->GetBindlessHandle()
                                                : -1;
        m_materials[i].DiffuseMap = diffuseMapBindlessHandle;

        GLuint64 normalMapBindlessHandle = materials[i].pTextures[TEX_TYPE_NORMAL]
                                               ? materials[i].pTextures[TEX_TYPE_NORMAL]->GetBindlessHandle()
                                               : -1;
        m_materials[i].NormalMap = normalMapBindlessHandle;
    }

    glCreateBuffers(1, &m_materialsBuffer);
    glNamedBufferStorage(m_materialsBuffer, sizeof(m_materials[0]) * m_materials.size(),
                         m_materials.data(), GL_DYNAMIC_STORAGE_BIT);
}

void IndirectRender::Render(const glm::mat4 &objectMatrix)
{
    UpdatePerObjectData(objectMatrix);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_drawCmdBuffer);
    glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, NULL, (GLsizei)m_meshes.size(), 0);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}

void IndirectRender::RefreshMaterials(const std::vector<CoreMaterial> &materials)
{
    int numMaterials = (int)materials.size();

    for (int i = 0; i < numMaterials; i++)
    {
        m_materials[i].Color.AmbientColor = materials[i].AmbientColor;
        m_materials[i].Color.DiffuseColor = materials[i].DiffuseColor;
        m_materials[i].Color.SpecularColor = materials[i].SpecularColor;

        if (materials[i].pTextures[TEX_TYPE_BASE] &&
            (materials[i].pTextures[TEX_TYPE_BASE]->GetBindlessHandle() == -1))
        {
            printf("Diffuse texture exists but bindless handle is missing\n");
            exit(1);
        }

        GLuint64 diffuseMapBindlessHandle = materials[i].pTextures[TEX_TYPE_BASE]
                                                ? materials[i].pTextures[TEX_TYPE_BASE]->GetBindlessHandle()
                                                : -1;
        m_materials[i].DiffuseMap = diffuseMapBindlessHandle;

        GLuint64 normalMapBindlessHandle = materials[i].pTextures[TEX_TYPE_NORMAL]
                                               ? materials[i].pTextures[TEX_TYPE_NORMAL]->GetBindlessHandle()
                                               : -1;
        m_materials[i].NormalMap = normalMapBindlessHandle;
    }

    glNamedBufferSubData(m_materialsBuffer, 0,
                         sizeof(m_materials[0]) * m_materials.size(), m_materials.data());
}

void IndirectRender::UpdatePerObjectData(const glm::mat4 &objectMatrix)
{
    std::vector<PerObjectData> perObjectDataVector;
    perObjectDataVector.resize(m_meshes.size());

    for (int i = 0; i < m_meshes.size(); i++)
    {
        glm::mat4 finalWorldMatrix = objectMatrix * m_meshes[i].m_transformation;
        glm::mat4 worldInverse = glm::inverse(finalWorldMatrix);
        glm::mat4 worldInverseTranspose = glm::transpose(worldInverse);

        perObjectDataVector[i].WorldMatrix = finalWorldMatrix;
        perObjectDataVector[i].NormalMatrix = worldInverseTranspose;
        perObjectDataVector[i].MaterialIndex.x = m_meshes[i].m_materialIndex;
    }

    glNamedBufferSubData(m_perObjectBuffer, 0,
                         sizeof(perObjectDataVector[0]) * perObjectDataVector.size(),
                         perObjectDataVector.data());

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_INDEX_PER_OBJ_DATA, m_perObjectBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SSBO_INDEX_MATERIALS, m_perObjectBuffer);
}
