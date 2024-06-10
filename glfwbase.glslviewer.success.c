//========================================================================
// re-used by: Suwandi Tanuwijaya (swndtan[at]gmail.com)
// based-on Heightmap example program by Olivier Delannoy
// Copyright (c) 2010 Olivier Delannoy
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stddef.h>

#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GL_UTIL_IMPLEMENTATION 
#include "glutil.h"

#include "deps/linmath.h"
/**********************************************************************
 * Values for shader uniforms
 *********************************************************************/
/* Map height updates */
#define MAX_CIRCLE_SIZE (5.0f)
#define MAX_DISPLACEMENT (1.0f)
#define DISPLACEMENT_SIGN_LIMIT (0.3f)
#define MAX_ITER (999)
#define NUM_ITER_AT_A_TIME (1)

/* Map general information */
#define MAP_SIZE (10.0f)
#define MAP_NUM_VERTICES (80)
#define MAP_NUM_TOTAL_VERTICES (MAP_NUM_VERTICES*MAP_NUM_VERTICES)
#define MAP_NUM_LINES (3* (MAP_NUM_VERTICES - 1) * (MAP_NUM_VERTICES - 1) + \
               2 * (MAP_NUM_VERTICES - 1))
               
static const char* defaultVS =
"#version 150\n"
"uniform mat4 project;\n"
"uniform mat4 modelview;\n"
"attribute vec3 vCol;\n"
"attribute vec2 vPos;\n"
"varying vec3 color;\n"
/*"in float x;\n"
"in float y;\n"
"in float z;\n"*/
"\n"
"void main()\n"
"{\n"
//"   gl_PointSize = 399.0;\n"
"   gl_Position = project * modelview * vec4(vPos, .0f, 1.0);\n"
"   color = vCol;\n"
"}\n";

static const char* defaultFS =
"#version 150\n"
"uniform vec2 uResolution;\n"
"uniform float uTime;\n"
"varying vec3 color;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
"    vec2 st = gl_FragCoord.yx/uResolution.yx;\n"
"    st.x *= uResolution.y/uResolution.x;\n"
"    st.y *= uResolution.x/uResolution.y;\n"
"    vec3 ctmp = vec3(0.);\n"
"    ctmp = vec3(st.y,st.x, (sin(uTime)));\n"
"    //finalColor = vec4(0.2, 1.0, 0.2, 1.0); \n"
"    finalColor = vec4(color.rgb, 1.0f)+vec4(ctmp.rgb, .5);\n"
"}\n";

static const struct
{
    float x, y;
    float r, g, b;
} vertices[4] =
{
    { -29.6f, -19.f, 1.f, 0.f, 0.f },
    {  29.6f, -19.f, 0.f, 1.f, 0.f },
    {  29.6f, 19.f, 0.f, 1.f, 0.f },
    { -29.6f, 19.f, 0.f, 0.f, 1.f }
};

/*
static const struct
{
    float x, y;
    float r, g, b;
} vertices[3] =
{
    { -0.6f, -0.4f, 1.f, 0.f, 0.f },
    {  0.6f, -0.4f, 0.f, 1.f, 0.f },
    {   0.f,  0.6f, 0.f, 0.f, 1.f }
};
*/

/* Frustum configuration */
static GLfloat view_angle = 45.0f;
static GLfloat aspect_ratio = 16.0f/9.0f;
static GLfloat z_near = 9.0f;
static GLfloat z_far = 259.f;

/* Projection matrix */
static GLfloat projection_matrix[16] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

/* Model view matrix */
static GLfloat modelview_matrix[16] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

/**********************************************************************
 * GLFW callback functions
 *********************************************************************/
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    switch(key)
    {
        case GLFW_KEY_ESCAPE:
            /* Exit program on Escape */
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
    }
}

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

int main(int argc, char** argv)
{
    GLFWwindow* window;
    double dt, last_update_time;
    float f;
    GLint vertex_buffer, uloc_modelview;
    GLint uloc_project, uTimeLoc, uResLoc, vpos_location, vcol_location;
    int iter, frame, width, height;

    GLuint shader_program;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    window = glfwCreateWindow(800, 600, "GLFW OpenGL3 Heightmap demo", NULL, NULL);
    if (! window )
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    /* Register events callback */
    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(1);
    
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    /* Prepare opengl resources for rendering */
    shader_program = CreateShaderProgram(defaultVS, defaultFS);
    if (shader_program == 0u)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    
    glUseProgram(shader_program);
    vpos_location = glGetAttribLocation(shader_program, "vPos");
    vcol_location = glGetAttribLocation(shader_program, "vCol");
    
    uloc_project   = glGetUniformLocation(shader_program, "project");
    uloc_modelview = glGetUniformLocation(shader_program, "modelview");
    uTimeLoc =  glGetUniformLocation(shader_program, "uTime");
    uResLoc =  glGetUniformLocation(shader_program, "uResolution");
    
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) 0);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) (sizeof(float) * 2));
                          
    /* Compute the projection matrix */
    f = 1.0f / tanf(view_angle / 2.0f);
    projection_matrix[0]  = f / aspect_ratio;
    projection_matrix[5]  = f;
    projection_matrix[10] = (z_far + z_near)/ (z_near - z_far);
    projection_matrix[11] = -1.0f;
    projection_matrix[14] = 2.0f * (z_far * z_near) / (z_near - z_far);
    glUniformMatrix4fv(uloc_project, 1, GL_FALSE, projection_matrix);

    /* Set the camera position */
    modelview_matrix[12]  = -5.0f;
    modelview_matrix[13]  = -5.0f;
    modelview_matrix[14]  = -20.0f;
    glUniformMatrix4fv(uloc_modelview, 1, GL_FALSE, modelview_matrix);

    /* Create vao + vbo to store the mesh */
    /* Create the vbo to store all the information for the grid and the height */

    /* setup the scene ready for rendering */
    //glfwGetFramebufferSize(window, &width, &height);
    //glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 255.0f);
    float res[2] = {width, height};
    glUniform2fv(uResLoc, 1, &res);
    glEnable(GL_PROGRAM_POINT_SIZE);
    
    glPointSize(299);
    /* main loop */
    frame = 0;
    iter = 0;
    last_update_time = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        mat4x4 m, p, mvp;
        
        ++frame;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glViewport(0, 0, width, height);
        //glClear(GL_COLOR_BUFFER_BIT);
        
        dt = glfwGetTime();
        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, dt);
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);
        
        
        float uTime = dt;//(dt - last_update_time);
        glUniform1fv(uTimeLoc, 1, &uTime);
        
        /* render the next frame */
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(shader_program);
            //glUniformMatrix4fv(uloc_project, 1, GL_FALSE, (const GLfloat*) p);
            //glUniformMatrix4fv(uloc_modelview, 1, GL_FALSE, (const GLfloat*) m);
            glBindVertexArray(vertex_buffer);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        //glDrawElements(GL_LINES, 2* MAP_NUM_LINES , GL_UNSIGNED_INT, 0);
        glUseProgram(0);
        
        /* display and process events through callbacks */
        glfwSwapBuffers(window);
        glfwPollEvents();
        /* Check the frame rate and update the heightmap if needed */
        /*
        dt = glfwGetTime();
        if ((dt - last_update_time) > 0.2)
        {
            / * generate the next iteration of the heightmap * /
            if (iter < MAX_ITER)
            {
                
                
                //UpdateMap(NUM_ITER_AT_A_TIME);
                //UpdateMesh();
                iter += NUM_ITER_AT_A_TIME;
            }
            last_update_time = dt;
            frame = 0;
        } */       
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

