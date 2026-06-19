#include <cassert>
#include <map>
#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices)

#define MAX_NUM_BONES_PER_VERTEX 4

struct VertexBoneData
{
    unsigned int BoneIDs[MAX_NUM_BONES_PER_VERTEX] = {0};
    float Weights[MAX_NUM_BONES_PER_VERTEX] = {0.0f};

    VertexBoneData() {}

    void AddBoneData(unsigned int BoneID, float Weight)
    {
        for (unsigned int i = 0; i < std::size(BoneIDs); i++)
        {
            if (Weights[i] == 0.0)
            {
                BoneIDs[i] = BoneID;
                Weights[i] = Weight;
                printf("bone %d weight %f index %i\n", BoneID, Weight, i);
                return;
            }
        }

        assert(0);
    }
};

std::vector<VertexBoneData> vertex_to_bones;
std::vector<int> mesh_base_vertex;
std::map<std::string, unsigned int> bone_name_to_index_map;

int GetBoneId(const aiBone *pBone)
{
    int bone_id = 0;
    std::string bone_name(pBone->mName.C_Str());

    if (bone_name_to_index_map.find(bone_name) == bone_name_to_index_map.end())
    {
        bone_id = (int)bone_name_to_index_map.size();
        bone_name_to_index_map[bone_name] = bone_id;
    }
    else
    {
        bone_id = bone_name_to_index_map[bone_name];
    }

    return bone_id;
}

void ParseSingleBone(int mesh_index, const aiBone *pBone)
{
    printf("Bone '%s': num vertices affected by this bone: %d\n", pBone->mName.C_Str(), pBone->mNumWeights);

    int bone_id = GetBoneId(pBone);
    printf("bone id %d\n", bone_id);

    for (unsigned int i = 0; i < pBone->mNumWeights; i++)
    {
        const aiVertexWeight &vw = pBone->mWeights[i];
        // printf("%d: vertex id %d weight %.2f\n", i, vw.mVertexId, vw.mWeight);

        unsigned int global_vertex_id = mesh_base_vertex[mesh_index] + vw.mVertexId;
        printf("Vertex id %d ", global_vertex_id);

        assert(global_vertex_id < vertex_to_bones.size());
        vertex_to_bones[global_vertex_id].AddBoneData(bone_id, vw.mWeight);
    }
}

void ParseMeshBones(int mesh_index, const aiMesh *pMesh)
{
    for (unsigned int i = 0; i < pMesh->mNumBones; i++)
    {
        ParseSingleBone(mesh_index, pMesh->mBones[i]);
    }
}

void ParseMeshes(const aiScene *pScene)
{
    printf("Parsing %d meshes \n\n", pScene->mNumMeshes);

    int total_vertices = 0;
    int total_indices = 0;
    int total_bones = 0;

    mesh_base_vertex.resize(pScene->mNumMeshes);

    for (unsigned int i = 0; i < pScene->mNumMeshes; i++)
    {
        const aiMesh *pMesh = pScene->mMeshes[i];
        int num_vertices = pMesh->mNumVertices;
        int num_indices = pMesh->mNumFaces * 3;
        int num_bones = pMesh->mNumBones;

        printf("Mesh %d '%s': vertices %d indices %d bones %d\n", i, pMesh->mName.C_Str(), num_vertices, num_indices, num_bones);

        mesh_base_vertex[i] = total_vertices;
        total_vertices += num_vertices;
        total_indices += num_indices;
        total_bones += num_bones;

        vertex_to_bones.resize(total_vertices);

        if (pMesh->HasBones())
        {
            ParseMeshBones(i, pMesh);
        }
    }

    printf("\nTotal vertices %d total indices %d total bones %d\n", total_vertices, total_indices, total_bones);
}

void ParseScene(const aiScene *pScene)
{
    ParseMeshes(pScene);
}

int main(int argc, char *argv[])
{
    const char *filename = "assets/example/example.glb";

    Assimp::Importer Importer;
    const aiScene *pScene = Importer.ReadFile(filename, ASSIMP_LOAD_FLAGS);

    if (!pScene)
    {
        printf("Error parsing '%s': '%s'\n", filename, Importer.GetErrorString());
        return 1;
    }

    ParseScene(pScene);

    return 0;
}
