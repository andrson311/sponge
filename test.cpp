#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cstdint>
#include <unistd.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "includes/stb_image.h"

#include "utils/general.h"
#include "utils/shader_utils.h"
#include "utils/world_transform.h"
#include "utils/camera.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

GLuint VBO;
GLuint IBO;
GLuint gWVPLocation;
GLuint gSamplerLocation;
GLuint gTexture;

WorldTrans CubeWorldTransform;
glm::vec3 CameraPos(0.0f, 0.0f, -1.0f);
glm::vec3 CameraTarget(0.0f, 0.0f, 1.0f);
glm::vec3 CameraUp(0.0f, 1.0f, 0.0f);
Camera GameCamera(WINDOW_WIDTH, WINDOW_HEIGHT, CameraPos, CameraTarget, CameraUp);

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

static void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GameCamera.OnRender();

    static float RotationAngle = 0.02f;

    CubeWorldTransform.SetPosition(0.0f, 0.0f, 2.0f);
    CubeWorldTransform.Rotate(RotationAngle, RotationAngle, 0.0f);
    glm::mat4 World = CubeWorldTransform.GetMatrix();
    glm::mat4 Camera = GameCamera.GetMatrix();

    float aspectRatio = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

    glm::mat4 Projection = glm::perspective(
        glm::radians(90.0f),
        aspectRatio,
        0.1f,
        10.0f);

    glm::mat4 WVP = Projection * Camera * World;

    glUniformMatrix4fv(
        gWVPLocation,
        1,
        GL_FALSE,
        glm::value_ptr(WVP));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTexture);
    glUniform1i(gSamplerLocation, 0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void *)(3 * sizeof(float)));

    glDrawElements(
        GL_TRIANGLES,
        36,
        GL_UNSIGNED_INT,
        0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glutPostRedisplay();
    glutSwapBuffers();
}

static bool LoadTexture(const char *pFileName)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(1); // flip so texture is right-side up
    u_char *data = stbi_load(pFileName, &width, &height, &channels, 0);

    if (!data)
    {
        fprintf(stderr, "Failed to load texture '%s': %s\n", pFileName, stbi_failure_reason());
        return false;
    }

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    glGenTextures(1, &gTexture);
    glBindTexture(GL_TEXTURE_2D, gTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return true;
}

static void CreateVertexBuffer()
{
    glm::vec2 t00(0.0f, 0.0f); // Bottom left
    glm::vec2 t01(0.0f, 1.0f); // Top left
    glm::vec2 t10(1.0f, 0.0f); // Bottom right
    glm::vec2 t11(1.0f, 1.0f); // Top right

    Vertex Vertices[8];
    Vertices[0] = Vertex(glm::vec3(0.5f, 0.5f, 0.5f), t00);
    Vertices[1] = Vertex(glm::vec3(-0.5f, 0.5f, -0.5f), t01);
    Vertices[2] = Vertex(glm::vec3(-0.5f, 0.5f, 0.5f), t10);
    Vertices[3] = Vertex(glm::vec3(0.5f, -0.5f, -0.5f), t11);
    Vertices[4] = Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), t00);
    Vertices[5] = Vertex(glm::vec3(0.5f, 0.5f, -0.5f), t10);
    Vertices[6] = Vertex(glm::vec3(0.5f, -0.5f, 0.5f), t01);
    Vertices[7] = Vertex(glm::vec3(-0.5f, -0.5f, 0.5f), t11);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

static void CreateIndexBuffer()
{
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
        0, 2, 7};

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
}

// names of the shader files
const char *pVSFileName = "shader.vs";
const char *pFSFileName = "shader.fs";

static void CompileShaders()
{
    GLuint ShaderProgram = glCreateProgram();

    if (ShaderProgram == 0)
    {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    std::string vs, fs;

    if (!ReadFile(pVSFileName, vs))
    {
        exit(1);
    }

    AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);

    if (!ReadFile(pFSFileName, fs))
    {
        exit(1);
    }

    AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = {0};

    glLinkProgram(ShaderProgram);

    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if (Success == 0)
    {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    gWVPLocation = glGetUniformLocation(ShaderProgram, "gWVP");
    if (gWVPLocation == -1)
    {
        printf("Error getting uniform location of 'gWVP'\n");
        exit(1);
    }

    gSamplerLocation = glGetUniformLocation(ShaderProgram, "gSampler");
    if (gSamplerLocation == -1)
    {
        printf("Error getting uniform location of 'gSampler'\n");
        exit(1);
    }

    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success)
    {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glUseProgram(ShaderProgram);
}

static void KeyboardCB(u_char key, int mouse_x, int mouse_y)
{
    GameCamera.OnKeyboard(key);
}

static void SpecialKeyboardCB(int key, int mouse_x, int mouse_y)
{
    GameCamera.OnKeyboard(key);
}

static void PassiveMouseCB(int x, int y)
{
    GameCamera.OnMouse(x, y);
}

static void InitializeGlutCallbacks()
{
    glutDisplayFunc(RenderSceneCB);
    glutKeyboardFunc(KeyboardCB);
    glutSpecialFunc(SpecialKeyboardCB);
    glutPassiveMotionFunc(PassiveMouseCB);
}

int main(int argc, char **argv)
{
    srandom(getpid());

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    // int x = 1920 + 200; // offset for 1920 pixels to display the window on the second screen
    // int y = 100;
    // glutInitWindowPosition(x, y);
    // int win = glutCreateWindow("Test");
    // printf("window id: %d\n", win);

    char game_mode_string[64];
    snprintf(
        game_mode_string,
        sizeof(game_mode_string),
        "%dx%d@32",
        WINDOW_WIDTH,
        WINDOW_HEIGHT);

    glutGameModeString(game_mode_string);
    glutEnterGameMode();

    InitializeGlutCallbacks();

    GLenum res = glewInit();

    if (res != GLEW_OK)
    {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    GLclampf Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f;
    glClearColor(Red, Green, Blue, Alpha);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    int texture_units = 0;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
    printf("Number of texture units %d\n", texture_units);

    CreateVertexBuffer();
    CreateIndexBuffer();

    CompileShaders();

    if (!LoadTexture("assets/bricks.jpg"))
    {
        return 1;
    }

    glutMainLoop();

    return 0;
}