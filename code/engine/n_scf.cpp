#include "n_shared.h"
#include "code/src/g_game.h"

/*
char s[64];
float f;
int32_t i;
qboolean b;
*/

static void SCF_ParseFile(void)
{
    int parm = I_GetParm("-config");
    if (parm != -1) {

    }
}

#define NOMAD_CONFIG "default.cfg"

/*
Com_LoadConfig: loads the default configuration file
*/
void Com_LoadConfig(void)
{
    Cmd_ExecuteText("exec " NOMAD_CONFIG);
}
