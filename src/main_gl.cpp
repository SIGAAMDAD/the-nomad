#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <stb/stb_image.h>

#undef NULL
#define NULL 0



uint32_t box_indices[] = {
    0, 1, 3,
    1, 2, 3
};

static GLuint R_CompileShader(const char* filepath, GLenum type)
{
    std::ifstream file(filepath, std::ios::in);
    if (file.fail()) {
        fprintf(stderr, "Error: failed to open shader file %s\n", filepath);
        return 0;
    }
    std::stringstream str;
    std::string line;
    while (std::getline(file, line)) {
        str << line << '\n';
    }
    const std::string& buffer = str.str();
    GLuint id = glCreateShader(type);
    const char *buf = buffer.c_str();
    glShaderSource(id, 1, &buf, NULL);
    glCompileShader(id);
    int success;
    char infoLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        fprintf(stderr, "Error: failed to compile shader: %s\n", infoLog);
        glDeleteShader(id);
        return 0;
    }
    return id;
}

GLuint R_AllocShader(const char* vertfile, const char* fragfile)
{
    GLuint vertid = R_CompileShader(vertfile, GL_VERTEX_SHADER);
    GLuint fragid = R_CompileShader(fragfile, GL_FRAGMENT_SHADER);

    GLuint shader = glCreateProgram();
    glAttachShader(shader, vertid);
    glAttachShader(shader, fragid);
    glLinkProgram(shader);
    glValidateProgram(shader);
    glDeleteShader(vertid);
    glDeleteShader(fragid);
    return shader;
}

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_Window* window = SDL_CreateWindow("GL Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1080, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext context = SDL_GL_CreateContext(window);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        return -1;
    
    glDisable(GL_DEPTH_TEST);
    
}