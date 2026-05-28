#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cstdint>
#include <unistd.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "utils/general.h"
#include "utils/shader_utils.h"
#include "utils/world_transform.h"
#include "utils/camera.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

GLuint VBO;
GLuint IBO;
GLint gWVPLocation;

WorldTrans CubeWorldTransform;
Camera GameCamera;

static void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static float RotationAngle = 0.02f;

    CubeWorldTransform.SetPosition(0.0f, 0.0f, -2.0f);
    CubeWorldTransform.Rotate(RotationAngle, RotationAngle, 0.0f);
    glm::mat4 World = CubeWorldTransform.GetMatrix();

    // glm::vec3 CameraPos(-1.0f, 1.0f, 1.0f);
    // glm::vec3 CameraTarget(0.0f, 0.0f, -1.0f);  // N vector (forward)
    // glm::vec3 CameraUp(0.0f, 1.0f, 0.0f);       // V vector (up)
    // glm::mat4 Camera = glm::lookAt(
    //     CameraPos, 
    //     CameraPos + CameraTarget, 
    //     CameraUp
    // );

    glm::mat4 Camera = GameCamera.GetMatrix();

    float aspectRatio = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

    glm::mat4 Projection = glm::perspective(
        glm::radians(90.0f), 
        aspectRatio, 
        0.1f, 
        10.0f
    );

    glm::mat4 WVP = Projection * Camera * World;

    glUniformMatrix4fv(
        gWVPLocation, 
        1, 
        GL_FALSE, 
        glm::value_ptr(WVP)
    );

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 
        3, 
        GL_FLOAT, 
        GL_FALSE, 
        6 * sizeof(float), 
        0
    );

    // color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 
        3, 
        GL_FLOAT, 
        GL_FALSE, 
        6 * sizeof(float), 
        (void*)(3 * sizeof(float))
    );

    glDrawElements(
        GL_TRIANGLES,
        36,
        GL_UNSIGNED_INT,
        0
    );

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glutPostRedisplay();
    glutSwapBuffers();
}

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;

    Vertex() {}

    Vertex(float x, float y, float z) {
        pos = glm::vec3(x, y, z);
        color = glm::vec3(
            (float)rand() / (float)(RAND_MAX),
            (float)rand() / (float)(RAND_MAX),
            (float)rand() / (float)(RAND_MAX)
        );
    }
};

static void CreateVertexBuffer() {
    Vertex Vertices[8];

    Vertices[0] = Vertex(0.5f, 0.5f, 0.5f);
    Vertices[1] = Vertex(-0.5f, 0.5f, -0.5f);
    Vertices[2] = Vertex(-0.5f, 0.5f, 0.5f);
    Vertices[3] = Vertex(0.5f, -0.5f, -0.5f);
    Vertices[4] = Vertex(-0.5f, -0.5f, -0.5f);
    Vertices[5] = Vertex(0.5f, 0.5f, -0.5f);
    Vertices[6] = Vertex(0.5f, -0.5f, 0.5f);
    Vertices[7] = Vertex(-0.5f, -0.5f, 0.5f);
    
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

static void CreateIndexBuffer() {
    uint32_t Indices[] = {
        0, 1, 2,
        1, 3, 4,
        5, 6, 3,
        7, 3, 6,
        2, 4, 7,
        0, 7, 6,
        0, 5, 1,
        1, 5, 3,
        5, 0, 6,
        7, 4, 3, 
        2, 1, 4,
        0, 2, 7
    };

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
}

// names of the shader files
const char* pVSFileName = "shader.vs";
const char* pFSFileName = "shader.fs";

static void CompileShaders() {
    GLuint ShaderProgram = glCreateProgram();

    if (ShaderProgram == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    std::string vs, fs;

    if (!ReadFile(pVSFileName, vs)) {
        exit(1);
    }

    AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);

    if (!ReadFile(pFSFileName, fs)) {
        exit(1);
    }

    AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glLinkProgram(ShaderProgram);

    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    gWVPLocation = glGetUniformLocation(ShaderProgram, "gWVP");
    if (gWVPLocation == -1) {
        printf("Error getting uniform location of 'gWVP'\n");
        exit(1);
    }

    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glUseProgram(ShaderProgram);
}

static void KeyboardCB(u_char key, int mouse_x, int mouse_y) {
    GameCamera.OnKeyboard(key);
}

static void SpecialKeyboardCB(int key, int mouse_x, int mouse_y) {
    GameCamera.OnKeyboard(key);
}

int main(int argc, char** argv)
{
    srandom(getpid());

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    int x = 1920 + 200; // offset for 1920 pixels to display the window on the second screen
    int y = 100;
    glutInitWindowPosition(x, y);
    int win = glutCreateWindow("Test");
    printf("window id: %d\n", win); 

    GLenum res = glewInit();

    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    GLclampf Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f;
    glClearColor(Red, Green, Blue, Alpha);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    CreateVertexBuffer();
    CreateIndexBuffer();

    CompileShaders();

    //GameCamera.SetPosition(-1.0f, 1.0f, 1.0f);

    glutDisplayFunc(RenderSceneCB);
    glutKeyboardFunc(KeyboardCB);
    glutSpecialFunc(SpecialKeyboardCB);

    glutMainLoop();

    return 0;
}