#include <GL/glew.h>
#include <glm/glm.hpp>
#include "glfw.h"
#include "camera.h"
#include "lighting_technique.h"
#include "simple_color_technique.h"
#include "picking_texture.h"
#include "picking_technique.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

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
    void HandleMouseButtonPressed();
    void DragTheObject();
    void RenderObjects();

    void KeyBoardCB(u_int key, int state);
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
    bool m_mobileCamera = false;
    DirectionalLight m_directionalLight;
    Mesh *pMesh = NULL;
    PickingTexture m_pickingTexture;

    struct
    {
        bool IsPressed = false;
        bool FirstTime = true;
        int x;
        int y;
    } m_leftMouseButton;

    glm::vec3 m_worldPos[3];

    glm::vec4 m_objViewSpacePos;
    glm::vec3 m_intersectionPoint;
    glm::vec3 m_translation;
    int m_clicked_object_id = -1;
};