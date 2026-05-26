#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>  
#include "utils/general.h"
#include "utils/shader_utils.h"

GLuint VBO;
GLint gScalingLocation;

static void ScalingExample() {
    static float Scale = 1.0f;
    static float Delta = 0.01f;

    Scale += Delta;
    if ((Scale >= 1.5f) || (Scale <= 0.5)) {
        Delta *= -1.0f;
    }

    glm::mat4 Scaling = glm::scale(
        glm::mat4(1.0f),
        glm::vec3(Scale)
    );

    glUniformMatrix4fv(gScalingLocation, 1, GL_FALSE, glm::value_ptr(Scaling));
}

static void CombiningTransformationsExample1() {
    static float Scale = 1.5f;
    glm::mat4 Scaling = glm::scale(
        glm::mat4(1.0f),
        glm::vec3(Scale)
    );

    static float Location = 0.0f;
    static float Delta = 0.01f;

    Location += Delta;
    if ((Location >= 0.5f) || (Location <= -0.5f))
    {
        Delta *= -1.0f;
    }

    glm::mat4 Translation = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(Location, 0.0f, 0.0f)
    );

    //glm::mat4 FinalTransform = Translation * Scaling;
    glm::mat4 FinalTransform = Scaling * Translation;
    
    glUniformMatrix4fv(
        gScalingLocation,
        1,
        GL_FALSE,
        glm::value_ptr(FinalTransform)
    );
}

static void CombiningTransformationsExample2() {
    static float Scale = 0.25f;

    glm::mat4 Scaling = glm::scale(
        glm::mat4(1.0f),
        glm::vec3(Scale)
    );

    static float AngleInRadians = 0.0f;
    static float Delta = 0.01f;

    AngleInRadians += Delta;
    glm::mat4 Rotation = glm::rotate(
        glm::mat4(1.0f),
        AngleInRadians,
        glm::vec3(0.0f, 0.0f, 1.0f)
    );

    static float Location = 0.5f;
    glm::mat4 Translation = glm::translate(
        glm::mat4(1.0f),
        glm::vec3(Location, 0.0f, 0.0f)
    );

    //glm::mat4 FinalTransform = Translation * Rotation * Scaling;
    glm::mat4 FinalTransform = Rotation * Translation * Scaling;

    glUniformMatrix4fv(
        gScalingLocation,
        1,
        GL_FALSE,
        glm::value_ptr(FinalTransform)
    );
}

static void RenderSceneCB()
{

    glClear(GL_COLOR_BUFFER_BIT);

    ScalingExample();
    //CombiningTransformationsExample1();
    //CombiningTransformationsExample2();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(0);
    glutPostRedisplay();
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

    gScalingLocation = glGetUniformLocation(ShaderProgram, "gScaling");
    if (gScalingLocation == -1) {
        printf("Error getting uniform location of 'gScaling'\n");
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

    GLclampf Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f;
    glClearColor(Red, Green, Blue, Alpha);

    CreateVertexBuffer();

    CompileShaders();

    glutDisplayFunc(RenderSceneCB);

    glutMainLoop();

    return 0;
}