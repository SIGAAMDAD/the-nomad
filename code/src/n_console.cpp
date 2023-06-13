#include "n_shared.h"
#include "m_renderer.h"
#include "n_scf.h"

#define MAX_CMD_LINE 1024
#define MAX_CMD_BUFFER 8192

const char *Cvar_GetValue(const cvar_t* cvar);

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
bool imgui_window_open = false;
static eastl::vector<char> con_buffer;

void Con_ClearBuffer(const char *msg, int length)
{
    if (con_buffer.size() > MAX_BUFFER_SIZE) {
        eastl::vector<char>::iterator ptr = con_buffer.end() - MAX_BUFFER_SIZE;
        while (ptr != con_buffer.end()) {
            if (*ptr == '\n') {
                ptr--;
                break;
            }
            ptr++;
        }
        con_buffer.erase(con_buffer.begin(), ptr);
    }
    con_buffer.insert(con_buffer.end(), msg, msg + length);
    con_buffer.emplace_back('\n');
}

void Con_Printf(loglevel_t level, const char *fmt, ...)
{
    if (level == DEV) {
        fprintf(stdout, "DEV: ");
    }
    else if (level == DEBUG) {
#ifdef _NOMAD_DEBUG
        fprintf(stdout, "DEBUG: ");
#else
        return;
#endif
    }

    va_list argptr;
    int length;
    char msg[MAX_MSG_SIZE];
    memset(msg, 0, sizeof(msg));

    va_start(argptr, fmt);
    length = vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (length >= sizeof(msg)) {
        N_Error("Con_Printf: overflow occured");
    }
    fprintf(stdout, "%s\n", msg);
    Con_ClearBuffer(msg, length);
}

void Con_Printf(const char *fmt, ...)
{
    va_list argptr;
    int length;
    char msg[MAX_MSG_SIZE];
    memset(msg, 0, sizeof(msg));

    va_start(argptr, fmt);
    length = vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (length >= sizeof(msg)) {
        N_Error("Con_Printf: overflow occured");
    }
    fprintf(stdout, "%s\n", msg);
    Con_ClearBuffer(msg, length);
}

static void Con_Clear_f(void)
{
    con_buffer.clear();
    con_buffer.reserve(MAX_BUFFER_SIZE);
}

static void Con_ProcessInput(void)
{
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

void Con_Error(const char *fmt, ...)
{
    va_list argptr;

    fprintf(stderr,"ERROR: ");
    va_start(argptr, fmt);
    vfprintf(stderr, fmt, argptr);
    va_end(argptr);
    fprintf(stderr, "\n");
    fflush(stderr);
}

#define MAX_CVAR_HASH 2048
static cvar_t cvar_list{0};

cvar_t* Cvar_Find(const char *name)
{
    cvar_t *cvar;

    for (cvar = &cvar_list; cvar; cvar = cvar->next) {
        if (N_strneq(cvar->name, name, sizeof(cvar->name))) {
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

void Cvar_RegisterName(const char *name, const char *value, cvartype_t type, qboolean save)
{
    cvar_t* cvar;

    if (Cvar_Find(name)) {
        Con_Printf("Cvar_Register: cvar %s already defined", name);
        return;
    }

    for (cvar = &cvar_list; cvar->next; cvar = cvar->next);
    cvar->next = (cvar_t *)Z_Malloc(sizeof(*cvar), TAG_STATIC, &cvar->next, "cvar");
    cvar = cvar->next;

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

    cvar->type = type;
    cvar->save = save;
}

void Cvar_ChangeValue(const char *name, const char *value)
{
    cvar_t* cvar;

    cvar = Cvar_Find(name);
    if (!cvar) {
        Con_Printf("WARNING: attempting to change the value of non-existent cvar %s", name);
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

void Cvar_Register(cvar_t* cvar)
{
    cvar_t* other;

    if (Cvar_Find(cvar->name)) {
        Con_Printf("Cvar_Register: cvar %s already defined", cvar->name);
        return;
    }

    for (other = &cvar_list; other->next; other = other->next);

    other->next = cvar;
}

#define DEFAULT_CFG_NAME "nomadconfig.scf"

static void Cvar_WriteCfg_f(void)
{
    cvar_t* cvar;

    json data;

//    for (cvar = &cvar_list; cvar; cvar = cvar->next) {
//        data[cvar->name] = Cvar_GetValue(cvar);
//    }
    for (cvar = G_GetCvars()[0]; cvar; cvar = cvar->next) {
        data[cvar->name] = Cvar_GetValue(cvar);
    }

    file_t out = FS_FOpenWrite("default.scf");
    if (out == FS_INVALID_HANDLE) {
        N_Error("FS_FOpenWrite: failed to open write stream for default.scf");
    }
    FS_Write(data.dump().c_str(), data.dump().size() + 1, out);
    FS_FClose(out);
}

static void Cvar_List_f(void)
{
    for (cvar_t *cvar = &cvar_list; cvar; cvar = cvar->next) {
        Con_Printf("%8s = %8s", cvar->name, Cvar_GetValue(cvar));
    }
}

void Con_Init()
{
    con_buffer.reserve(MAX_BUFFER_SIZE);
    Cmd_AddCommand("writecfg", Cvar_WriteCfg_f);
    Cmd_AddCommand("listcvars", Cvar_List_f);
    Cmd_AddCommand("print", Cvar_Print_f);
    Cmd_AddCommand("clear", Con_Clear_f);

    for (uint32_t i = 0; i < G_NumCvars(); i++) {
        Cvar_Register(G_GetCvars()[i]);
    }
}