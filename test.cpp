#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>  
#include "utils/general.h"
#include "utils/shader_utils.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

GLuint VBO;
GLuint IBO;
GLint gWorldLocation;


static void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT);

    static float Scale = 0.0f;

    glm::mat4 Rotation = glm::rotate(
        glm::mat4(1.0f),
        Scale,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

    glm::mat4 FinalMatrix = Projection * Rotation;

    glUniformMatrix4fv(
        gWorldLocation, 
        1, 
        GL_FALSE,
        glm::value_ptr(FinalMatrix) 
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
        54,
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

    Vertex(float x, float y) {
        pos = glm::vec3(x, y, 0.0f);
        color = glm::vec3(
            (float)rand() / (float)(RAND_MAX),
            (float)rand() / (float)(RAND_MAX),
            (float)rand() / (float)(RAND_MAX)
        );
    }
};

static void CreateVertexBuffer() {
    Vertex Vertices[19];

    // Center
    Vertices[0] = Vertex(0.0f, 0.0f);

    // Top row
    Vertices[1] = Vertex(-1.0f, 1.0f);
    Vertices[2] = Vertex(-0.75f, 1.0f);
    Vertices[3] = Vertex(-0.50f, 1.0f);
    Vertices[4] = Vertex(-0.25f, 1.0f);
    Vertices[5] = Vertex(0.0f, 1.0f);
    Vertices[6] = Vertex(0.25f, 1.0f);
    Vertices[7] = Vertex(0.50f, 1.0f);
    Vertices[8] = Vertex(0.75f, 1.0f);
    Vertices[9] = Vertex(1.0f, 1.0f);
    
    // Bottom row
    Vertices[10] = Vertex(-1.0f, -1.0f);
    Vertices[11] = Vertex(-0.75f, -1.0f);
    Vertices[12] = Vertex(-0.50f, -1.0f);
    Vertices[13] = Vertex(-0.25f, -1.0f);
    Vertices[14] = Vertex(0.0f, -1.0f);
    Vertices[15] = Vertex(0.25f, -1.0f);
    Vertices[16] = Vertex(0.50f, -1.0f);
    Vertices[17] = Vertex(0.75f, -1.0f);
    Vertices[18] = Vertex(1.0f, -1.0f);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

static void CreateIndexBuffer() {
    uint32_t Indices[] = {
        // Top triangles
        0, 2, 1,
        0, 3, 2,
        0, 4, 3,
        0, 5, 4,
        0, 6, 5,
        0, 7, 6,
        0, 8, 7,
        0, 9, 8,

        // Bottom triangles
        0, 10, 11,
        0, 11, 12,
        0, 12, 13,
        0, 13, 14,
        0, 14, 15,
        0, 15, 16,
        0, 16, 17,
        0, 17, 18,

        // Left triangle
        0, 1, 10,

        // Right triangle
        0, 18, 9
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

    gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");
    if (gWorldLocation == -1) {
        printf("Error getting uniform location of 'gWorld'\n");
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

    // GLclampf Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f;
    // glClearColor(Red, Green, Blue, Alpha);

    CreateVertexBuffer();
    CreateIndexBuffer();

    CompileShaders();

    glutDisplayFunc(RenderSceneCB);

    glutMainLoop();

    return 0;
}