

#include "n_shared.h"

#define MAX_BFF_FILES 16

static bff_t* bffList[MAX_BFF_FILES];

static cvar_t fs_numArchives;

static char* fs_homepath;
static char fs_gamedir[MAX_GDR_PATH];
static bool fs_initialized;

void FS_Init(void)
{
    fs_homepath = Sys_pwd();

    N_strncpy(fs_gamedir, "gamedata/", sizeof(fs_gamedir));
    nstat_t unused;

    // check if its actually there
    if (Sys_stat(&unused, fs_gamedir) == -1) 
        N_Error("FS_Init: missing required directory %s", fs_gamedir);
    
    int numArchives = N_atoi(fs_numArchives.value);
    if (numArchives >= MAX_BFF_FILES) {
        N_Error("FS_Init: too many bff files, more than %i", MAX_BFF_FILES);
    }

    memset(bffList, 0, sizeof(bffList));
    for (int i = 0; i < numArchives; i++) {
        char path[MAX_OSPATH];
        snprintf(path, sizeof(path), "%s/bff%i.bff", fs_gamedir, i);
        bffList[i] = B_AddModule(path);
        memset(bffList[i], 0, sizeof(bff_t));
    }

    fs_initialized = true;
}

file_t FS_FOpenWrite(const char *filepath)
{
}

file_t FS_FOpenRead(const char *filepath)
{
    
}