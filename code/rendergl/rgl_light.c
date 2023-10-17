#include "rgl_local.h"

/*
=================
R_LightForPoint
=================
*/
#if 0
int R_LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir )
{
    renderEntityDef_t ent;
	
	if ( rg.world->lightGridData == NULL )
	    return qfalse;

	memset(&ent, 0, sizeof(ent));
	VectorCopy( point, ent.origin );
	R_SetupEntityLightingGrid( &ent, rg.world );
	VectorCopy(ambientLight, ent.ambientLight);
	VectorCopy(directedLight, ent.directedLight);
	VectorCopy(lightDir, ent.lightDir);

	return qtrue;
}


int R_LightDirForPoint( vec3_t point, vec3_t lightDir, vec3_t normal, world_t *world )
{
    renderEntityDef_t ent;
	
	if ( world->lightGridData == NULL )
	    return qfalse;

	Com_Memset(&ent, 0, sizeof(ent));
	VectorCopy( ent.origin, point );
	R_SetupEntityLightingGrid( &ent, world );

	if (DotProduct(ent.lightDir, normal) > 0.2f)
		VectorCopy(lightDir, ent.lightDir);
	else
		VectorCopy(lightDir, normal);

	return qtrue;
}
#endif