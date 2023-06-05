#include "n_shared.h"
#include "n_scf.h"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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
    
    va_start(argptr, fmt);
    vfprintf(stdout, fmt, argptr);
    va_end(argptr);
    fprintf(stdout, "\n");
}

void Con_Printf(const char *fmt, ...)
{
    va_list argptr;

    va_start(argptr, fmt);
    vfprintf(stdout, fmt, argptr);
    va_end(argptr);

    fprintf(stdout, "\n");
    fflush(stdout);
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

// dynamically allocated list of cvars, not really for the static/extern ones
static cvar_t cvar_list = {0};

cvar_t* Cvar_Find(const char *name)
{
    cvar_t* cvar;

    for (cvar = &cvar_list; cvar; cvar = cvar->next) {
        if (N_strncmp(name, cvar->name, sizeof(cvar->name))) {
            return cvar;
        }
    }
    return NULL;
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
    cvar->name[sizeof(cvar->name) - 1] = '\0'; // extra careful

    N_strncpy(cvar->value, value, sizeof(cvar->value));
    cvar->value[sizeof(cvar->value) - 1] = '\0'; // also extra careful

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
    N_strncpy(cvar->value, value, sizeof(cvar->value));
    cvar->value[sizeof(cvar->value)] = '\0';
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

void Cvar_WriteCfg(void)
{
    cvar_t* cvar;

    json data;

    for (cvar = &cvar_list; cvar; cvar = cvar->next) {
        data[cvar->name] = cvar->value;
    }
    for (cvar = G_GetCvars()[0]; cvar; cvar = cvar->next) {
        data[cvar->name] = cvar->value;
    }

    char cfgpath[256];
    stbsp_snprintf(cfgpath, sizeof(cfgpath), "%s%c%s", fs_gamedir.value, PATH_SEP, DEFAULT_CFG_NAME);
    std::ofstream file(cfgpath, std::ios::out);
    if (!file.is_open()) {
        Con_Printf("WARNING: failed to open file %s to write cvar config", cfgpath);
        return;
    }
    file << data;
    file.close();
}