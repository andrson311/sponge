#pragma once

#include <iostream>
#include <GL/glew.h>
#include <glm/glm.hpp>

class PickingTexture
{
public:
    PickingTexture() {}
    ~PickingTexture();

    void Init(u_int WindowWidth, u_int WindowHeight);
    void EnableWriting();
    void DisableWriting();

    struct PixelInfo
    {
        u_int ObjectID = 0;
        u_int DrawID = 0;
        u_int PrimID = 0;

        void Print()
        {
            printf("Object %d draw %d prim %d\n", ObjectID, DrawID, PrimID);
        }
    };

    PixelInfo ReadPixel(u_int x, u_int y);

private:
    GLuint m_fbo = 0;
    GLuint m_pickingTexture = 0;
    GLuint m_depthTexture = 0;
};
