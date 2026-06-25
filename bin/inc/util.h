#pragma once

#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define SHADOW_MAP_SIZE 4096

#define ASSIMP_LOAD_FLAGS ( \
    aiProcess_Triangulate | \
    aiProcess_GenSmoothNormals | \
    aiProcess_JoinIdenticalVertices | \
    aiProcess_PopulateArmatureData | \
    aiProcess_LimitBoneWeights | \
    aiProcess_CalcTangentSpace | \
    aiProcess_GlobalScale)

#define INVALID_UNIFORM_LOCATION 0xFFFFFFFF

#define COLOR_TEXTURE_UNIT GL_TEXTURE0
#define COLOR_TEXTURE_UNIT_INDEX 0
#define SHADOW_TEXTURE_UNIT GL_TEXTURE1
#define SHADOW_TEXTURE_UNIT_INDEX 1
#define NORMAL_TEXTURE_UNIT GL_TEXTURE2
#define NORMAL_TEXTURE_UNIT_INDEX 2
#define RANDOM_TEXTURE_UNIT GL_TEXTURE3
#define RANDOM_TEXTURE_UNIT_INDEX 3
#define DISPLACEMENT_TEXTURE_UNIT GL_TEXTURE4
#define DISPLACEMENT_TEXTURE_UNIT_INDEX 4
#define ALBEDO_TEXTURE_UNIT GL_TEXTURE5
#define ALBEDO_TEXTURE_UNIT_INDEX 5
#define ROUGHNESS_TEXTURE_UNIT GL_TEXTURE6
#define ROUGHNESS_TEXTURE_UNIT_INDEX 6
#define MOTION_TEXTURE_UNIT GL_TEXTURE7
#define MOTION_TEXTURE_UNIT_INDEX 7
#define SPECULAR_TEXTURE_UNIT GL_TEXTURE8
#define SPECULAR_TEXTURE_UNIT_INDEX 8
#define METALLIC_TEXTURE_UNIT GL_TEXTURE9
#define METALLIC_TEXTURE_UNIT_INDEX 9
#define CASCADE_SHADOW_TEXTURE_UNIT1 GL_TEXTURE10
#define CASCADE_SHADOW_TEXTURE_UNIT1_INDEX 10
#define CASCADE_SHADOW_TEXTURE_UNIT2 GL_TEXTURE11
#define CASCADE_SHADOW_TEXTURE_UNIT2_INDEX 11
#define SHADOW_CUBE_MAP_TEXTURE_UNIT GL_TEXTURE12
#define SHADOW_CUBE_MAP_TEXTURE_UNIT_INDEX 12
#define SHADOW_MAP_RANDOM_OFFSET_TEXTURE_UNIT GL_TEXTURE13
#define SHADOW_MAP_RANDOM_OFFSET_TEXTURE_UNIT_INDEX 13
#define HEIGHT_TEXTURE_UNIT GL_TEXTURE14
#define HEIGHT_TEXTURE_UNIT_INDEX 14

#define MAX_BONES (200)
#define NUM_CUBE_MAP_FACES 6

long long GetCurrentTimeMillis();
bool ReadFile(const char *pFileName, std::string &outFile);
char* ReadBinaryFile(const char* pFileName, int& size);
std::string GetDirFromFilename(const std::string &Filename);
std::string GetFullPath(const std::string &Dir, const aiString &Path);
glm::mat4 AssimpToGLM(const aiMatrix4x4 &m);