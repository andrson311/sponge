#pragma once
#include <iostream>

struct PersProjInfo
{
    float FOV = 0.0f;
    float Width = 0.0f;
    float Height = 0.0f;
    float zNear = 0.0f;
    float zFar = 0.0f;
};


struct OrthoProjInfo
{
    float r;        // right
    float l;        // left
    float b;        // bottom
    float t;        // top
    float n;        // z near
    float f;        // z far

    float Width;
    float Height;    

    void Print()
    {
        printf("Left %f   Right %f\n", l, r);
        printf("Bottom %f Top %f\n", b, t);
        printf("Near %f   Far %f\n", n, f);
    }
};
