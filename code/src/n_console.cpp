#include "n_shared.h"
#include "m_renderer.h"
#include "n_scf.h"

#define MAX_CMD_LINE 1024
#define MAX_CMD_BUFFER 8192

enum class LogLevel
{
    LOG_INFO,
    LOG_DEBUG,
    LOG_DEV,
    LOG_ERR,
};

#ifdef __unix__
#define STDOUT_HANDLE STDOUT_FILENO
#define STDERR_HANDLE STDERR_FILENO
#define BAD_HANDLE NULL
typedef FILE* consoleHandle_t;
#define OpenConsoleHandle(fd) fdopen(fd,"w")
#define CloseConsoleHandle(fd) fclose(fd)
#elif defined(_WIN32)
#define STDOUT_HANDLE STD_OUT_HANDLE
#define STDERR_HANDLE STD_ERROR_HANDLE
#define BAD_HANDLE INVALID_HANDLE_VALUE
typedef HANDLE consoleHandle_t;
#define OpenConsoleHandle(fd) GetStdHandle(fd)
#define CloseConsoleHandle(fd) CloseHandle(fd)
#endif

#if 0

class GDRConsole
{
private:
    std::mutex m_conLock;
    GDRCondVar 
};

typedef struct
{
    consoleHandle_t stderr;
    consoleHandle_t stdout;

    char msg[MAX_MSG_SIZE];
    std::mutex m_conLock;
    std::condition_variable m_conVar;
} cmdConsole_t;

static cmdConsole_t conInfo;

void Con_Write(consoleHandle_t fd, bool nline = false)
{
#ifdef __unix__
    size_t bytesWritten;
    bytesWritten = fwrite((const void *)msg, sizeof(char), length, fd);
    if (bytesWritten != length)
        N_Error("Con_Write: bad fwrite!");
    if (nline) {
        bytesWritte = fwrite((const void *)"\n", sizeof(char), 1, fd);
        if (bytesWritten != 1)
            N_Error("Con_Write: bad fwrite!");
    }
    
#elif defined(_WIN32)
    DWORD bytesWritten;
    WriteConsoleA(conHandle, (const void *)msg, length, &bytesWritten, NULL);
//    if (bytesWritten != length) // ...?
    WriteConsoleA(conHandle, (const void *)"\n", 1, &bytesWritten, NULL);
//    if (bytesWritten != 1) // ...?
#endif
}

void pop_msg(void)
{
    std::unique_lock<std::mutex> lock{conLock};
    m_conVar.wait(lock, [this](){ return !m_msgQueue.empty(); });
    const eastl::string& msgBuf = m_msgQueue.front();
    m_msgQueue.pop();
}

void push_msg(LogLevel lvl, const char *fmt, ...)-
{
    va_list argptr;
    char msg[MAX_MSG_SIZE];
    int32_t length;

    va_start(argptr, fmt);
    length = vsnprintf(msg, MAX_MSG_SIZE, fmt, argptr);
    va_end(argptr);

    std::unique_lock<std::mutex> lock{conLock};
    m_msgQueue.push(conInfo.msgBuf);
    m_conVar.notify_one();
}
#endif

// thread-safety
static std::mutex conLock;
static thread_local va_list argptr;
static thread_local char msg[MAX_MSG_SIZE];
static thread_local int32_t length;

bool imgui_window_open = false;
static eastl::vector<char> con_buffer;

void Con_ClearBuffer(const char *msg, int32_t length)
{
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

    std::unique_lock<std::mutex> lock{conLock};
    if (level == DEV) {
        if (!c_devmode.b) {
            return; // don't print it if we're not in devmode
        }
        write(STDOUT_FILENO, "DEV: ", 5);
    }
    else if (level == DEBUG) {
#ifdef _NOMAD_DEBUG
        write(STDOUT_FILENO, "DEBUG: ", 7);
#else
        return;
#endif
    }
    write(STDOUT_FILENO, (const void *)msg, length);
    write(STDOUT_FILENO, "\n", 1);
    Con_ClearBuffer(msg, length);
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

    std::unique_lock<std::mutex> lock{conLock};
    write(STDOUT_FILENO, (const void *)msg, length);
    Con_ClearBuffer(msg, length);
}

static void Con_Clear_f(void)
{
    EASY_FUNCTION();
    con_buffer.clear();
    con_buffer.reserve(MAX_BUFFER_SIZE);
}

static void Con_ProcessInput(void)
{
    EASY_FUNCTION();

    if (!console_open)
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
}

void Con_EndFrame(void)
{
    int flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;

    ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetWindowSize(ImVec2((float)r_screenwidth.i, (float)(r_screenheight.i >> 1)));
    ImGui::Begin("Command Console", NULL, flags);
    con_buffer.emplace_back('\0');
    ImGui::Text("%s", con_buffer.data());
    con_buffer.pop_back();

    Con_ProcessInput();

    ImGui::End();
    if (console_open) {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}

void GDR_DECL Con_Error(const char *fmt, ...)
{
    EASY_FUNCTION();

    memset(msg, 0, sizeof(msg));
    va_start(argptr, fmt);
    length = vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    std::unique_lock<std::mutex> lock{conLock};
    write(STDERR_FILENO, "ERROR: ", 7);
    write(STDERR_FILENO, msg, length);
}

#define MAX_CVAR_HASH 2048
static cvar_t cvar_list;
static uint64_t cvar_count;

const char *Cvar_GetValue(const cvar_t* cvar);

cvar_t* Cvar_Find(const char *name)
{
    cvar_t *cvar;
    
    for (cvar = &cvar_list; cvar; cvar = cvar->next) {
        if (N_strcmp(cvar->name, name) == 1) {
            return cvar;
        }
    }
    return NULL;
}

static void Cvar_Set(const char *name, const char *value)
{
    cvar_t *cvar;
    int32_t i;
    char s[64];
    float f;

    if (!Cvar_Find(name) || strlen(name) >= 64) {
        Con_Printf("Invalid cvar name string %s", name);
        return;
    }
    if (strlen(value) >= 64) {
        Con_Printf("Invalid cvar value string %s", value);
    }

    cvar = Cvar_Find(name);
    f = N_atof(value);
    i = N_atoi(value);
    memset(cvar->s, 0, sizeof(cvar->s));
    N_strncpy(cvar->s, value, sizeof(cvar->s));

    if (i == (int32_t)f) {
        if (cvar->type != TYPE_INT) {
            Con_Printf("Cannot change cvar value type for cvar '%s'", name);
            return;
        }
        cvar->i = i;
    }
    else {
        if (cvar->type != TYPE_FLOAT && cvar->type != TYPE_STRING) {
            Con_Printf("Cannot change cvar value type for cvar '%s'", name);
            return;
        }
        cvar->f = f;
    }
}

void Cvar_Reset(const char *name)
{
    cvar_t *cvar;

    cvar = Cvar_Find(name);
    if (!cvar) {
        Con_Printf("Cvar_Reset: cvar %s doesn't exist", name);
        return;
    }
    
    if (cvar->flags & CVAR_DEV && !c_devmode.b) {
        Con_Printf("Cvar_Reset: cannot change value of a devmode only variable when not in devmode");
        return;
    }
    if (cvar->flags & CVAR_CHEAT && !c_cheatsallowed.b) {
        Con_Printf("Cvar_Reset: cheats are currently disabled");
        return;
    }
    if (cvar->flags & CVAR_ROM) {
        Con_Printf("Cvar_Reset: cannot change read-only cvar %s", name);
        return;
    }

    switch (cvar->type) {
    case TYPE_BOOL:
        cvar->b = qfalse;
        break;
    case TYPE_FLOAT:
        cvar->f = 0.0f;
        break;
    case TYPE_INT:
        cvar->i = 0;
        break;
    case TYPE_STRING:
        memset(cvar->s, 0, sizeof(cvar->s));
        N_strcpy(cvar->s, "null");
        break;
    };
}

static void Cvar_Print(const cvar_t *var)
{
    Con_Printf("'%s' is: '%s'", Cmd_Argv(1), Cvar_GetValue(var));
}


qboolean Cvar_Command(void)
{
    cvar_t *cvar;

    // check variables
    cvar = Cvar_Find(Cmd_Argv(0));
    if (!cvar) {
        return qfalse;
    }

    // perform a variable print or set
    if (Cmd_Argc() == 1) {
        Cvar_Print(cvar);
        return qtrue;
    }

    // set the value
    Cvar_Set(cvar->name, Cmd_ArgsFrom(1));
    return qtrue;
}

const char *Cvar_GetValue(const char *name)
{
    static char str[64];
    cvar_t *cvar;

    memset(str, 0, sizeof(str));
    cvar = Cvar_Find(name);
    if (!cvar) {
        Con_Printf("WARNING: attempted to retrieve cvar value from non-existent cvar");
        return "(null)";
    }

    switch (cvar->type) {
    case TYPE_BOOL:
        if (cvar->b)
            N_strncpy(str, "true", 64);
        else
            N_strncpy(str, "false", 64);
        break;
    case TYPE_INT:
        snprintf(str, 64, "%i", cvar->i);
        break;
    case TYPE_FLOAT:
        snprintf(str, 64, "%f", cvar->f);
        break;
    case TYPE_STRING:
        N_strncpy(str, cvar->s, strlen(cvar->s));
        break;
    };

    return str;
}

const char *Cvar_GetValue(const cvar_t* cvar)
{
    static char str[64];
    memset(str, 0, sizeof(str));
    switch (cvar->type) {
    case TYPE_BOOL:
        if (cvar->b)
            N_strncpy(str, "true", 64);
        else
            N_strncpy(str, "false", 64);
        break;
    case TYPE_INT:
        snprintf(str, 64, "%i", cvar->i);
        break;
    case TYPE_FLOAT:
        snprintf(str, 64, "%6.03f", cvar->f);
        break;
    case TYPE_STRING:
        N_strncpy(str, cvar->s, 63);
        break;
    };

    return str;
}

static void Cvar_Print_f(void)
{
	const char *name;
	cvar_t *cv;
	
	if (Cmd_Argc() != 2) {
		Con_Printf("usage: print <variable>");
		return;
	}

	name = Cmd_Argv(1);

	cv = Cvar_Find(name);
	
	if (cv)
		Cvar_Print(cv);
	else
		Con_Printf("Cvar '%s' does not exist", name);
}

void Cvar_RegisterName(const char *name, const char *value, cvartype_t type, int32_t flags)
{
    cvar_t* cvar;

    if (Cvar_Find(name)) {
        return;
    }

    for (cvar = &cvar_list; cvar->next; cvar = cvar->next);
    cvar->next = (cvar_t *)Z_Malloc(sizeof(*cvar), TAG_STATIC, &cvar->next, "cvar");
    cvar = cvar->next;
    
    cvar->flags = flags;
    N_strncpy(cvar->name, name, sizeof(cvar->name));
    cvar->name[sizeof(cvar->name) - 1] = '\0';

    switch (type) {
    case TYPE_BOOL:
        if (N_strneq("true", value, 4))
            cvar->b = qtrue;
        else
            cvar->b = qfalse;
        break;
    case TYPE_INT:
        cvar->i = N_atoi(value);
        break;
    case TYPE_FLOAT:
        cvar->f = N_atof(value);
        break;
    case TYPE_STRING:
        if (strlen(value) >= 64)
            N_Error("Cvar_RegisterName: cvar values musn't be greater than 64 characters long");
        N_strncpy(cvar->s, value, 63);
        cvar->s[63] = '\0';
        break;
    };

    cvar_count++;
    cvar->type = type;
}

void Cvar_ChangeValue(const char *name, const char *value)
{
    cvar_t* cvar;

    cvar = Cvar_Find(name);
    if (!cvar) {
        Con_Printf("WARNING: attempting to change the value of non-existent cvar %s", name);
        return;
    }

    if (cvar->flags & CVAR_ROM) {
        if (!c_devmode.b) {
            Con_Printf("Cvar_ChangeValue: cannot change read-only cvar when not in dev mode");
            return;
        }
    }
    if (cvar->flags & CVAR_DEV && !c_devmode.b) {
        Con_Printf("Cvar_ChangeValue: cannot change devmode cvar when not in devmode");
        return;
    }
    if (cvar->flags & CVAR_CHEAT && !c_cheatsallowed.b) {
        Con_Printf("Cvar_ChangeValue: cheats are currently disabled");
        return;
    }

    switch (cvar->type) {
    case TYPE_BOOL:
        if (N_strneq("true", value, 4))
            cvar->b = qtrue;
        else
            cvar->b = qfalse;
        break;
    case TYPE_INT:
        cvar->i = N_atoi(value);
        break;
    case TYPE_FLOAT:
        cvar->f = N_atof(value);
        break;
    case TYPE_STRING:
        if (strlen(value) >= 64)
            N_Error("Cvar_ChangeValue: cvar strings musn't be greater than 64 characters long");
        N_strncpy(cvar->s, value, strlen(value));
        cvar->s[63] = '\0';
        break;
    };
}

static void Cvar_ChangeValue(cvar_t *cvar, const char *value)
{
    if (cvar->flags & CVAR_ROM) {
        if (!c_devmode.b) {
            Con_Printf("Cvar_ChangeValue: cannot change read-only cvar when not in dev mode");
            return;
        }
    }
    if (cvar->flags & CVAR_DEV && !c_devmode.b) {
        Con_Printf("Cvar_ChangeValue: cannot change devmode cvar when not in devmode");
        return;
    }
    if (cvar->flags & CVAR_CHEAT && !c_cheatsallowed.b) {
        Con_Printf("Cvar_ChangeValue: cheats are currently disabled");
        return;
    }

    switch (cvar->type) {
    case TYPE_BOOL:
        if (N_strneq("true", value, 4))
            cvar->b = qtrue;
        else
            cvar->b = qfalse;
        break;
    case TYPE_INT:
        cvar->i = N_atoi(value);
        break;
    case TYPE_FLOAT:
        cvar->f = N_atof(value);
        break;
    case TYPE_STRING:
        if (strlen(value) >= 64)
            N_Error("Cvar_RegisterName: cvar values musn't be greater than 64 characters long");
        N_strncpy(cvar->s, value, 63);
        cvar->s[63] = '\0';
        break;
    };
}

// Cvar_ValueCheck: if the value param is different from the given cvar's value, change it, otherwise, return
static void Cvar_ValueCheck(cvar_t *cvar, const char *value)
{
    switch (cvar->type) {
    case TYPE_BOOL:
        if ((qboolean)N_strtobool(value) != cvar->b)
            cvar->b = (qboolean)N_strtobool(value);
        break;
    case TYPE_INT:
        if (N_atoi(value) != cvar->i)
            cvar->i = N_atoi(value);
        break;
    case TYPE_FLOAT:
        if (N_atof(value) != cvar->f)
            cvar->f = N_atof(value);
        break;
    case TYPE_STRING: {
        if (strlen(value) >= 64)
            N_Error("Cvar_RegisterName: cvar values musn't be greater than 64 characters long");
        if (!N_strncmp(cvar->s, value, sizeof(cvar->s))) {
            N_strncpy(cvar->s, value, 63);
            cvar->s[63] = '\0';
        }
        break; }
    };
}

void Cvar_Register(cvar_t* cvar, const char *value)
{
    cvar_t* other;

    other = Cvar_Find(cvar->name);
    if (other) { // already exists
        Con_Printf("Cvar_Register: cvar %s already exists", cvar->name);
        if (value)
            Cvar_ValueCheck(cvar, value);
        return;
    }
    for (other = &cvar_list;; other = other->next) {
        if (!other->next) {
            other->next = cvar;
            break;
        }
    }

    cvar_count++;
    cvar->next = NULL;
    if (value)
        Cvar_ValueCheck(cvar, value);
}

#define DEFAULT_CFG_NAME "nomadconfig.scf"

static void Cvar_WriteCfg_f(void)
{
    cvar_t* cvar;

    file_t out = FS_FOpenWrite("default.scf");
    if (out == FS_INVALID_HANDLE) {
        N_Error("FS_FOpenWrite: failed to open write stream for default.scf");
    }
    FS_Write("{\n", 2, out);
    for (cvar = cvar_list.next; cvar; cvar = cvar->next) {
        if (!(cvar->flags & CVAR_SAVE))
            continue; // don't write it

        char buffer[256];
        snprintf(buffer, sizeof(buffer), "\t\"%s\": \"%s\"", cvar->name, Cvar_GetValue(cvar));
        FS_Write(buffer, strlen(buffer), out);
        if (cvar->next) { // not the end of the list
            FS_Write(",\n", 2, out);
        }
        else {
            FS_Write("\n", 1, out);
        }
    }
    FS_Write("}\n", 2, out);
    FS_FClose(out);
}

static void Cvar_List_f(void)
{
    for (cvar_t *cvar = cvar_list.next; cvar; cvar = cvar->next) {
        Con_Printf("%s%24 = %s", cvar->name, Cvar_GetValue(cvar));
    }
}

static cvartype_t Cvar_StringToType(const char *str)
{
    if (N_strcmp("boolean", str) == 1) return TYPE_BOOL;
    else if (N_strcmp("float", str) == 1) return TYPE_FLOAT;
    else if (N_strcmp("integer", str) == 1) return TYPE_INT;
    else if (N_strcmp("string", str) == 1) return TYPE_STRING;

    Con_Printf("Unkown cvar type string: %s", str);
    return (cvartype_t)-1;
}

static const char *Cvar_TypeToString(const cvar_t *cvar)
{
    switch (cvar->type) {
    case TYPE_BOOL: return "boolean";
    case TYPE_FLOAT: return "float";
    case TYPE_INT: return "integer";
    case TYPE_STRING: return "string";
    };
    N_Error("unkown cvar type for cvar %s", cvar->name);
}

static void Cvar_Set_f(void)
{
    uint32_t c;
    const char *cmd;
    cvar_t *cvar;

    c = Cmd_Argc();
    cmd = Cmd_Argv(0);

    if (c < 2) {
        Con_Printf("usage: %s <variable> <type> <value>", cmd);
        return;
    }
    if (c == 2) {
        Cvar_Print_f();
        return;
    }

    cvar = Cvar_Find(Cmd_Argv(1));
    if (!cvar) {
        if (Cvar_StringToType(Cmd_Argv(2)) == -1) {
            Con_Printf("Invalid cvar type, types are: boolean, float, integer, or string");
            return;
        }
        Cvar_RegisterName(Cmd_Argv(1), Cmd_ArgsFrom(3), Cvar_StringToType(Cmd_Argv(2)), CVAR_USER_CREATED);
        cvar = Cvar_Find(Cmd_Argv(1));
    }
    else {
        if (Cvar_StringToType(Cmd_Argv(2)) != cvar->type) {
            Con_Printf("Cvar type for %s isn't the same as %s, use the correct type", cvar->name, Cmd_Argv(2));
        }
        Cvar_ChangeValue(cvar, Cmd_ArgsFrom(2));
    }
    
    switch (cmd[3]) {
    case 's':
        if (!(cvar->flags & CVAR_SAVE))
            cvar->flags |= CVAR_SAVE;
        break;
    case 'd': {
        if (!c_devmode.b) {
            Con_Printf("Cvar_Set: (setd) not in devmode");
            break;
        }
        if (!(cvar->flags & CVAR_DEV))
            cvar->flags |= CVAR_DEV;
        break; }
    case 'u':
        if (!(cvar->flags & CVAR_USER_CREATED))
            cvar->flags |= CVAR_USER_CREATED;
        break;
    };
}

static void Cvar_Reset_f(void)
{
    if (Cmd_Argc() != 2) {
        Con_Printf("usage: reset <variable>");
        return;
    }
    Cvar_Reset(Cmd_Argv(1));
}

static void Cvar_PrintType_f(void)
{
    if (Cmd_Argc() != 2) {
        Con_Printf("usage: print_type <variable>");
        return;
    }
    const cvar_t *cvar = Cvar_Find(Cmd_Argv(1));
    if (!cvar) {
        Con_Printf("Cvar %s doesn't exist", Cmd_Argv(1));
        return;
    }
    Con_Printf("Type of cvar %s is %s", cvar->name, Cvar_TypeToString(cvar));
}

void Con_Shutdown(void)
{
#if 0
    CloseConsoleHandle(conInfo.stdout);
    CloseConsoleHandle(conInfo.stderr);
#endif

#ifdef _WIN32
    if (!FreeConsole()) {

    }
#endif
}

void Con_Init()
{
#ifdef _WIN32
    if (!AllocConsole()) {

    }
#endif

#if 0
    conInfo.stdout = OpenConsoleHandle(STDOUT_HANDLE);
    if (conInfo.stdout == BAD_HANDLE)
        N_Error();

    conInfo.stderr = OpenConsoleHandle(STDERR_HANDLE)
    if (conInfo.stderr == BAD_HANDLE)
        N_Error();
#endif

    con_buffer.reserve(MAX_BUFFER_SIZE);
    Cmd_AddCommand("writecfg", Cvar_WriteCfg_f);
    Cmd_AddCommand("listcvars", Cvar_List_f);
    Cmd_AddCommand("print", Cvar_Print_f);
    Cmd_AddCommand("clear", Con_Clear_f);
    Cmd_AddCommand("sets", Cvar_Set_f);
    Cmd_AddCommand("setu", Cvar_Set_f);
    Cmd_AddCommand("setd", Cvar_Set_f);
    Cmd_AddCommand("reset", Cvar_Reset_f);
    Cmd_AddCommand("print_type", Cvar_PrintType_f);

    cvar_list.next = NULL;
    memset(cvar_list.name, 0, sizeof(cvar_list.name));
    memset(cvar_list.s, 0, sizeof(cvar_list.s));
    cvar_list.i = 0;
    cvar_list.f = 0.0f;
    cvar_list.flags = 0;
    cvar_list.type = TYPE_STRING;

    for (uint32_t i = 0; i < G_NumCvars(); i++) {
        Cvar_Register(G_GetCvars()[i], NULL);
    }
}