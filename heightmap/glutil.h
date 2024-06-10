#ifndef GL_UTIL_H
#define GL_UTIL_H

    static GLuint CreateShader(GLenum type, const char* text);
    static GLuint CreateShaderProgram(const char* vs_text, const char* fs_text);    



#endif /* GL_UTIL_H */

#if defined GL_UTIL_IMPLEMENTATION
    /* implementation here */
    /**********************************************************************
     * OpenGL helper functions
     *********************************************************************/

    /* Creates a program object using the specified vertex and fragment text
     */
    static GLuint CreateShaderProgram(const char* vs_text, const char* fs_text)
    {
        GLuint program = 0u;
        GLint program_ok;
        GLuint vertex_shader = 0u;
        GLuint fragment_shader = 0u;
        GLsizei log_length;
        char info_log[8192];

        vertex_shader = CreateShader(GL_VERTEX_SHADER, vs_text);
        if (vertex_shader != 0u)
        {
            fragment_shader = CreateShader(GL_FRAGMENT_SHADER, fs_text);
            if (fragment_shader != 0u)
            {
                /* make the program that connect the two shader and link it */
                program = glCreateProgram();
                if (program != 0u)
                {
                    /* attach both shader and link */
                    glAttachShader(program, vertex_shader);
                    glAttachShader(program, fragment_shader);
                    glLinkProgram(program);
                    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);

                    if (program_ok != GL_TRUE)
                    {
                        fprintf(stderr, "ERROR, failed to link shader program\n");
                        glGetProgramInfoLog(program, 8192, &log_length, info_log);
                        fprintf(stderr, "ERROR: \n%s\n\n", info_log);
                        glDeleteProgram(program);
                        glDeleteShader(fragment_shader);
                        glDeleteShader(vertex_shader);
                        program = 0u;
                    }
                }
            }
            else
            {
                fprintf(stderr, "ERROR: Unable to load fragment shader\n");
                glDeleteShader(vertex_shader);
            }
        }
        else
        {
            fprintf(stderr, "ERROR: Unable to load vertex shader\n");
        }
        return program;
    }



    /* Creates a shader object of the specified type using the specified text
     */
    static GLuint CreateShader(GLenum type, const char* text)
    {
        GLuint shader;
        GLint shader_ok;
        GLsizei log_length;
        char info_log[8192];

        shader = glCreateShader(type);
        if (shader != 0)
        {
            glShaderSource(shader, 1, (const GLchar**)&text, NULL);
            glCompileShader(shader);
            glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
            if (shader_ok != GL_TRUE)
            {
                fprintf(stderr, "ERROR: Failed to compile %s shader\n", (type == GL_FRAGMENT_SHADER) ? "fragment" : "vertex" );
                glGetShaderInfoLog(shader, 8192, &log_length,info_log);
                fprintf(stderr, "ERROR: \n%s\n\n", info_log);
                glDeleteShader(shader);
                shader = 0;
            }
        }
        return shader;
    }
#endif
