#pragma once

#include <GL/glew.h>
#include "skydome_technique.h"
#include "camera.h"
#include "texture.h"

class Skydome
{
public:
    Skydome(int NumPitchStripes, int NumHeadingStripes, float Radius,
            const char *pTextureFilename, GLenum TextureUnit, int TextureUnitIndex);

    void Render(const Camera &Camera);

private:
    struct Vertex
    {
        glm::vec3 Pos;
        glm::vec2 Tex;

        Vertex() {}
        Vertex(const glm::vec3 &p);
    };

    void CreateGLState();
    void PopulateBuffers(int NumRows, int NumCols, float Radius);
    void LoadTexture(const char* pTextureFilename);

    int m_numVertices = 0;
    GLuint m_vao;
    GLuint m_vb;
    Texture m_texture;
    SkydomeTechnique m_skydomeTech;
    int m_textureUnit = 0;
    GLenum m_textureUnitIndex = 0;
};
