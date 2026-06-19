#include <GL/glew.h>
#include <glm/glm.hpp>
#include "glfw.h"
#include "camera.h"
#include "lighting_technique.h"
#include "simple_color_technique.h"
#include "picking_texture.h"
#include "picking_technique.h"

class App
{
public:
    App();
    virtual ~App();

    void Init();
    void Run();
    void RenderSceneCB();
    void PickingPhase();
    void RenderPhase();

    void KeyBoardCB(unsigned int key, int state);
    void PassiveMouseCB(int x, int y);
    void MouseCB(int button, int action, int x, int y);

private:
    void CreateWindow();
    void InitCallbacks();
    void InitCamera();
    void InitShaders();
    void InitMesh();
    
    GLFWwindow *window = NULL;
    LightingTechnique m_lightingEffect;
    PickingTechnique m_pickingEffect;
    SimpleColorTechnique m_simpleColorEffect;
    Camera *m_pGameCamera = NULL;
    DirectionalLight m_directionalLight;
    Mesh *pMesh = NULL;
    PickingTexture m_pickingTexture;

    struct
    {
        bool IsPressed = false;
        int x;
        int y;
    } m_leftMouseButton;

    glm::vec3 m_worldPos[3];
};