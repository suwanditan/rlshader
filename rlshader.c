/*******************************************************************************************
*
*   raylib [shaders] example - OpenGL point particle system
*
*   Example originally created with raylib 3.8, last time updated with raylib 2.5
*
*   Example contributed by Stephan Soller (@arkanis) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2021-2024 Stephan Soller (@arkanis) and Ramon Santamaria (@raysan5)
*
********************************************************************************************
*
*   Mixes raylib and plain OpenGL code to draw a GL_POINTS based particle system. The
*   primary point is to demonstrate raylib and OpenGL interop.
*
*   rlgl batched draw operations internally so we have to flush the current batch before
*   doing our own OpenGL work (rlDrawRenderBatchActive()).
*
*   The example also demonstrates how to get the current model view projection matrix of
*   raylib. That way raylib cameras and so on work as expected.
*
********************************************************************************************/
#include <stdlib.h>
#include <stdio.h>

#include "raylib.h"
/*
#if defined(PLATFORM_DESKTOP)
    #if defined(GRAPHICS_API_OPENGL_ES2)
        #include "glad_gles2.h"       // Required for: OpenGL functionality 
        #define glGenVertexArrays glGenVertexArraysOES
        #define glBindVertexArray glBindVertexArrayOES
        #define glDeleteVertexArrays glDeleteVertexArraysOES
        #define GLSL_VERSION            100
    #else
        #if defined(__APPLE__)
            #include <OpenGL/gl3.h>     // OpenGL 3 library for OSX
            #include <OpenGL/gl3ext.h>  // OpenGL 3 extensions library for OSX
        #else
            #include "glad.h"       // Required for: OpenGL functionality 
        #endif
        #define GLSL_VERSION            330
    #endif
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif
*/
#define GLSL_VERSION            330

#include "rlgl.h"           // Required for: rlDrawRenderBatchActive(), rlGetMatrixModelview(), rlGetMatrixProjection()
#include "raymath.h"        // Required for: MatrixMultiply(), MatrixToFloat()
#include <GL/glew.h>
#include "GLFW/glfw3.h"         // Windows/Context and inputs management

#define MAX_PARTICLES       999

// Particle type
typedef struct Particle {
    float x;
    float y;
    float period;
} Particle;

GLFWwindow *window;
static void ErrorCallback(int error, const char *description)
{
    fprintf(stderr, "%s", description);
}

// GLFW3: Keyboard callback
static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 600;
    
    glfwSetErrorCallback(ErrorCallback);
    if (!glfwInit())
    {
        printf("GLFW3: Can not initialize GLFW\n");
        exit(1);
    }
    else printf("GLFW3: GLFW initialized successfully\n");
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    
    // WARNING: OpenGL 3.3 Core profile only
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    //InitWindow(screenWidth, screenHeight, "raylib - point particles");
    window = glfwCreateWindow(screenWidth, screenHeight, "RLGL OpenGL Interop", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(2);
    }
    else printf("GLFW3: Window created successfully\n");
        

    glfwSetWindowPos(window, 200, 200);
    glfwSetKeyCallback(window, KeyCallback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // !IMPORTANT
    
    rlLoadExtensions(glfwGetProcAddress);
    
    // Initialize OpenGL context (states and resources)
    rlglInit(screenWidth, screenHeight);
    rlLoadTextureDepth(screenWidth, screenHeight, true);
    // Initialize viewport and internal projection/modelview matrices
    rlViewport(0, 0, screenWidth, screenHeight);
    rlMatrixMode(RL_PROJECTION);                        // Switch to PROJECTION matrix
    rlLoadIdentity();                                   // Reset current matrix (PROJECTION)
    rlOrtho(0, screenWidth, screenHeight, 0, 0.0f, 1.0f); // Orthographic projection with top-left corner at (0,0)
    rlMatrixMode(RL_MODELVIEW);                         // Switch back to MODELVIEW matrix
    rlLoadIdentity();                                   // Reset current matrix (MODELVIEW)

    rlClearColor(0, 0, 0, 0);                   // Define clear color
    rlEnableDepthTest();                          // Enable DEPTH_TEST for 3D
    
    //rlClearColor(0, 0, 0, 255);
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    
    //glEnable(GL_DEPTH_TEST); 
    //glDepthFunc(GL_LESS);
    
    
    Shader shader = LoadShader(TextFormat("resources/shaders/glsl%i/point_particle.vs", GLSL_VERSION),
                               TextFormat("resources/shaders/glsl%i/point_particle.fs", GLSL_VERSION));

    int currentTimeLoc = GetShaderLocation(shader, "currentTime");
    int colorLoc = GetShaderLocation(shader, "color");

    rlClearScreenBuffers();             // Clear current framebuffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);       
    // Initialize the vertex buffer for the particles and assign each particle random values
    Particle particles[MAX_PARTICLES] = { 0 };

    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        particles[i].x = GetRandomValue(20, 800 - 20)/2*50;
        particles[i].y = GetRandomValue(50, 600 - 20);
        
        // Give each particle a slightly different period. But don't spread it to much. 
        // This way the particles line up every so often and you get a glimps of what is going on.
        particles[i].period = (float)GetRandomValue(10, 30)/5.0f;
        //printf("num-%d\n", i);
    }
/*
    Matrix matProj = MatrixOrtho(0.0, screenWidth, screenHeight, 0.0, 0.0, 1.0);
    Matrix matView = MatrixIdentity();

    rlSetMatrixModelview(matView);    // Set internal modelview matrix (default shader)
    rlSetMatrixProjection(matProj);   // Set internal projection matrix (default shader)
  */  

    rlClearColor(0, 0, 0, 0);                   // Define clear color
    rlEnableDepthTest();                          // Enable DEPTH_TEST for 3D
    /*
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        / * Problem: glewInit failed, something is seriously wrong. * /
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    */
    //glEnable(GL_DEPTH_TEST); 
    //glDepthFunc(GL_LESS);
    
    
    
    // Create a plain OpenGL vertex buffer with the data and an vertex array object 
    // that feeds the data from the buffer into the vertexPosition shader attribute.
    //rlClearScreenBuffers();             // Clear current framebuffer
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);       
    GLuint vao = 0;
    GLuint vbo = 0;
    
        
    
    unsigned int vao2 = rlLoadVertexArray();
    //unsigned int vbo2 = rlLoadVertexBuffer(particles, MAX_PARTICLES*sizeof(Particle), false);
    
    glGenVertexArrays(3, &vao);
    glBindVertexArray(vao);    
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, (int)MAX_PARTICLES*sizeof(Particle), particles, GL_STATIC_DRAW);
        
        // Note: LoadShader() automatically fetches the attribute index of "vertexPosition" and saves it in shader.locs[SHADER_LOC_VERTEX_POSITION]
        rlSetVertexAttribute(shader.locs[SHADER_LOC_VERTEX_POSITION], 3, GL_FLOAT, GL_FALSE, 0, 0);
        //shader.locs[SHADER_LOC_VERTEX_POSITION]
        //GetShaderLocation(shader, "vertex_position")
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 0, 0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Allows the vertex shader to set the point size of each particle individually
    glEnable(GL_PROGRAM_POINT_SIZE);
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------
    unsigned int framesCounter = 0;
    // Main game loop
    //while (!WindowShouldClose())    // Detect window close button or ESC key
    while (!glfwWindowShouldClose(window))
    {
        framesCounter++; //printf("frame: %d\n", framesCounter); 
        if (framesCounter%1800 == 0) framesCounter=0;
        Matrix matProj = MatrixOrtho(0.0, screenWidth, screenHeight, 0.0, 0.0, 1.0);
        Matrix matView = MatrixIdentity();

        rlSetMatrixModelview(matView);    // Set internal modelview matrix (default shader)
        rlSetMatrixProjection(matProj);   // Set internal projection matrix (default shader)
        Matrix modelViewProjection = MatrixMultiply(matView, matProj);
        
        // Draw
        //----------------------------------------------------------------------------------
        //BeginDrawing();
            ClearBackground(BLANK);

            DrawRectangle(10, 10, 780, 30, RAYWHITE);
            //DrawRectangle(10, 49, 780, 30, RAYWHITE);
            //DrawText(TextFormat("%zu particles in one vertex buffer", MAX_PARTICLES), 20, 20, 10, RAYWHITE);
            
            rlDrawRenderBatchActive();      // Draw iternal buffers data (previous draw calls)

            // Switch to plain OpenGL
            //------------------------------------------------------------------------------
            glUseProgram(shader.id);
            
                glUniform1f(currentTimeLoc, GetTime());

                Vector4 color = ColorNormalize((Color){ 0, 0, 255, 128 });
                glUniform4fv(colorLoc, 1, (float *)&color);

                // Get the current modelview and projection matrix so the particle system is displayed and transformed
                //Matrix modelViewProjection = MatrixMultiply(rlGetMatrixModelview(), rlGetMatrixProjection());
                //modelViewProjection
                //shader.locs[SHADER_LOC_MATRIX_MVP]
                glUniformMatrix4fv(shader.locs[SHADER_LOC_MATRIX_MVP], 1, false, MatrixToFloat(matProj));

                glBindVertexArray(vao);
                    glDrawArrays(GL_POINTS, 0, (int)MAX_PARTICLES);
                glBindVertexArray(0);
                
            glUseProgram(0);
            //------------------------------------------------------------------------------
            
            //DrawFPS( 100, 10);
            
        //EndDrawing();
        //----------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    UnloadShader(shader);   // Unload shader

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
