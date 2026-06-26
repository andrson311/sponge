#pragma once

#include "terrain.h"

class MidpointDispTerrain : public BaseTerrain
{
public:
    MidpointDispTerrain() {}

    void CreatMidpointDisplacement(int Size, float Roughness, float MinHeight, float MaxHeight);

private:
    void CreateMidpointDisplacementF32(float Roughness);
    void DiamondStep(int RectSize, float CurHeight);
    void SquareStep(int RectSize, float CurHeight);
};
