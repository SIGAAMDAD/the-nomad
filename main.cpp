#include "../src/glad/src/glad.c"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <string>
#include <fstream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

uint32_t box_indices[] = {
    0, 1, 3,
    1, 2, 3
};

static GLuint R_CompileShader(const std::string& src, GLenum type)
{
    GLuint id = glCreateShader(type);
    const char *buffer = src.c_str();
    glShaderSource(id, 1, &buffer, NULL);
    glCompileShader(id);
    int success;
    char infolog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        glGetShaderInfoLog(id, sizeof(infolog), NULL, infolog);
        fprintf(stderr, "Error: failed to compile shader: %s\n", infolog);
        glDeleteShader(id);
        return 0;
    }
    return id;
}

static GLuint R_AllocShader(const char *shaderfile)
{
    std::ifstream file(shaderfile, std::ios::in);
    if (!file) {
        fprintf(stderr, "Error: failed to open shader file %s\n", shaderfile);
        return 0;
    }
    std::string line;
    int index;
    std::stringstream stream[2];
    while (std::getline(file, line)) {
        if (line == "#shader vertex")
            index = 0;
        else if (line == "#shader fragment")
            index = 1;
        else
            stream[index] << line << '\n';
    }
    const std::string vertsrc = stream[0].str();
    const std::string fragsrc = stream[1].str();
    file.close();
    GLuint vertid = R_CompileShader(vertsrc, GL_VERTEX_SHADER);
    GLuint fragid = R_CompileShader(fragsrc, GL_FRAGMENT_SHADER);

    GLuint shader = glCreateProgram();
    glAttachShader(shader, vertid);
    glAttachShader(shader, fragid);
    glLinkProgram(shader);
    glValidateProgram(shader);
    glDeleteShader(vertid);
    glDeleteShader(fragid);
    return shader;
}

int main()
{
    SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);

    float vertices[] = {
        100.0f, 100.0f,
        200.0f, 100.0f,
        200.0f, 200.0f
    };

    
    SDL_Window* window = SDL_CreateWindow("GLtest", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 720, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        fprintf(stderr, "Error: glad failed to initialize\n");
        return -1;
    }

    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));

    SDL_Event event;
    memset(&event, 0, sizeof(event));
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

    GLuint shader = R_AllocShader("shader.glsl");
    glUseProgram(shader);

    glm::mat4 proj = glm::ortho(0.0f, 960.0f, 0.0f, 540.0f, -1.0f, 1.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(-100, 0, 0));
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(200, 200, 0));
    glm::mat4 mvp = proj * view * model;

    glm::vec3 translation(100, 100, 0);

    GLint location = glGetUniformLocation(shader, "u_MVP");
    glUniformMatrix4fv(location, 1, GL_FALSE, &mvp[0][0]);

    while (1) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                goto done;
            }
            switch (event.key.keysym.sym) {
            case SDLK_w:
                translation.y += 10;
                break;
            case SDLK_a:
                translation.x -= 10;
                break;
            case SDLK_s:
                translation.y -= 10;
                break;
            case SDLK_d:
                translation.x += 10;
                break;
            };
        }
        model = glm::translate(glm::mat4(1.0f), translation);
        mvp = proj * view * model;

        glUniformMatrix4fv(location, 1, GL_FALSE, &mvp[0][0]);

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        SDL_GL_SwapWindow(window);
    }
done:
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &buffer);
    glDeleteProgram(shader);
    SDL_DestroyWindow(window);
    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return 0;
}