#pragma once

#include <string>
#include "math_3d.h"

struct BasicMeshEntry
{
    u_int NumIndices = 0;
    u_int NumVertices = 0;
    u_int BaseVertex = 0;
    u_int BaseIndex = 0;
    u_int ValidFaces = 0;
    int MaterialIndex = -1;
    glm::mat4 Transformation;
    std::string Name;
};
