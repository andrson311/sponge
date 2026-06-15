#pragma once

#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices)

#define INVALID_UNIFORM_LOCATION 0xFFFFFFFF

#define COLOR_TEXTURE_UNIT GL_TEXTURE0
#define COLOR_TEXTURE_UNIT_INDEX 0
#define SPECULAR_TEXTURE_UNIT GL_TEXTURE8
#define SPECULAR_TEXTURE_UNIT_INDEX 8

#define MAX_BONES (200)

long long GetCurrentTimeMillis();
bool ReadFile(const char *pFileName, std::string &outFile);
std::string GetDirFromFilename(const std::string &Filename);
std::string GetFullPath(const std::string &Dir, const aiString &Path);
glm::mat4 AssimpToGLM(const aiMatrix4x4 &m);