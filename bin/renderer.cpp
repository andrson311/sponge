#include "renderer.h"
#include "util.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

Renderer::Renderer()
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    persProjInfo = {
        45.0f,
        (float)WINDOW_WIDTH,
        (float)WINDOW_HEIGHT,
        1.0f,
        100.0f};
}

Renderer::~Renderer()
{
    delete pTexture;
    delete pGameCamera;

    if (CubeVAO != -1)
        glDeleteVertexArrays(1, &CubeVAO);
    if (CubeVBO != -1)
        glDeleteVertexArrays(1, &CubeVBO);
    if (CubeIBO != -1)
        glDeleteVertexArrays(1, &CubeIBO);
    if (PyramidVAO != -1)
        glDeleteVertexArrays(1, &PyramidVAO);
    if (PyramidVBO != -1)
        glDeleteVertexArrays(1, &PyramidVBO);
    if (PyramidIBO != -1)
        glDeleteVertexArrays(1, &PyramidIBO);
}

bool Renderer::Init()
{
    CreateCubeVAO();
    CreatePyramidVAO();

    glBindVertexArray(CubeVAO);

    CompileShaders();

    pTexture = new Texture(GL_TEXTURE_2D, "assets/bricks.jpg");
    if (!pTexture->Load())
    {
        return false;
    }

    pTexture->Bind(GL_TEXTURE0);
    glUniform1i(SamplerLocation, 0);

    glm::vec3 CameraPos(0.0f, 0.0f, -1.0f);
    glm::vec3 CameraTarget(0.0f, 0.0f, 1.0f);
    glm::vec3 CameraUp(0.0f, 1.0f, 0.0f);
    pGameCamera = new Camera(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        CameraPos,
        CameraTarget,
        CameraUp);

    return true;
}

void Renderer::RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    pGameCamera->OnRender();

    float RotationAngle = 0.01f;

    CubeWorldTransform.SetPosition(0.0f, 0.0f, 2.0f);
    CubeWorldTransform.Rotate(RotationAngle, RotationAngle, 0.0f);

    glm::mat4 World = CubeWorldTransform.GetMatrix();
    glm::mat4 View = pGameCamera->GetMatrix();

    float aspectRatio = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

    glm::mat4 Projection = glm::perspective(
        glm::radians(persProjInfo.FOV), aspectRatio, persProjInfo.zNear, persProjInfo.zFar);

    glm::mat4 WVP = Projection * View * World;
    glUniformMatrix4fv(WVPLocation, 1, GL_FALSE, glm::value_ptr(WVP));

    GLint CurrentVAO;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &CurrentVAO);

    if (CurrentVAO == (GLint)CubeVAO)
    {
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }
    else
    {
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
    }

    glutPostRedisplay();
    glutSwapBuffers();
}

void Renderer::KeyboardCB(u_char key, int mouse_x, int mouse_y)
{
    switch (key)
    {
    case 'q':
    case 27: // escape key code
        exit(0);

    case '1':
        glBindVertexArray(CubeVAO);
        break;

    case '2':
        glBindVertexArray(PyramidVAO);
        break;
    }

    pGameCamera->OnKeyboard(key);
}

void Renderer::SpecialKeyboardCB(int key, int mouse_x, int mouse_y)
{
    pGameCamera->OnKeyboard(key);
}

void Renderer::PassiveMouseCB(int x, int y) {
    pGameCamera->OnMouse(x, y);
}

void Renderer::CreateCubeVAO()
{
    glGenVertexArrays(1, &CubeVAO);
    glBindVertexArray(CubeVAO);

    glm::vec2 t00(0.0f, 0.0f);
    glm::vec2 t01(0.0f, 1.0f);
    glm::vec2 t10(1.0f, 0.0f);
    glm::vec2 t11(1.0f, 1.0f);

    Vertex Vertices[8];
    Vertices[0] = Vertex(glm::vec3( 0.5f,  0.5f,  0.5f), t00);
    Vertices[1] = Vertex(glm::vec3(-0.5f,  0.5f, -0.5f), t01);
    Vertices[2] = Vertex(glm::vec3(-0.5f,  0.5f,  0.5f), t10);
    Vertices[3] = Vertex(glm::vec3( 0.5f, -0.5f, -0.5f), t11);
    Vertices[4] = Vertex(glm::vec3(-0.5f, -0.5f, -0.5f), t00);
    Vertices[5] = Vertex(glm::vec3( 0.5f,  0.5f, -0.5f), t10);
    Vertices[6] = Vertex(glm::vec3( 0.5f, -0.5f,  0.5f), t01);
    Vertices[7] = Vertex(glm::vec3(-0.5f, -0.5f,  0.5f), t11);

    glGenBuffers(1, &CubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, CubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));

    uint32_t Indices[] = {
        0, 1, 2,   1, 3, 4,
        5, 6, 3,   7, 3, 6,
        2, 4, 7,   0, 7, 6,
        0, 5, 1,   1, 5, 3,
        5, 0, 6,   7, 4, 3,
        2, 1, 4,   0, 2, 7
    };

    glGenBuffers(1, &CubeIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CubeIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Renderer::CreatePyramidVAO()
{
    glGenVertexArrays(1, &PyramidVAO);
    glBindVertexArray(PyramidVAO);

    glm::vec2 t00 (0.0f, 0.0f);
    glm::vec2 t050(0.5f, 0.0f);
    glm::vec2 t10 (1.0f, 0.0f);
    glm::vec2 t051(0.5f, 1.0f);

    Vertex Vertices[4] = {
        Vertex(glm::vec3(-1.0f, -1.0f,  0.5773f), t00),
        Vertex(glm::vec3( 0.0f, -1.0f, -1.15475f), t050),
        Vertex(glm::vec3( 1.0f, -1.0f,  0.5773f), t10),
        Vertex(glm::vec3( 0.0f,  1.0f,  0.0f),    t051)
    };

    glGenBuffers(1, &PyramidVBO);
    glBindBuffer(GL_ARRAY_BUFFER, PyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));

    uint32_t Indices[] = {
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };

    glGenBuffers(1, &PyramidIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, PyramidIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Renderer::AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
    GLuint ShaderObj = glCreateShader(ShaderType);
    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(1);
    }

    const GLchar* p[1];
    p[0] = pShaderText;

    GLint Lengths[1];
    Lengths[0] = (GLint)strlen(pShaderText);

    glShaderSource(ShaderObj, 1, p, Lengths);
    glCompileShader(ShaderObj);

    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }

    glAttachShader(ShaderProgram, ShaderObj);
}

void Renderer::CompileShaders()
{
    GLuint ShaderProgram = glCreateProgram();

    if (ShaderProgram == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    std::string vs, fs;

    if (!ReadFile("shader.vs", vs)) exit(1);
    AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);

    if (!ReadFile("shader.fs", fs)) exit(1);
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

    WVPLocation = glGetUniformLocation(ShaderProgram, "gWVP");
    if (WVPLocation == -1) {
        printf("Error getting uniform location of 'gWVP'\n");
        exit(1);
    }

    SamplerLocation = glGetUniformLocation(ShaderProgram, "gSampler");
    if (SamplerLocation == -1) {
        printf("Error getting uniform location of 'gSampler'\n");
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
