#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include "utils/general.h"
#include "shader_utils.h"

GLuint VBO;

static void RenderSceneCB()
{
    static GLclampf c = 0.0f;
    glClearColor(c, c, c, c);

    //c += 1.0f / 256.0f;

    if (c >= 1.0f) {
        c = 0;
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);

    //glutPostRedisplay();
    glutSwapBuffers();
}

static void CreateVertexBuffer() {
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW); // GL_CCW by default
    // glCullFace(GL_FRONT) // note: sets the cull face

    glm::vec3 Vertices[3];

    Vertices[0] = glm::vec3(-1.0f, -1.0f, 0.0f);
    Vertices[1] = glm::vec3(0.0f, 1.0f, 0.0f);
    Vertices[2] = glm::vec3(1.0f, -1.0f, 0.0f);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
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

    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glUseProgram(ShaderProgram);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);

    int width = 1280;
    int height = 720;
    glutInitWindowSize(width, height);

    int x = 200;
    int y = 100;
    glutInitWindowPosition(x, y);
    int win = glutCreateWindow("Test");
    printf("window id: %d\n", win); 

    GLenum res = glewInit();

    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    //GLclampf Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f;
    //glClearColor(Red, Green, Blue, Alpha);

    CreateVertexBuffer();

    CompileShaders();

    glutDisplayFunc(RenderSceneCB);

    glutMainLoop();

    return 0;
}