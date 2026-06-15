#include <GL/glew.h>
#include <glm/glm.hpp>

#include "stb_image.h"
#include "texture.h"
#include "camera.h"
#include "world_transform.h"
#include "mesh.h"
#include "mesh_skinned.h"
#include "lighting_technique.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

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
    MeshSkinned* pMesh1 = NULL;
    LightingTechnique* pLightingTech = NULL;
    DirectionalLight dirLight;
    PointLight pointLights[LightingTechnique::MAX_POINT_LIGHTS];
    SpotLight spotLights[LightingTechnique::MAX_SPOT_LIGHTS];
    long long StartTimeMillis = 0;
    
    float FOV   = 45.0f;
    float zNear = 1.0f;
    float zFar  = 100.0f;

    float counter = 0.0f;
    int DisplayBoneIndex = 0;
};
