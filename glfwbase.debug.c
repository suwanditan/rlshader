//========================================================================
// Re-used library by: Suwandi Tanuwijaya (swndtan[at]gmail.com)
// based on: Simple GLFW example Copyright (c) Camilla Löwy <elmindreda@glfw.org>
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
//! [code]

#include <math.h>

#include "deps/glad/gl.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "deps/linmath.h"

#include <stdlib.h>
#include <stdio.h>

//#include "raylib.h"
//#include "rlgl.h"

#ifndef RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION
    #define RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION     "vertexPosition"    // Bound by default to shader location: 0
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD
    #define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD     "vertexTexCoord"    // Bound by default to shader location: 1
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL
    #define RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL       "vertexNormal"      // Bound by default to shader location: 2
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR
    #define RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR        "vertexColor"       // Bound by default to shader location: 3
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT
    #define RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT      "vertexTangent"     // Bound by default to shader location: 4
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2
    #define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2    "vertexTexCoord2"   // Bound by default to shader location: 5
#endif

#ifndef RL_DEFAULT_SHADER_UNIFORM_NAME_MVP
    #define RL_DEFAULT_SHADER_UNIFORM_NAME_MVP         "mvp"               // model-view-projection matrix
#endif
#ifndef RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW
    #define RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW        "matView"           // view matrix
#endif
#ifndef RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION
    #define RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION  "matProjection"     // projection matrix
#endif
#ifndef RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL
    #define RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL       "matModel"          // model matrix
#endif
#ifndef RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL
    #define RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL      "matNormal"         // normal matrix (transpose(inverse(matModelView))
#endif
#ifndef RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR
    #define RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR       "colDiffuse"        // color diffuse (base tint color, multiplied by texture color)
#endif
#ifndef RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0
    #define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0  "texture0"          // texture0 (texture slot active 0)
#endif
#ifndef RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1
    #define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1  "texture1"          // texture1 (texture slot active 1)
#endif
#ifndef RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2
    #define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2  "texture2"          // texture2 (texture slot active 2)
#endif

#define RAND_MAX2 32767
#define MAX_PARTICLES       99
typedef struct Particle {
    float x, y;
    float period;
} Particle;

// Color, 4 components, R8G8B8A8 (32bit)
typedef struct Color {
    unsigned char r;        // Color red value
    unsigned char g;        // Color green value
    unsigned char b;        // Color blue value
    unsigned char a;        // Color alpha value
} Color;

// Vector4, 4 components
typedef struct Vector4 {
    float x;                // Vector x component
    float y;                // Vector y component
    float z;                // Vector z component
    float w;                // Vector w component
} Vector4;


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

static const char* defaultVS =
"#version 330                       \n"
"in vec3 vertexPosition;            \n"
"in vec2 vertexTexCoord;            \n"
"in vec4 vertexColor;               \n"
"out vec2 fragTexCoord;             \n"
"out vec4 fragColor;                \n"
"uniform mat4 mvp;                  \n"
"void main()                        \n"
"{                                  \n"
"    fragTexCoord = vertexTexCoord; \n"
"    fragColor = vertexColor;       \n"
"    gl_Position = mvp*vec4(vertexPosition, 1.0); \n"
"}                                  \n";

static const char* defaultFS = 
"#version 330       \n"
"in vec2 fragTexCoord;              \n"
"in vec4 fragColor;                 \n"
"out vec4 finalColor;               \n"
"uniform vec2 uResolution;\n"
"uniform float uTime;\n"
"uniform sampler2D texture0;        \n"
"uniform vec4 colDiffuse;           \n"
"void main()                        \n"
"{                                  \n"
"    vec4 texelColor = texture(texture0, fragTexCoord);   \n"
"    finalColor = texelColor*colDiffuse*fragColor;        \n"
"}                                  \n";

static const char* vsText = 
"#version 330\n"
"// Input vertex attributes\n"
"//in vec3 vertexPosition;\n"
"attribute vec2 vPos;\n"
"// Input uniform values\n"
"//uniform vec3 vertex_position;\n"
"uniform mat4 MVP;\n"
"uniform float uTime;\n"
"           \n"             
"void main()\n"
"{          \n"
"    // Unpack data from vertexPosition\n"
"    vec2  pos    = vPos.xy;\n"
"    float period = 1.0f;\n"
"                        \n"
"    // Calculate final vertex position (jiggle it around a bit horizontally)\n"
"    pos += vec2(100, 0) * sin(period*uTime);\n"
"    gl_Position = MVP * vec4(pos, 0.0, 1.0);\n"
"                                            \n"
"    // Calculate the screen space size of this particle (also vary it over time)\n"
"    gl_PointSize = 10 - 5 * abs(sin(period * uTime));\n"
"}\n";

static const char* fsText =
"#version 330\n"
"// Input uniform values\n"
"//uniform vec4 color;\n"
"uniform vec2 uResolution;\n"
"uniform float uTime;\n"
"// Output fragment color\n"
"out vec4 finalColor;\n"
"void main()\n"
"{          \n"
"    // Each point is drawn as a screen space square of gl_PointSize size. gl_PointCoord contains where we are inside of\n"
"    // it. (0, 0) is the top left, (1, 1) the bottom right corner.\n"
"    // Draw each point as a colored circle with alpha 1.0 in the center and 0.0 at the outer edges.\n"
"   vec2 st = gl_FragCoord.xy/uResolution.xy;\n"
"   st.x *= uResolution.x/uResolution.y;\n"
"    vec3 ctmp = vec3(0.);\n"
"    ctmp = vec3(st.x, st.y, abs(sin(uTime)));\n"
"                                           \n"
"    //gl_FragColor = vec4(color.rgb,1.0);\n"
"    finalColor = vec4(color.rgb, 1.0 );\n"
"    //finalColor = vec4(color.rgb, color.a * (1 - length(gl_PointCoord.xy - vec2(0.5))*2));\n"
"}\n";

static const char* vertex_shader_text =
"#version 110\n"
"uniform float uTime;\n"
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec2 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"#version 110\n"
"varying vec3 color;\n"
"uniform float uTime;\n"
"void main()\n"
"{\n"
"    vec3 ctmp = vec3(0.);\n"
"    ctmp = vec3(color.r, color.b, (cos(uTime)) );\n"
"    gl_FragColor = vec4(ctmp.rgb, .5 );\n"
//"    gl_FragColor = vec4(ctmp.rgb, 1.0);\n"
"}\n";

// Get color normalized as float [0..1]
Vector4 ColorNormalize(Color color)
{
    Vector4 result;

    result.x = (float)color.r/255.0f;
    result.y = (float)color.g/255.0f;
    result.z = (float)color.b/255.0f;
    result.w = (float)color.a/255.0f;

    return result;
}

int GetRandomValue(int min, int max)
{
    int value = 0;

    if (min > max)
    {
        int tmp = max;
        max = min;
        min = tmp;
    }

#if defined(SUPPORT_RPRAND_GENERATOR)
    value = rprand_get_value(min, max);
#else
    // WARNING: Ranges higher than RAND_MAX will return invalid results
    // More specifically, if (max - min) > INT_MAX there will be an overflow,
    // and otherwise if (max - min) > RAND_MAX the random value will incorrectly never exceed a certain threshold
    // NOTE: Depending on the library it can be as low as 32767
    if ((unsigned int)(max - min) > (unsigned int)RAND_MAX2)
    {
        fprintf(stderr, "Error: Invalid GetRandomValue() arguments, range should not be higher than %i\n", RAND_MAX2);
        //TRACELOG(LOG_WARNING, "Invalid GetRandomValue() arguments, range should not be higher than %i", RAND_MAX);
        exit(1);
    }

    value = (rand()%(abs(max - min) + 1) + min);
#endif
    return value;
}

static void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void)
{
    GLFWwindow* window;
    GLuint vao, vbo, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location, utime_loc, res_loc;

    static Particle particles[MAX_PARTICLES];// = {0};
    
    static int screenWidth = 800;
    static int screenHeight = 600;
    
    glfwSetErrorCallback(ErrorCallback);

    if (!glfwInit())
        exit(EXIT_FAILURE);
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    
    window = glfwCreateWindow(screenWidth, screenHeight, "Base", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, KeyCallback);
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);
    glfwSwapInterval(0);
    
    //glShadeModel(GL_SMOOTH);
    //glEnable(GL_DEPTH_TEST);
    glPointSize(99.0);
    //glClearColor(245, 245, 245, 255);
    
    particles[MAX_PARTICLES] = (Particle){0};
    for (int i=0; i<MAX_PARTICLES; i++)
    {
        particles[i].x = GetRandomValue(10, 800 - 20)/2*50;
        particles[i].y = GetRandomValue(10, 600 - 20);
        
        // Give each particle a slightly different period. But don't spread it to much. 
        // This way the particles line up every so often and you get a glimps of what is going on.
        particles[i].period = (float)GetRandomValue(10, 30)/5.0f;
        //printf("num-%d\n", i);
    }
    
    // NOTE: OpenGL error checks have been omitted for brevity
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES*sizeof(particles), particles, GL_STATIC_DRAW);
    
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &defaultVS, NULL);
    glCompileShader(vertex_shader);
    
    GLint success = 0;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);    
    if (success == GL_FALSE)
    {
        fprintf(stderr, "Error: SHADER: [ID %i] Failed to compile vertex shader code\n", vertex_shader);
        int maxLength = 0;
        glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &maxLength);

        if (maxLength > 0)
        {
            int length = 0;
            char *log = (char *)calloc(maxLength, sizeof(char));
            glGetShaderInfoLog(vertex_shader, maxLength, &length, log);
            fprintf(stderr, "Error: SHADER: [ID %i] Failed to compile vertex shader code %s\n", program, log);
            //TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Compile error: %s", shader, log);
            free(log);
        }
    } else {
        fprintf(stderr, "[INFO]: SHADER: [ID %i] Vertex shader compiled successfully \n", vertex_shader);
    }
    
    printf("\ncompile-vertex: %d\n\n", success);
    
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &defaultFS, NULL);
    glCompileShader(fragment_shader);
    
    success = 0;
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);    
    if (success == GL_FALSE)
    {
        fprintf(stderr, "Error: SHADER: [ID %i] Failed to Fragment vertex shader code\n", fragment_shader);
        int maxLength = 0;
        glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &maxLength);

        if (maxLength > 0)
        {
            int length = 0;
            char *log = (char *)calloc(maxLength, sizeof(char));
            glGetShaderInfoLog(fragment_shader, maxLength, &length, log);
            fprintf(stderr, "Error: SHADER: [ID %i] Failed to compile vertex shader code %s\n", program, log);
            //TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Compile error: %s", shader, log);
            free(log);
        }
    } else {
        fprintf(stderr, "[INFO]: SHADER: [ID %i] Fragment shader compiled successfully \n", fragment_shader);
    }
    
    printf("\ncompile-fragment: %d\n\n", success);
    
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    
    glBindAttribLocation(program, 0, RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION);
    glBindAttribLocation(program, 1, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD);
    glBindAttribLocation(program, 2, RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL);
    glBindAttribLocation(program, 3, RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR);
    glBindAttribLocation(program, 4, RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT);
    glBindAttribLocation(program, 5, RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2);
    
    
    glLinkProgram(program);

    //GLint success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success == GL_FALSE)
    {
        //TRACELOG(RL_LOG_WARNING, "SHADER: [ID %i] Failed to link compute shader program", program);
        fprintf(stderr, "Error: SHADER: [ID %i] Failed to link compute shader program\n", program);
        
        int maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        if (maxLength > 0)
        {
            int length = 0;
            char *log = (char *)calloc(maxLength, sizeof(char));
            glGetProgramInfoLog(program, maxLength, &length, log);
            fprintf(stderr, "Error: SHADER: [ID %i] Link error: %s\n", program, log);
            free(log);
        }

        glDeleteProgram(program);

        program = 0;
    }

    mvp_location = glGetUniformLocation(program, "mvp");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "color");
    utime_loc = glGetUniformLocation(program, "uTime");
    res_loc = glGetUniformLocation(program, "uResolution");
    
    printf("\nloc: %d %d %d %d %d\n\n", mvp_location, vpos_location, vcol_location, utime_loc, res_loc );
    //exit(1);
    float res[2] = {screenWidth, screenHeight};
    glUniform2fv(res_loc, 1, (float *)&res);

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(particles[0]), (void*) 0);
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 0, 0);
    /*
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(vertices[0]), (void*) (sizeof(float) * 2));
    */
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    glEnable(GL_PROGRAM_POINT_SIZE);
    while (!glfwWindowShouldClose(window))
    {
        float ratio; 
        GLfloat utime;
        int width, height;
        mat4x4 m, p, mvp, projection;
        
        /*
        glMatrixMode(GL_PROJECTION);
        mat4x4_perspective(projection,
                           60.f * (float) M_PI / 180.f,
                           ratio,
                           1.f, 1024.f);
        glLoadMatrixf((const GLfloat*) projection);
        */
        //glMatrixMode(GL_MODELVIEW);
        //glLoadIdentity();
    
        glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
        ratio = screenWidth / (float) screenHeight;
        //printf("wsize: %d  %d ratio: %.2f \n", screenWidth, screenHeight, ratio);
        glViewport(0, 0, screenWidth, screenHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        utime = (float) glfwGetTime();
        mat4x4_identity(m);
        //mat4x4_rotate_Z(m, m, utime);
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);
    
        glUseProgram(program);
        Vector4 color = ColorNormalize((Color){ 0, 0, 255, 128 });
        float ctmp[4] = {color.x, color.y, color.z, color.w };
        glUniform4fv(vcol_location, 1, (float *)&ctmp);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
        //glUniform4f(RLGL.State.currentShaderLocs[RL_SHADER_LOC_COLOR_DIFFUSE], 1.0f, 1.0f, 1.0f, 1.0f);
        glUniform4f(utime_loc, 1.0f, 1.0f, 1.0f, utime/9);
        glBindVertexArray(vbo);
            glDrawArrays(GL_POINTS, 0, MAX_PARTICLES);
        glBindVertexArray(0);
        glUseProgram(0);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

//! [code]
