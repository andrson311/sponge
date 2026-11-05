#pragma once

#include <vector>
#include "math_3d.h"
#include "core_material.h"
#include "gl_basic_mesh_entry.h"

class IndirectRender
{
public:
    IndirectRender() {}
    ~IndirectRender() {}

    void Init(const std::vector<BasicMeshEntry> &meshes, const std::vector<CoreMaterial> &materials);
    void Render(const glm::mat4 &objectMatrix);
    void RefreshMaterials(const std::vector<CoreMaterial> &materials);

private:
    void InitMeshes(const std::vector<BasicMeshEntry> &meshes);
    void InitDrawCmdsBuffer(const std::vector<BasicMeshEntry> &meshes);
    void AllocPerObjectBuffer(const std::vector<BasicMeshEntry> &meshes);
    void UpdatePerObjectData(const glm::mat4 &objectMatrix);
    void PrepareIndirectRenderMaterials(const std::vector<CoreMaterial> &materials);

    struct MaterialColorIndirect
    {
        glm::vec4 AmbientColor;
        glm::vec4 DiffuseColor;
        glm::vec4 SpecularColor;
    };

    struct MaterialIndirect
    {
        MaterialColorIndirect Color;
        GLuint64 DiffuseMap;
        GLuint64 NormalMap;
    };

    std::vector<MaterialIndirect> m_materials;

    GLuint m_drawCmdBuffer = 0;
    GLuint m_perObjectBuffer = 0;
    GLuint m_materialsBuffer = 0;

    struct Mesh
    {
        glm::mat4 m_transformation;
        int m_materialIndex = 0;
    };

    std::vector<Mesh> m_meshes;
};
