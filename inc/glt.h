#pragma once
#include "inc/gl.h"
#include "GLFW/glfw3.h"

GLFWwindow* createOpenGLWindow(int width, int height, const char* title);
int compileProgram(const char* vshader_src_file, const char* fshader_src_file);
int compileGProgram(const char* vshader_src_file, const char* gshader_src_file, const char* fshader_src_file);
void printOpenGLVersion();
void setupDebugPrint();
char *read_file(const char *fn);
