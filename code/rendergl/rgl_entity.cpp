#include "rgl_local.h"

extern "C" nhandle_t RE_RegisterEntity(void)
{
}

extern "C" void RE_InitPLRef(void)
{
    rg.entityDefs[0] = (renderEntityRef_t *)ri.Z_Malloc(sizeof(*rg.entityDefs[0]), TAG_RENDERER, &rg.entityDefs, "entityDefPL");
    rg.plRef = rg.entityDefs[0];

    memset(rg.plRef, 0, sizeof(*rg.plRef));
}