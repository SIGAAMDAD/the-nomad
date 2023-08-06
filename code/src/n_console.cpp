#include "n_shared.h"
#include "n_scf.h"
#include "../rendergl/imgui.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/ansicolor_sink.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/callback_sink.h>

#define MAX_CMD_LINE 1024
#define MAX_CMD_BUFFER 8192

// this allows vsnprintf to run on multiple threads without a lock
static thread_local char msg[MAX_MSG_SIZE];
static thread_local int32_t length;
static thread_local va_list argptr;

extern file_t logfile;
//static boost::mutex conLock;

bool imgui_window_open = false;
static eastl::vector<char> con_buffer;

void Con_ClearBuffer(void)
{
//    boost::unique_lock<boost::mutex> lock{conLock};
    EASY_FUNCTION();

    if (con_buffer.size() >= MAX_BUFFER_SIZE) {
        EASY_BLOCK("Console Buffer Resize");
        char *it = con_buffer.end() - MAX_BUFFER_SIZE;
        char *ptr = strrchr(it, '\n');
        
        if (ptr)
            con_buffer.erase(con_buffer.begin(), ptr);
    }
    con_buffer.insert(con_buffer.end(), msg, msg + length);
    con_buffer.emplace_back('\n');
}

void GDR_DECL Con_Printf(loglevel_t level, const char *fmt, ...)
{
    EASY_FUNCTION();

    memset(msg, 0, sizeof(msg));
    va_start(argptr, fmt);
    length = vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (length >= sizeof(msg)) {
        N_Error("Con_Printf: overflow occured");
    }

#ifndef _NOMAD_DEBUG
    if (level == DEBUG)
        return;
#else
    if (level == DEBUG)
        fprintf(stdout, C_BLUE "DEBUG: ");
#endif
    else if (level == DEV && !Cvar_VariableBoolean("c_devmode"))
        return;
    else if (level == DEV && Cvar_VariableBoolean("c_devmode"))
        fprintf(stdout, C_GREEN "DEVLOG: ");
    else if (level == WARNING)
        fprintf(stderr, C_YELLOW "WARNING: ");
    else if (level == ERROR)
        fprintf(stderr, C_RED "ERROR: ");
    
    fprintf(stdout, "%s\n" C_RESET, msg);
    if (logfile && FS_Initialized()) {
        FS_Write(msg, length, logfile);
        FS_Write("\n", 1, logfile);
    }

    Con_ClearBuffer();
}

void GDR_DECL Con_Printf(const char *fmt, ...)
{
    EASY_FUNCTION();

    memset(msg, 0, sizeof(msg));
    va_start(argptr, fmt);
    length = vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (length >= sizeof(msg)) {
        N_Error("Con_Printf: overflow occured");
    }

    fprintf(stdout, "%s\n", msg);
    if (logfile && FS_Initialized()) {
        FS_Write(msg, length, logfile);
        FS_Write("\n", 1, logfile);
    }
    Con_ClearBuffer();
}

void Con_GetInput(void)
{
    EASY_FUNCTION();

    int flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetWindowSize(ImVec2((float)Cvar_VariableInteger("r_screenwidth"), (float)(Cvar_VariableInteger("r_screenheight") / 2)));
    ImGui::Begin("Command Console", NULL, flags);
    con_buffer.emplace_back('\0');
    ImGui::Text("%s", Con_GetBuffer().data());
    con_buffer.clear();

    if (!RE_ConsoleIsOpen())
        return; // nothing to process
        
    char buffer[MAX_CMD_BUFFER];
    memset(buffer, 0, sizeof(buffer));
    
    ImGui::Text("> ");
    ImGui::SameLine();
    if (ImGui::InputText(" ", buffer, MAX_CMD_BUFFER, ImGuiInputTextFlags_EnterReturnsTrue)) {
        Con_Printf("]%s", buffer); // echo it into the console
        if (*buffer == '/') { // its a command
            Cmd_ExecuteText(buffer);
        }
    }

    ImGui::End();
}

static void Con_Clear_f(void)
{
    EASY_FUNCTION();
    con_buffer.clear();
    con_buffer.reserve(MAX_BUFFER_SIZE);
}

eastl::vector<char>& Con_GetBuffer(void)
{
    return con_buffer;
}

void GDR_DECL Con_Error(bool exit, const char *fmt, ...)
{
    EASY_FUNCTION();

    memset(msg, 0, sizeof(msg));
    va_start(argptr, fmt);
    length = vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    fprintf(stderr, C_RED "ERROR: %s\n" C_RESET, msg);
    if (logfile && FS_Initialized()) {
        FS_Write(msg, length, logfile);
        FS_Write("\n", 1, logfile);
    }
    if (exit)
        Sys_Exit(-1);
}

void Con_Shutdown(void)
{
    con_buffer.clear();
}

void Con_Init(void)
{
    con_buffer.reserve(MAX_BUFFER_SIZE);
}