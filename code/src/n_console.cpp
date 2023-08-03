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
    else if (level == DEV && !c_devmode.b)
        return;
    else if (level == DEV && c_devmode.b)
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
    ImGui::SetWindowSize(ImVec2((float)r_screenwidth.i, (float)(r_screenheight.i / 2)));
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


#define MAX_CVAR_HASH 2048
static cvar_t cvar_list;
static uint64_t cvar_count;

const char *Cvar_GetValue(const cvar_t* cvar);

static cvar_t *Cvar_VMToVar(const vmCvar_t *cvar)
{
    cvar_t *cv;
    for (cv = cvar_list.next; cv; cv = cv->next) {
        if (cv->id == cvar->handle)
            return cv;
    }
    N_Error("Cvar_VMToVar: invalid vmCvar handle");
}

static int Cvar_MakeHandle(cvar_t *cvar)
{
    int handle;

    handle = cvar_count;
    cvar->id = handle;
    return handle;
}

void Cvar_Find(vmCvar_t *cvar, const char *name)
{
    cvar_t *cv;

    cv = Cvar_Find(name);
    if (!cv) {
        cvar->handle = CVAR_INVALID_HANDLE;
        return;
    }
    cvar->b = cv->b;
    cvar->i = cv->i;
    cvar->f = cv->f;
    N_strncpyz(cvar->s, cv->s, MAX_CVAR_NAME);
    cvar->handle = Cvar_MakeHandle(cv);

    if (cv->group != CVG_VM)
        cv->group = CVG_VM;
}

cvar_t* Cvar_Find(const char *name)
{
    cvar_t *cvar;
    
    for (cvar = &cvar_list; cvar; cvar = cvar->next) {
        if (!N_stricmpn(cvar->name, name, MAX_CVAR_NAME)) // found it
            return cvar;
    }
    return NULL;
}

static void Cvar_Set(const char *name, const char *value)
{
    cvar_t *cvar;
    int32_t i;
    char s[MAX_CVAR_VALUE];
    float f;
    qboolean b;

    if (!Cvar_Find(name) || strlen(name) >= MAX_CVAR_NAME) {
        Con_Printf("Invalid cvar name string %s", name);
        return;
    }
    if (strlen(value) >= MAX_CVAR_VALUE) {
        Con_Printf("Invalid cvar value string %s", value);
    }

    cvar = Cvar_Find(name);
    f = N_atof(value);
    i = N_atoi(value);
    b = (qboolean)N_strtobool(value);
    cvar->b = b;
    memset(cvar->s, 0, sizeof(cvar->s));
    N_strncpyz(cvar->s, value, sizeof(cvar->s));

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
        N_strncpyz(cvar->s, "null", MAX_CVAR_VALUE);
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

void Cvar_SetGroup(cvar_t *cvar, cvarGroup_t group)
{
    cvar->group = group;
}

const char *Cvar_GetValue(const char *name)
{
    static char str[MAX_CVAR_VALUE];
    cvar_t *cvar;

    memset(str, 0, sizeof(str));
    cvar = Cvar_Find(name);
    if (!cvar) {
        Con_Printf("WARNING: attempted to retrieve cvar value from non-existent cvar");
        return "(null)";
    }

    switch (cvar->type) {
    case TYPE_BOOL:
        N_strncpyz(str, N_booltostr(cvar->b), MAX_CVAR_VALUE);
        break;
    case TYPE_INT:
        snprintf(str, 64, "%i", cvar->i);
        break;
    case TYPE_FLOAT:
        snprintf(str, 64, "%f", cvar->f);
        break;
    case TYPE_STRING:
        N_strncpyz(str, cvar->s, MAX_CVAR_VALUE);
        break;
    };

    return str;
}

const char *Cvar_GetValue(const cvar_t* cvar)
{
    static char str[MAX_CVAR_VALUE];
    memset(str, 0, sizeof(str));
    switch (cvar->type) {
    case TYPE_BOOL:
        N_strncpyz(str, N_booltostr(cvar->b), MAX_CVAR_VALUE);
        break;
    case TYPE_INT:
        snprintf(str, 64, "%i", cvar->i);
        break;
    case TYPE_FLOAT:
        snprintf(str, 64, "%6.03f", cvar->f);
        break;
    case TYPE_STRING:
        N_strncpyz(str, cvar->s, MAX_CVAR_VALUE - 1);
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
    uint64_t hash;

    if (Cvar_Find(name)) {
        return;
    }

    for (cvar = cvar_list.next; cvar->next; cvar = cvar->next);
    
    cvar->next = (cvar_t *)Z_Malloc(sizeof(*cvar), TAG_STATIC, &cvar->next, "cvar");
    cvar = cvar->next;
    
    cvar->flags = flags;
    N_strncpyz(cvar->name, name, MAX_CVAR_NAME);
    cvar->type = type;
    cvar_count++;

    Cvar_ChangeValue(cvar, value);
}

void Cvar_Register(vmCvar_t *cvar, const char *name, const char *value, cvartype_t type)
{
    cvar_t *cv;

    cv = Cvar_Find(name);
    if (cv) { // already exists
        Con_Printf("Cvar_RegisterName: vmCvar '%s' already exists with value '%s'", name, value);
        
        // initialize the vmCvar to the current state
        Cvar_Find(cvar, name);
        return;
    }
    Cvar_SetGroup(Cvar_VMToVar(cvar), CVG_VM);
    Cvar_RegisterName(name, value, type, CVAR_VM_CREATED);
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
        cvar->b = (qboolean)N_strtobool(value);
        break;
    case TYPE_INT:
        cvar->i = N_atoi(value);
        break;
    case TYPE_FLOAT:
        cvar->f = N_atof(value);
        break;
    case TYPE_STRING:
        N_strncpyz(cvar->s, value, MAX_CVAR_VALUE);
        break;
    };
}

void Cvar_ChangeValue(cvar_t *cvar, const char *value)
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
        cvar->b = (qboolean)N_strtobool(value);
        break;
    case TYPE_INT:
        cvar->i = N_atoi(value);
        break;
    case TYPE_FLOAT:
        cvar->f = N_atof(value);
        break;
    case TYPE_STRING:
        N_strncpyz(cvar->s, value, MAX_CVAR_VALUE - 1);
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
    case TYPE_STRING:
        if (N_stricmpn(cvar->s, value, MAX_CVAR_VALUE) == -1)
            N_strncpyz(cvar->s, value, MAX_CVAR_VALUE);
        break;
    };
}

void Cvar_Register(cvar_t* cvar, const char *value)
{
    cvar_t* other;
    uint64_t hash;

    other = Cvar_Find(cvar->name);
    if (other) { // already exists
        Con_Printf("Cvar_Register: cvar %s already exists", cvar->name);
        if (value)
            Cvar_ValueCheck(cvar, value);
        return;
    }

    for (other = &cvar_list; other; other = other->next) {
        if (!other->next)
            break;
    }

    other->next = cvar;
    other = other->next;
    cvar_count++;

    if (value)
        Cvar_ValueCheck(other, value);
}

#define DEFAULT_CFG_NAME "nomadconfig.scf"

static void Cvar_Sort_f(void);

static void Cvar_WriteCfg_f(void)
{
    cvar_t* cvar;

    file_t out = FS_FOpenWrite("default.scf");
    if (out == FS_INVALID_HANDLE) {
        N_Error("FS_FOpenWrite: failed to open write stream for default.scf");
    }
    FS_Write("{\n", 2, out);
    for (cvar = &cvar_list; cvar; cvar = cvar->next) {
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
    Cvar_Sort_f();
    for (cvar_t *cvar = &cvar_list; cvar; cvar = cvar->next) {
        Con_Printf("%s%24 = %s", cvar->name, Cvar_GetValue(cvar));
    }
}

static cvartype_t Cvar_StringToType(const char *str)
{
    if (!N_stricmpn("boolean", str, sizeof("boolean"))) return TYPE_BOOL;
    else if (!N_stricmpn("float", str, sizeof("float"))) return TYPE_FLOAT;
    else if (!N_stricmpn("integer", str, sizeof("integer"))) return TYPE_INT;
    else if (!N_stricmpn("string", str, sizeof("string"))) return TYPE_STRING;

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
            return;
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

static void Cvar_QSortByName( cvar_t **a, uint64_t n )
{
	cvar_t *temp;
	cvar_t *m;
	int32_t i, j;

	i = 0;
	j = n;
	m = a[ n>>1 ];

	do {
		// sort in descending order
		while ( strcmp( a[i]->name, m->name ) > 0 ) i++;
		while ( strcmp( a[j]->name, m->name ) < 0 ) j--;

		if ( i <= j ) {
			temp = a[i]; 
			a[i] = a[j]; 
			a[j] = temp;
			i++; 
			j--;
		}
	} while ( i <= j );

	if ( j > 0 ) Cvar_QSortByName( a, j );
	if ( n > i ) Cvar_QSortByName( a+i, n-i );
}


static void Cvar_Sort_f( void ) 
{
	cvar_t *list[ MAX_CVAR_HASH ], *var;
	uint64_t count;
	uint64_t i;

	for ( count = 0, var = cvar_list.next; var; var = var->next ) {
		if ( var->name ) {
			list[ count++ ] = var;
		} else {
			N_Error( "%s: NULL cvar name", __func__ );
		}
	}

	if ( count < 2 ) {
		return; // nothing to sort
	}

	Cvar_QSortByName( &list[0], count-1 );
	
	cvar_list.next = NULL;

	// relink cvars
	for ( i = 0; i < count; i++ ) {
		var = list[ i ];
		// link the variable in
		var->next = cvar_list.next;
		if ( cvar_list.next )
			cvar_list.prev = var;
		var->prev = NULL;
		cvar_list.next = var;
	}
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

static qboolean historyLoaded = qfalse;

#define COMMAND_HISTORY 32

static field_t historyEditLines[COMMAND_HISTORY];


void Con_Shutdown(void)
{
    con_buffer.clear();
}

void Cvar_Init(void)
{
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

void Con_Init(void)
{
#if 0
    std::array<spdlog::sink_ptr, 2> sinks;

    sinks[0] = std::make_shared<spdlog::sinks::basic_file_sink_mt>("gamedata/logfile.txt", true);
    sinks[1] = std::make_shared<spdlog::sinks::stdout_sink_mt>();

    sinks[0]->set_pattern("%l: %v");
    sinks[1]->set_pattern("%^%l: %v%$");

    spdlog::set_pattern("%^%l: %v%$");

    conLogger = std::make_shared<spdlog::logger>("conlog", begin(sinks), end(sinks));

    spdlog::register_logger(conLogger);
    conLogger->set_level(spdlog::level::trace);
    conLogger->flush_on(spdlog::level::trace);

    spdlog::set_default_logger(conLogger);
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
    Cmd_AddCommand("sortvars", Cvar_Sort_f);
}