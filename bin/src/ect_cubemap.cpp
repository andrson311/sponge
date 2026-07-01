#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <gli/gli.hpp>

#include "math_3d.h"
#include "ect_cubemap.h"

#define CUBE_MAP_INDEX_POS_X 0
#define CUBE_MAP_INDEX_NEG_X 1
#define CUBE_MAP_INDEX_POS_Y 2
#define CUBE_MAP_INDEX_NEG_Y 3
#define CUBE_MAP_INDEX_POS_Z 4
#define CUBE_MAP_INDEX_NEG_Z 5

static int types[6] = {
    CUBE_MAP_INDEX_POS_X,
    CUBE_MAP_INDEX_NEG_X,
    CUBE_MAP_INDEX_POS_Y,
    CUBE_MAP_INDEX_NEG_Y,
    CUBE_MAP_INDEX_POS_Z,
    CUBE_MAP_INDEX_NEG_Z};

#define CUBEMAP_NUM_FACES 6

static glm::vec3 FaceCoordsToXYZ(int x, int y, int FaceID, int FaceSize)
{
    float a = 2.0f * float(x) / FaceSize;
    float b = 2.0f * float(y) / FaceSize;

    glm::vec3 Ret;

    switch (FaceID)
    {
    case CUBE_MAP_INDEX_POS_X:
        Ret = glm::vec3(a - 1.0f, 1.0f, 1.0f - b);
        break;

    case CUBE_MAP_INDEX_NEG_X:
        Ret = glm::vec3(1.0f - a, -1.0f, 1.0f - b);
        break;

    case CUBE_MAP_INDEX_POS_Y:
        Ret = glm::vec3(1.0f - b, a - 1.0f, 1.0f);
        break;

    case CUBE_MAP_INDEX_NEG_Y:
        Ret = glm::vec3(b - 1.0f, a - 1.0f, -1.0f);
        break;

    case CUBE_MAP_INDEX_POS_Z:
        Ret = glm::vec3(-1.0f, a - 1.0f, 1.0f - b);
        break;

    case CUBE_MAP_INDEX_NEG_Z:
        Ret = glm::vec3(1.0f, 1.0f - a, 1.0f - b);
        break;

    default:
        assert(0);
    }

    return Ret;
}

int ConvertEquirectangularImageToCubemap(const Bitmap &b, std::vector<Bitmap> &Cubemap)
{
    int FaceSize = b.w_ / 4;
    Cubemap.resize(CUBEMAP_NUM_FACES);

    for (int i = 0; i < CUBEMAP_NUM_FACES; i++)
    {
        Cubemap[i].Init(FaceSize, FaceSize, b.comp_, b.fmt_);
    }

    int MaxW = b.w_ - 1;
    int MaxH = b.h_ - 1;

    float pi = glm::pi<float>();

    for (int face = 0; face < CUBEMAP_NUM_FACES; face++)
    {
        for (int y = 0; y < FaceSize; y++)
        {
            for (int x = 0; x < FaceSize; x++)
            {
                glm::vec3 p = FaceCoordsToXYZ(x, y, face, FaceSize);
                float r = sqrtf(p.x * p.x + p.y * p.y);
                float phi = atan2f(p.y, p.x);
                float theta = atan2f(p.z, r);

                float u = (float)((phi + pi) / (2.0f * pi));
                float v = ((float(pi / 2.0f - theta) / pi));

                float U = u * b.w_;
                float V = v * b.h_;

                int U1 = std::min(std::max(int(floor(U)), 0), MaxW);
                int V1 = std::min(std::max(int(floor(V)), 0), MaxH);
                int U2 = std::min(std::max(U1 + 1, 0), MaxW);
                int V2 = std::min(std::max(V1 + 1, 0), MaxH);

                float s = U - U1;
                float t = V - V1;

                glm::vec4 BottomLeft = b.getPixel(U1, V1);
                glm::vec4 BottomRight = b.getPixel(U2, V1);
                glm::vec4 TopLeft = b.getPixel(U1, V2);
                glm::vec4 TopRight = b.getPixel(U2, V2);

                glm::vec4 color = BottomLeft * (1 - s) * (1 - t) +
                                  BottomRight * s * (1 - t) +
                                  TopLeft * (1 - s) * t +
                                  TopRight * s * t;
                
                Cubemap[face].setPixel(x, y, color);
            }
        }
    }

    return FaceSize;
}
