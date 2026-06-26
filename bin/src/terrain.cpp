#include "terrain.h"

BaseTerrain::~BaseTerrain()
{
    Destroy();
}

void BaseTerrain::Destroy()
{
    m_heightMap.Destroy();
    m_triangleList.Destroy();
}

void BaseTerrain::InitTerrain(float WorldScale)
{
    if (!m_terrainTech.Init())
    {
        printf("Error initializing terrain tech\n");
        exit(0);
    }

    m_worldScale = WorldScale;
}

void BaseTerrain::LoadFromFile(const char *pFilename)
{
    LoadHeightMapFile(pFilename);
    m_triangleList.CreateTriangleList(m_terrainSize, m_terrainSize, this);
}

void BaseTerrain::LoadHeightMapFile(const char *pFilename)
{
    int FileSize = 0;
    u_char *p = (u_char *)ReadBinaryFile(pFilename, FileSize);

    if (FileSize % sizeof(float) != 0)
    {
        printf("%s:%d - '%s' does not contain an whole number of floats (size %d)\n", __FILE__, __LINE__, pFilename, FileSize);
        exit(0);
    }

    m_terrainSize = (int)sqrtf((float)FileSize / (float)sizeof(float));

    printf("Terrain size %d\n", m_terrainSize);

    if ((m_terrainSize * m_terrainSize) != (FileSize / sizeof(float)))
    {
        printf("%s:%d - '%s' does not contain a square height map - size %d\n", __FILE__, __LINE__, pFilename, FileSize);
        exit(0);
    }

    m_heightMap.InitArray2D(m_terrainSize, m_terrainSize, (float *)p);
}

void BaseTerrain::Render(const Camera &Camera)
{
    glm::mat4 VP = Camera.GetViewProjMatrix();

    m_terrainTech.Enable();
    m_terrainTech.SetVP(VP);

    m_triangleList.Render();
}