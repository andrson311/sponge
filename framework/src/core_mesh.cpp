#include <stdio.h>
#include "core_mesh.h"

MeshFileHeader LoadMeshData(const char *pMeshFile, MeshData &out)
{
    FILE *f = NULL;
    f = fopen(pMeshFile, "rb");

    if (!f)
    {
        printf("Cannot open '%s'\n", pMeshFile);
        assert(0);
        exit(1);
    }

    MeshFileHeader header;
    size_t size = fread(&header, 1, sizeof(header), f);

    if (size != sizeof(header))
    {
        printf("Unable to read mesh file header.\n");
        assert(0);
        exit(1);
    }

    out.m_vertexSize = header.m_vertexSizeBytes;
    out.m_totalIndices = header.m_numIndices;
    out.m_totalVertices = header.m_numVertices;

    out.m_meshes.resize(header.m_meshCount);

    size = fread(out.m_meshes.data(), sizeof(Mesh), header.m_meshCount, f);

    if (size != header.m_meshCount)
    {
        printf("Could not read mesh descriptors.\n");
        assert(0);
        exit(1);
    }

    out.m_boxes.resize(header.m_meshCount);
    size = fread(out.m_boxes.data(), sizeof(BoundingBox), header.m_meshCount, f);

    if (size != header.m_meshCount)
    {
        printf("Could not read bounding m_boxes.\n");
        assert(0);
        exit(1);
    }

    out.m_indexData.Size = header.m_indexDataSizeBytes;
    out.m_indexData.pMem = (char*)malloc(header.m_indexDataSizeBytes);

    size = fread(out.m_indexData.pMem, 1, header.m_indexDataSizeBytes, f);

    if (size != header.m_indexDataSizeBytes)
    {
        printf("Unable to read index data.\n");
        assert(0);
        exit(1);
    }

    out.m_vertexData.Size = header.m_vertexDataSizeBytes;
    out.m_vertexData.pMem = (char *)malloc(header.m_vertexDataSizeBytes);

    size = fread(out.m_vertexData.pMem, 1, header.m_vertexDataSizeBytes, f);

    if (size != header.m_vertexDataSizeBytes)
    {
        printf("Could not read vertex data\n");
        assert(0);
        exit(1);
    }

    fclose(f);

    return header;
}

void SaveMeshData(const char *pFilename, const MeshData &m)
{
    FILE *f = NULL;
    f = fopen(pFilename, "wb");

    if (!f)
    {
        printf("Cannot open '%s'\n", pFilename);
        assert(0);
        exit(1);
    }

    MeshFileHeader header = {
        .m_meshCount = (u_int)m.m_meshes.size(),
        .m_numIndices = m.m_totalIndices,
        .m_indexDataSizeBytes = (u_int)(m.m_indexData.Size),
        .m_numVertices = m.m_totalVertices,
        .m_vertexDataSizeBytes = (u_int)(m.m_vertexData.Size),
        .m_vertexSizeBytes = m.m_vertexSize};

    fwrite(&header, 1, sizeof(header), f);
    fwrite(m.m_meshes.data(), sizeof(Mesh), header.m_meshCount, f);
    fwrite(m.m_boxes.data(), sizeof(BoundingBox), header.m_meshCount, f);
    fwrite(m.m_indexData.pMem, 1, header.m_indexDataSizeBytes, f);
    fwrite(m.m_vertexData.pMem, 1, header.m_vertexDataSizeBytes, f);

    fclose(f);

    printf("Mesh saved to '%s'\n", pFilename);
}
