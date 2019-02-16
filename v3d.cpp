#include <stdlib.h>
#include <stdbool.h>
#include "emscripten.h"

#include <GL/glew.h>
#include <SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shaders.h"
#include "error.h"

#define degToRad(angle) (angle * M_PI / 180.0)

// Globals
SDL_Surface* screen;
int screen_width, screen_height;

GLuint program, vbo_cube_vertices, vbo_cube_colors, ibo_cube_elements;
GLint attribute_coord3d, attribute_v_color;
GLint uniform_mvp;

bool init_resources(void) {
    // Initialize vertex buffer
    GLfloat cube_vertices[] = {
        -1.0, -1.0, 1.0,
        1.0, -1.0, 1.0,
        1.0, 1.0, 1.0,
        -1.0, 1.0, 1.0,
        // back
        -1.0, -1.0, -1.0,
        1.0, -1.0, -1.0,
        1.0, 1.0, -1.0,
        -1.0, 1.0, -1.0,
    };
    glGenBuffers(1, &vbo_cube_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    // Initialize color buffer
    GLfloat cube_colors[] = {
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0,
        1.0, 1.0, 1.0,
        // back
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0,
        1.0, 1.0, 1.0,
    };
    glGenBuffers(1, &vbo_cube_colors);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_colors);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_colors), cube_colors, GL_STATIC_DRAW);

    // Initialize cube elements
    GLushort cube_elements[] = {
        // front
        0, 1, 2,
        2, 3, 0,
        // right
        1, 5, 6,
        6, 2, 1,
        // back
        7, 6, 5,
        5, 4, 7,
        // left
        4, 0, 3,
        3, 7, 4,
        // bottom
        4, 5, 1,
        1, 0, 4,
        // top
        3, 2, 6,
        6, 7, 3
    };
    glGenBuffers(1, &ibo_cube_elements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);

    // Vertex Shader
    const char *vs_source =
        "attribute vec3 coord3d;"
        "attribute vec3 v_color;"
        "varying vec3 f_color;"
        "uniform mat4 mvp;"
        "void main() {"
        "   gl_Position = mvp * vec4(coord3d, 1.0);"
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
        "void main(void) {"
        "   gl_FragColor = vec4(f_color.r, f_color.g, f_color.b, 1.0);"
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

    uniform_mvp = glGetUniformLocation(program, "mvp");
    if(uniform_mvp == -1) {
        setError("Couldn't bind uniform");
    }

    return isError() ? 1 : 0;
}

void logic(float radius, float phi, float theta) {
    phi = degToRad(phi);
    theta = degToRad(theta);
    
    // Translate cube to background (move it in the world)
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0));

    // Calculate our view (move and aim the camera)
    // lookAt(cameraPosition, lookingAt, and upVector)
    glm::vec3 camera = glm::vec3(radius * sinf(phi) * cosf(theta),
                        radius * sinf(phi) * sinf(theta),
                        radius * cosf(phi)
                        );
    glm::mat4 view = glm::lookAt(camera, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0));;

    // Calculate projection onto screen
    // perspective(verticalFieldOfView, aspectRatio, zNear, zFar)
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f * screen_width / screen_height, 0.1f, 20.0f);

    // Combine the result into one transformation matrix
    glm::mat4 mvp = projection * view * model;

    glUseProgram(program);
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
    // Bind buffers and activate arrays
    glEnableVertexAttribArray(attribute_coord3d);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
    glVertexAttribPointer(attribute_coord3d, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(attribute_v_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_colors);
    glVertexAttribPointer(attribute_v_color, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Pass elements to vertex shader
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
    int size;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    
    glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);

    // Display the result
    SDL_GL_SwapBuffers();

    // Clean up
    glDisableVertexAttribArray(attribute_coord3d);
    glDisableVertexAttribArray(attribute_v_color);
}

EMSCRIPTEN_KEEPALIVE
extern "C" int init(int width, int height) {
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

    screen_width = width;
    screen_height = height;

    GLenum glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        setError((char*)glewGetErrorString(glew_status));
        return 1;
    }

    if(init_resources() == 1) {
        return 1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
    glEnable(GL_DEPTH_TEST);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1); // request alpha channel
    
    glViewport(0, 0, width, height);

    return 0;
}

EMSCRIPTEN_KEEPALIVE
extern "C" void drawFrame(float radius, float phi, float theta) {
    logic(radius, phi, theta);
    render();
}

EMSCRIPTEN_KEEPALIVE
void free_resources() {
    glDeleteProgram(program);
}
