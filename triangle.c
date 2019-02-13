#include <stdlib.h>
#include <stdbool.h>
#include "emscripten.h"

#include <GL/glew.h>
#include <SDL.h>
#include "shaders.h"
#include "error.h"

// Globals
SDL_Surface* screen;
GLuint program, vbo_triangle;
GLint attribute_coord3d, attribute_v_color;
GLint uniform_fade;

typedef struct attributes {
    GLfloat coord3d[3];
    GLfloat v_color[3];
} attributes;

bool init_resources(void) {
    // Initialize triangle buffer
    attributes triangle_attributes[] = {
        {{0.0f,  0.4f, 0.0}, {1.0f, 1.0f, 0.0f}},
        {{-0.4f, -0.4f, 0.0}, {0.0f, 0.0f, 1.0f}},
        {{0.4f, -0.4f, 0.0}, {1.0f, 0.0f, 0.0f}},
    };
    glGenBuffers(1, &vbo_triangle);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_attributes), triangle_attributes, GL_STATIC_DRAW);

    // Vertex Shader
    const char *vs_source =
        "attribute vec3 coord3d;"
        "attribute vec3 v_color;"
        "varying vec3 f_color;"
        "void main() {"
        "   gl_Position = vec4(coord3d, 1.0);"
        "   f_color = v_color;"
        "}";
	
    GLuint vs = loadShader(GL_VERTEX_SHADER, vs_source);
    if(isError()) {
        return 1;
    }
    
    // Fragment Shader
    const char *fs_source =
        "precision mediump float;"
        "varying vec3 f_color;"
        "uniform float fade;"
        "void main(void) {"
        "   gl_FragColor = vec4(f_color.r, f_color.g, f_color.b, fade);"
        "}";

    GLuint fs = loadShader(GL_FRAGMENT_SHADER, fs_source);

    if(isError()) {
        return 1;
    }
    
    // Link it!
    program = buildProgram(vs, fs, "vPosition");

    // Declare attributes and uniforms
    attribute_coord3d = glGetAttribLocation(program, "coord3d");
    attribute_v_color = glGetAttribLocation(program, "v_color");
    if(attribute_coord3d == -1 || attribute_v_color == -1) {
        setError("Couldn't bind attribute");
    }

    uniform_fade = glGetUniformLocation(program, "fade");
    if(uniform_fade == -1) {
        setError("Couldn't bind uniform");
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

    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        setError((char*)glewGetErrorString(glew_status));
        return 1;
    }

    if(init_resources() == 1) {
        return 1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1); // request alpha channel
    glViewport(0, 0, width, height);
    //glEnableVertexAttribArray(0);

    return 0;
}

EMSCRIPTEN_KEEPALIVE
void drawTriangle(float fade) {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);
    
    // Enable alpha
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);    

    // Set uniform value
    glUniform1f(uniform_fade, fade);

    // Bind buffers and activate arrays
    glEnableVertexAttribArray(attribute_coord3d);
    glEnableVertexAttribArray(attribute_v_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
    glVertexAttribPointer(attribute_coord3d, 3, GL_FLOAT, GL_FALSE, sizeof(attributes), 0);
    glVertexAttribPointer(attribute_v_color, 3, GL_FLOAT, GL_FALSE, sizeof(attributes), (GLvoid*) offsetof(attributes, v_color));

    glDrawArrays(GL_TRIANGLES, 0, 3);

    // Display the result
    SDL_GL_SwapBuffers();

    // Clean up
    glDisableVertexAttribArray(attribute_v_color);
}

EMSCRIPTEN_KEEPALIVE
void free_resources() {
    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_triangle);
}
