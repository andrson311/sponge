#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

int GetGLMajorVersion();
int GetGLMinorVersion();
int IsGLVersionHigher(int MajorVer, int MinorVer);

GLFWwindow *InitGLFW(int width, int height, bool is_full_screen, const char *title);