#include <stdlib.h>
#include <stdbool.h>
#include "emscripten.h"

#include <GL/glew.h>
#include <SDL.h>
#include "shaders.h"
#include "error.h"

// Globals
GLuint program;
SDL_Surface* screen;
GLfloat triangle_vertices[] = {
    0.0f,  0.4f, 0.0f,
   -0.4f, -0.4f, 0.0f,
    0.4f, -0.4f, 0.0f,
};
GLfloat triangle_vertices2[] = {
    0.8f,  0.4f, 0.0f,
    0.0f,  0.4f, 0.0f,
    0.4f, -0.4f, 0.0f,
};
GLint uniformOriginX, uniformOriginY, uniformZoom;

GLint attribute_coord2d;

bool init_resources(void) {
    // Vertex Shader
    const char *vs_source =
        /*"attribute vec4 vPosition;                           \n"
	"uniform float originX, originY;                     \n"
	"uniform float zoom;                                 \n"
	"varying vec3 color;                                 \n"
	"void main()                                         \n"
	"{                                                   \n"
	"   gl_Position = vPosition;                         \n"
	"   gl_Position.x = (originX + gl_Position.x) * zoom;\n"
	"   gl_Position.y = (originY + gl_Position.y) * zoom;\n"
	"   color = gl_Position.xyz + vec3(0.5);             \n"
        "}                                                   \n";*/
        "attribute vec2 coord2d;"
        "varying vec3 color;"
        "void main() {"
        "   gl_Position = vec4(coord2d, 0.0, 1.0);"
        "   color = gl_Position.xyz + vec3(0.5);"
        "}";
	
    GLuint vs = loadShader(GL_VERTEX_SHADER, vs_source);
    if(isError()) {
        return 1;
    }
    
    // Fragment Shader
    const char *fs_source =
        "precision mediump float;"
        "varying vec3 color;"
        "void main(void) {"
        "   gl_FragColor = vec4(color, 1.0);"
        "}";
    GLuint fs = loadShader(GL_FRAGMENT_SHADER, fs_source);

    if(isError()) {
        return 1;
    }
    
    // Link it!
    program = buildProgram(vs, fs, "vPosition");

    attribute_coord2d = glGetAttribLocation(program, "coord2d");
    if(attribute_coord2d == -1) {
        setError("Couldn't bind attribute");
    }

    return isError() ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
int init(int width, int height) {
    if(SDL_Init(SDL_INIT_VIDEO) == 0) {
        screen = SDL_SetVideoMode(width, height, 0, SDL_OPENGL);
        if(screen == NULL) {
            setError((char*)SDL_GetError());
            return 1;
        }
    } else {
        setError((char*)SDL_GetError());
        return 1;
    }
/*
    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        setError((char*)glewGetErrorString(glew_status));
        return 1;
    }
*/
    if(init_resources() == 1) {
        return 1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glViewport(0, 0, width, height);
    glEnableVertexAttribArray(0);

    return 0;
}

EMSCRIPTEN_KEEPALIVE
void drawTriangle() {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);
        
    // Set up the translation
    glUniform1f(uniformOriginX, 0.5f);
    glUniform1f(uniformOriginY, 0.5f);
    glUniform1f(uniformZoom, 1.0f);

    // Describe our vertices array to OpenGL, then push each element to the vertex shader
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, triangle_vertices);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, triangle_vertices2);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Display the result
    SDL_GL_SwapBuffers();
}
