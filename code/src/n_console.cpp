#include "n_shared.h"
#include "g_game.h"

typedef struct cvar_s
{
    char* name;
    void* value;
} cvar_t;

typedef struct cmd_s
{
    char* name;
    cmdfunc_t cmdfunc;
} cmd_t;

static std::unordered_map<std::string, cvar_t*> cvar_list;
static std::unordered_map<std::string, cmd_t*> cmd_list;
static nomadvector<eastl::string> cmd_history;

void Cmd_AddCommand(const char* name, cmdfunc_t func)
{
    if (cmd_list.find(name) != cmd_list.end()) {
        LOG_WARN("attempted to add a command twice ({})", name);
        return;
    }
    cmd_t* cmd = (cmd_t *)Z_Malloc(sizeof(cmd_t), TAG_STATIC, &cmd, "cmd");
    cmd->name = (char *)xmalloc(strlen(name)+1);
    strncpy(cmd->name, name, strlen(name));
    cmd->cmdfunc = func;

    cmd_list[name] = cmd;
}
void Cmd_ExecuteCmd(const char* name)
{
    if (cmd_list.find(name) != cmd_list.end()) {
        cmd_list[name]->cmdfunc();
        cmd_history.emplace_back(name);
    }
    else {
        Con_Printf("Error: command \"%s\" does not exist", name);
    }
}
nomadvector<eastl::string>& Cmd_History(void)
{
    return cmd_history;
}

static thread_local va_list con_argptr;
static thread_local va_list com_argptr;

void Con_Printf(const char *fmt, ...)
{
    va_start(con_argptr, fmt);
    vfprintf(stdout, fmt, con_argptr);
    va_end(con_argptr);
}
void Con_Error(const char *fmt, ...)
{
    fprintf(stderr, "Error: ");
    va_start(con_argptr, fmt);
    vfprintf(stderr, fmt, con_argptr);
    va_end(con_argptr);
}
void Con_Flush(void)
{
    fflush(stdout);
    fflush(stderr);
}
void G_Printf(const char *fmt)
{
    fprintf(stdout, "%s", fmt);
}
void G_Error(const char *fmt)
{
    fprintf(stderr, "%s", fmt);
}
#if 0
void Com_Printf(const char *fmt, ...)
{
    va_start(com_argptr, fmt);
    vfprintf(stdout, fmt, com_argptr);
    va_end(com_argptr);
}
void Com_Error(const char *fmt, ...)
{
    va_start(com_argptr, fmt);
    vfprintf(stderr, fmt, com_argptr);
    va_end(com_argptr);
}
#endif