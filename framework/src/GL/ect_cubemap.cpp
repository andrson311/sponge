#include <gli/gli.hpp>
#include "math_3d.h"
#include "ext_cubemap.h"

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
    CUBE_MAP_INDEX_NEG_Z,
};

#define CUBEMAP_NUM_FACES 6

static glm::vec3 FaceCoordsToXYZ(int x, int y, int faceID, int faceSize)
{
    float a = 2.0f * float(x) / faceSize;
    float b = 2.0f * float(y) / faceSize;

    glm::vec3 ret;

    switch (faceID)
    {
    case CUBE_MAP_INDEX_POS_X:
        ret = glm::vec3(a - 1.0f, 1.0f, 1.0f - b);
        break;

    case CUBE_MAP_INDEX_NEG_X:
        ret = glm::vec3(1.0f - a, -1.0f, 1.0f - b);
        break;

    case CUBE_MAP_INDEX_POS_Y:
        ret = glm::vec3(1.0f - b, a - 1.0f, 1.0f);
        break;

    case CUBE_MAP_INDEX_NEG_Y:
        ret = glm::vec3(b - 1.0f, a - 1.0f, -1.0f);
        break;

    case CUBE_MAP_INDEX_POS_Z:
        ret = glm::vec3(-1.0f, a - 1.0f, 1.0f - b);
        break;

    case CUBE_MAP_INDEX_NEG_Z:
        ret = glm::vec3(1.0f, 1.0f - a, 1.0f - b);
        break;

    default:
        assert(0);
    }

    return ret;
}

int ConvertEquirectangularImageToCubemap(const Bitmap &b, std::vector<Bitmap> &cubemap)
{
    int faceSize = b.w_ / 4;
    cubemap.resize(CUBEMAP_NUM_FACES);

    for (int i = 0; i < CUBEMAP_NUM_FACES; i++)
    {
        cubemap[i].Init(faceSize, faceSize, b.comp_, b.fmt_);
    }

    int maxW = b.w_ - 1;
    int maxH = b.h_ - 1;

    for (int face = 0; face < CUBEMAP_NUM_FACES; face++)
    {
        for (int y = 0; y < faceSize; y++)
        {
            for (int x = 0; x < faceSize; x++)
            {
                glm::vec3 p = FaceCoordsToXYZ(x, y, face, faceSize);
                float r = sqrtf(p.x * p.x + p.y * p.y);
                float phi = atan2f(p.y, p.x);
                float theta = atan2f(p.z, r);

                float u = (float)((phi + M_PI) / (2.0f * M_PI));
                float v = (float((M_PI / 2.0f - theta) / M_PI));

                float U = u * b.w_;
                float V = v * b.h_;

                // 4-samples for bilinear interpolation
                int U1 = std::clamp(int(floor(U)), 0, maxW);
                int V1 = std::clamp(int(floor(V)), 0, maxH);
                int U2 = std::clamp(U1 + 1, 0, maxW);
                int V2 = std::clamp(V1 + 1, 0, maxH);

                // Calculate the fractional part
                float s = U - U1;
                float t = V - V1;

                // Fetch 4-samples
                glm::vec4 BottomLeft = b.getPixel(U1, V1);
                glm::vec4 BottomRight = b.getPixel(U2, V1);
                glm::vec4 TopLeft = b.getPixel(U1, V2);
                glm::vec4 TopRight = b.getPixel(U2, V2);

                // Bilinear interpolation
                glm::vec4 color = BottomLeft * (1 - s) * (1 - t) +
                                  BottomRight * (s) * (1 - t) +
                                  TopLeft * (1 - s) * t +
                                  TopRight * (s) * (t);

                cubemap[face].setPixel(x, y, color);
            }
        }
    }

    return faceSize;
}