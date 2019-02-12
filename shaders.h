#ifndef SHADERS_H
#define SHADERS_H

#include <GL/glew.h>

GLuint loadShader(GLenum type, const char *source);
GLuint buildProgram(GLuint vertexShader, GLuint fragmentShader, const char * vertexPositionName);

#endif
