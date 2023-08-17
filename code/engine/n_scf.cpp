#include "n_shared.h"
#include "code/src/g_game.h"

/*
char s[64];
float f;
int32_t i;
qboolean b;
*/
#define NOMAD_CONFIG "default.cfg"

/*
Com_LoadConfig: loads the default configuration file
*/
void Com_LoadConfig(void)
{
    Cbuf_ExecuteText(EXEC_NOW, "exec " NOMAD_CONFIG);
    Cbuf_Execute();
}
