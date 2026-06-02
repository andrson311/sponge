#include <GL/glew.h>
#include <glm/glm.hpp>

#include "../includes/stb_image.h"

#include "texture.h"
#include "camera.h"
#include "world_transform.h"
#include "mesh.h"
#include "lighting_technique.h"

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
    GLuint WVPLocation;
    GLuint SamplerLocation;
    Camera *pGameCamera = NULL;
    Mesh* pMesh = NULL;
    LightingTechnique* pLightingTech = NULL;
    BaseLight baseLight;
    
    float FOV   = 45.0f;
    float zNear = 1.0f;
    float zFar  = 100.0f;
};
