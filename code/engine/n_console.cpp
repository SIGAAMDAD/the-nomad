#include "n_shared.h"
#include "n_scf.h"
#include "../rendergl/imgui.h"
#include "../rendergl/rgl_public.h"

typedef struct
{
    uint16_t buf[MAX_CMD_BUFFER];
    uint32_t bytesUsed;
    uint32_t curlines;
    uint32_t maxlines;
    uint32_t current;

    uint32_t x;
    uint32_t linewidth;

    qboolean newline;
    qboolean initialized;
} console_t;

extern file_t logfile;

static console_t con;
static cvar_t *con_noprint;

bool imgui_window_open = false;
static boost::shared_mutex conLock;
static eastl::vector<char> con_buffer;

void Con_ClearBuffer(const char *msg, int length)
{
    boost::lock_guard<boost::shared_mutex> lock{conLock};

    if (con_buffer.size() >= MAX_BUFFER_SIZE) {
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
    char msg[MAXPRINTMSG];
    va_list argptr;
    int length;

    va_start(argptr, fmt);
    length = stbsp_vsprintf(msg, fmt, argptr);
    va_end(argptr);

    if (length >= sizeof(msg)) {
        N_Error("Con_Printf: overflow occured of %i bytes, buffer size is %lu bytes", length, sizeof(msg));
    }

#ifndef _NOMAD_DEBUG
    if (level == DEBUG)
        return;
#else
    if (level == DEBUG)
        Sys_Printf(C_BLUE "DEBUG: ");
#endif
    else if (level == DEV && !Cvar_VariableBoolean("c_devmode"))
        return;
    else if (level == DEV && Cvar_VariableBoolean("c_devmode"))
        Sys_Printf(C_GREEN "DEVLOG: ");
    else if (level == WARNING)
        Sys_Printf(C_YELLOW "WARNING: ");
    else if (level == ERROR)
        Sys_Printf(C_RED "ERROR: ");
    
    Sys_Printf("%s\n" C_RESET, msg);
    if (logfile && FS_Initialized()) {
        FS_Write(msg, length, logfile);
        FS_Write("\n", 1, logfile);
    }

    Con_ClearBuffer(msg, length);
}

void GDR_DECL Con_Printf(const char *fmt, ...)
{
    char msg[MAXPRINTMSG];
    va_list argptr;
    int length;

    va_start(argptr, fmt);
    length = stbsp_vsprintf(msg, fmt, argptr);
    va_end(argptr);

    if (length >= sizeof(msg)) {
        N_Error("Con_Printf: overflow occured");
    }

    Sys_Printf("%s\n", msg);
    if (logfile && FS_Initialized()) {
        FS_Write(msg, length, logfile);
        FS_Write("\n", 1, logfile);
    }
    Con_ClearBuffer(msg, length);
}

void Con_GetInput(void)
{
    int flags = ImGuiWindowFlags_NoCollapse;

    ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetWindowSize(ImVec2(Cvar_VariableInteger("r_screenwidth"), Cvar_VariableInteger("r_screenheight") / 2));
    ImGui::Begin("Command Console", NULL, flags);
    con_buffer.emplace_back('\0');
    ImGui::Text("%s", con_buffer.data());
    con_buffer.pop_back();

    if (!RE_ConsoleIsOpen())
        return; // nothing to process
    
    char buffer[MAX_CMD_BUFFER];
    
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
    con_buffer.clear();
    con_buffer.reserve(MAX_BUFFER_SIZE);
}

eastl::vector<char>& Con_GetBuffer(void)
{
    return con_buffer;
}

void GDR_DECL Con_Error(bool exit, const char *fmt, ...)
{
    char msg[MAXPRINTMSG];
    va_list argptr;
    int length;

    va_start(argptr, fmt);
    length = vsprintf(msg, fmt, argptr);
    va_end(argptr);

    Sys_Printf(C_RED "ERROR: %s\n" C_RESET, msg);
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