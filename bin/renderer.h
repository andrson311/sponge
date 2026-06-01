#include <GL/glew.h>
#include <glm/glm.hpp>

#include "../includes/stb_image.h"

#include "texture.h"
#include "camera.h"
#include "world_transform.h"

struct Vertex
{
    glm::vec3 pos;
    glm::vec2 tex;

    Vertex() {}

    Vertex(const glm::vec3 &pos_, const glm::vec2 &tex_)
    {
        pos = pos_;
        tex = tex_;
    }
};

struct PersProjInfo
{
    float FOV;
    float Width;
    float Height;
    float zNear;
    float zFar;
};

class Renderer
{
public:
    Renderer();
    ~Renderer();

    bool Init();

    void RenderSceneCB();
    void KeyboardCB(u_char key, int mouse_x, int mouse_y);
    void SpecialKeyboardCB(int key, int mouse_x, int mouse_y);
    void PassiveMouseCB(int x, int y);

private:
    void CreateCubeVAO();
    void CreatePyramidVAO();
    void CompileShaders();
    void AddShader(GLuint ShaderProgram, const char *pShaderText, GLenum ShaderType);

    GLuint CubeVAO = -1;
    GLuint CubeVBO = -1;
    GLuint CubeIBO = -1;

    GLuint PyramidVAO = -1;
    GLuint PyramidVBO = -1;
    GLuint PyramidIBO = -1;

    GLuint WVPLocation;
    GLuint SamplerLocation;
    Texture *pTexture = NULL;
    Camera *pGameCamera = NULL;
    WorldTrans CubeWorldTransform;
    PersProjInfo persProjInfo;
};
