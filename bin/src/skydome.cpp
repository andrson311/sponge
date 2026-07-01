#include <vector>
#include "skydome.h"

Skydome::Skydome(int NumRows, int NumCols, float Radius,
                 const char *pTextureFilename, GLenum TextureUnit, int TextureUnitIndex) : m_texture(GL_TEXTURE_2D)
{
    m_textureUnit = TextureUnit;
    m_textureUnitIndex = TextureUnitIndex;

    CreateGLState();
    PopulateBuffers(NumRows, NumCols, Radius);
    LoadTexture(pTextureFilename);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (!m_skydomeTech.Init())
    {
        exit(0);
    }

    m_skydomeTech.Enable();
    m_skydomeTech.SetTextureUnit(TextureUnitIndex);
}

Skydome::Vertex::Vertex(const glm::vec3 &p)
{
    Pos = p;
    glm::vec3 pn = glm::normalize(p);

    float pi = glm::pi<float>();
    Tex.x = asinf(pn.x) / pi + 0.5f;
    Tex.y = asinf(pn.y) / pi + 0.5f;
}

void Skydome::CreateGLState()
{
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vb);
    glBindBuffer(GL_ARRAY_BUFFER, m_vb);

    int pos_loc = 0;
    int tex_loc = 1;

    size_t Offset = 0;

    glEnableVertexAttribArray(pos_loc);
    glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)Offset);
    Offset += 3;

    glEnableVertexAttribArray(tex_loc);
    glVertexAttribPointer(tex_loc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)(Offset * sizeof(float)));
    Offset += 2;
}

void Skydome::PopulateBuffers(int NumRows, int NumCols, float Radius)
{
    int NumVerticesTopStrip = 3 * NumCols;
    int NumVerticesRegularStrip = 6 * NumCols;
    m_numVertices = NumVerticesTopStrip + (NumRows - 1) * NumVerticesRegularStrip;

    std::vector<Vertex> Vertices(m_numVertices);

    float PitchAngle = 90.0f / (float)NumRows;
    float HeadingAngle = 360.0f / (float)NumCols;

    glm::vec3 Apex(0.0f, Radius, 0.0f);
    float Pitch = -90.0f;
    int i = 0;

    for (float Heading = 0.0f; Heading < 360.0f; Heading += HeadingAngle)
    {
        Vertex v0(Apex);
        Vertices[i++] = v0;

        glm::vec3 Pos1 = SphericalToCartesian(Radius, Pitch + PitchAngle, Heading + HeadingAngle);
        Vertex v1(Pos1);
        Vertices[i++] = v1;

        glm::vec3 Pos2 = SphericalToCartesian(Radius, Pitch + PitchAngle, Heading);
        Vertex v2(Pos2);
        Vertices[i++] = v2;
    }

    for (Pitch = -90.0f + PitchAngle; Pitch < 0.0f; Pitch += PitchAngle)
    {
        for (float Heading = 0.0f; Heading < 360.0f; Heading += HeadingAngle)
        {
            glm::vec3 Pos0 = SphericalToCartesian(Radius, Pitch, Heading);
            Vertex v0(Pos0);

            glm::vec3 Pos1 = SphericalToCartesian(Radius, Pitch, Heading + HeadingAngle);
            Vertex v1(Pos1);

            glm::vec3 Pos2 = SphericalToCartesian(Radius, Pitch + PitchAngle, Heading);
            Vertex v2(Pos2);

            glm::vec3 Pos3 = SphericalToCartesian(Radius, Pitch + PitchAngle, Heading + HeadingAngle);
            Vertex v3(Pos3);

            assert(i + 6 <= m_numVertices);

            Vertices[i++] = v0;
            Vertices[i++] = v1;
            Vertices[i++] = v2;

            Vertices[i++] = v1;
            Vertices[i++] = v3;
            Vertices[i++] = v2;
        }
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices[0]) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);
}

void Skydome::LoadTexture(const char *pTextureFilename)
{
    std::string s(pTextureFilename);
    m_texture.Load(s);
}

void Skydome::Render(const Camera &Camera)
{
    m_skydomeTech.Enable();

    static float foo = 0.0f;
    foo += 0.01f;

    glm::mat4 Rotate = glm::eulerAngleXYX(0.0f, glm::radians(foo), 0.0f);

    glm::mat4 World = glm::translate(glm::mat4(1.0f), Camera.GetPos() - glm::vec3(0.0f, 0.2f, 0.0f));
    glm::mat4 View = Camera.GetMatrix();
    glm::mat4 Proj = Camera.GetProjectionMat();
    glm::mat4 WVP = Proj * View * World * Rotate;
    m_skydomeTech.SetWVP(WVP);

    m_texture.Bind(m_textureUnit);
    GLint OldDepthFuncMode;
    glGetIntegerv(GL_DEPTH_FUNC, &OldDepthFuncMode);
    glDepthFunc(GL_LEQUAL);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_numVertices);
    glBindVertexArray(0);
    glDepthFunc(OldDepthFuncMode);
}
