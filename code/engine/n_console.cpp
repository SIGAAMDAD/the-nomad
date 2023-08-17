#include "n_shared.h"
#include "n_scf.h"
#include "../rendergl/imgui.h"
#include "../rendergl/rgl_public.h"

typedef struct
{
    char buf[MAX_BUFFER_SIZE];
    char colors[MAX_BUFFER_SIZE];
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

void Con_InsertText(const char *msg, int length)
{
    boost::lock_guard<boost::shared_mutex> lock{conLock};

    // move the old stuff out of the buffer
    if (con.bytesUsed + length >= MAX_BUFFER_SIZE) {
        memmove(con.buf, con.buf + (con.bytesUsed / 2), con.bytesUsed / 2);
        char *p = con.buf;

        while (*p != '\n') {
            *p = '\0';
            ++p;
        }
    }
    memcpy(con.buf + con.bytesUsed, msg, length);
    con.bytesUsed += length;
    con.buf[con.bytesUsed] = '\n';
}

static void Con_IPrintf(const char *fmt, ...)
{
    va_list argptr;
    int length;
    char msg[MAXPRINTMSG];

    va_start(argptr, fmt);
    length = N_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    Con_TextColor(msg, length);
}

void Con_ProcessTextColor(const char *in, int length)
{
    ImVec4 colorVec;
    const char *colorStr;
    constexpr int len = 6;
    char *buf, *buf_p;

    buf = (char *)alloca(length + 1);
    buf_p = buf;

    for (int i = 0; i < length; i++, in++) {
        if (*in == '\\' && *(in + 1) == '^') {
            in += 2;
            switch (*in) {
            case '0':
                colorStr = C_YELLOW;
                colorVec = { colorYellow[0], colorYellow[1], colorYellow[2], colorYellow[3] };
                break;
            case '1':
                colorStr = C_RED;
                colorVec = { colorRed[0], colorRed[1], colorRed[2], colorRed[3] };
                break;
            case '2':
                colorStr = C_GREEN;
                colorVec = { colorGreen[0], colorGreen[1], colorGreen[2], colorGreen[3] };
                break;
            case '3':
                colorStr = C_BLUE;
                colorVec = { colorBlue[0], colorBlue[1], colorBlue[2], colorBlue[3] };
                break;
            case '4':
                colorStr = C_BLACK;
                colorVec = { colorBlack[0], colorBlack[1], colorBlack[2], colorBlack[3] };
                break;
            case '5':
                colorStr = C_MAGENTA;
                colorVec = { colorMagenta[0], colorMagenta[1], colorMagenta[2], colorMagenta[3] };
                break;
            case '6':
                colorStr = C_WHITE;
                colorVec = { colorWhite[0], colorWhite[1], colorWhite[2], colorWhite[3] };
                break;
            case '7':
                colorStr = C_GRAY;
                colorVec = { colorMdGrey[0], colorMdGrey[1], colorMdGrey[2], colorMdGrey[3] };
                break;
            case '8':
            default:
                break;
            };

            if (*in == '8') {
                ImGui::Text("%c", *in);
                *buf_p++ = *in;
                continue;
            }

            strcpy(buf_p, colorStr);
            buf_p += len;
            *buf_p++ = *in;

            ImGui::TextColored(colorVec, "%c", *in);
        }
        else {
            *buf_p++ = *in;
            ImGui::Text("%c", *in);
        }
    }
    Sys_Printf("%s", buf);
}

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
    length = N_vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (length >= sizeof(msg)) {
        N_Error("Con_Printf: overflow occured of %i bytes, buffer size is %lu bytes", length, sizeof(msg));
    }

#ifndef _NOMAD_DEBUG
    if (level == DEBUG)
        return;
#else
    if (level == DEBUG)
        Con_IPrintf(COLOR_BLUE "DEBUG: ");
#endif
    else if (level == DEV && !Cvar_VariableBoolean("c_devmode"))
        return;
    else if (level == DEV && Cvar_VariableBoolean("c_devmode"))
        Sys_Printf(COLOR_GREEN "DEVLOG: ");
    else if (level == WARNING)
        Sys_Printf(COLOR_YELLOW "WARNING: ");
    else if (level == ERROR)
        Sys_Printf(COLOR_RED "ERROR: ");
    
    Sys_Printf("%s\n" COLOR_RESET, msg);
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
    length = N_vsnprintf(msg, sizeof(msg), fmt, argptr);
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
    
    ImGui::Text("] ");
    ImGui::SameLine();
    if (ImGui::InputText(" ", buffer, MAX_CMD_BUFFER, ImGuiInputTextFlags_EnterReturnsTrue)) {
        Con_Printf("]%s", buffer); // echo it into the console
        if (*buffer == '/' || *buffer == '\\') { // its a command
            Cbuf_ExecuteText(EXEC_NOW, buffer);
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