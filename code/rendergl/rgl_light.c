#include "rgl_local.h"

static void R_LightForPoint( const vec3_t origin, const maplight_t *light, vec3_t color )
{
	vec3_t lightPos;
	float distance, diffuse;
	float range, attenuation;
	
	VectorClear( lightPos );
	VectorCopy2( lightPos, light->origin );
	
	distance = DotProduct( origin, lightPos );
	diffuse = 0.0f;
	
	if ( distance <= light->range ) {
		diffuse = 1.0 - fabs( distance / light->range );
	}
	
	diffuse += light->brightness;
	
	color[0] = MIN( diffuse * ( color[0] + light->color[0] ), color[0] );
	color[1] = MIN( diffuse * ( color[1] + light->color[1] ), color[1] );
	color[2] = MIN( diffuse * ( color[2] + light->color[2] ), color[2] );
	
	range = light->range + light->brightness;
	
	attenuation = ( light->constant + light->linear * range + light->quadratic * ( range * range ) );
	
	VectorScale( color, attenuation, color );
}

static void R_DlightForPoint( const vec3_t origin, const dlight_t *dl, vec3_t color )
{
	vec3_t lightPos;
	float distance, diffuse;
	float range, attenuation;
	
	VectorClear( lightPos );
	VectorCopy( lightPos, dl->origin );
	
	distance = DotProduct( origin, lightPos );
	diffuse = 0.0f;
	
	if ( distance <= dl->range ) {
		diffuse = 1.0 - fabs( distance / dl->range );
	}
	
	diffuse += dl->brightness;
	
	color[0] = MIN( diffuse * ( color[0] + dl->color[0] ), color[0] );
	color[1] = MIN( diffuse * ( color[1] + dl->color[1] ), color[1] );
	color[2] = MIN( diffuse * ( color[2] + dl->color[2] ), color[2] );
	
	range = dl->range + dl->brightness;
	
	attenuation = 1.0 / ( range * range );
	
	VectorScale( color, attenuation, color );
}

void R_SetupTileLighting( void )
{
	uint32_t x, y, i;
	vec4_t color;
	const maplight_t *light;
	float distance, diffuse, range;
	float attenuation;
	vec3_t worldPos;
	
//	if ( r_lightingQuality->i > 0 ) {
//		return; // only ever do software baked lighting if we have the lowest lighting quality
//	}
	
	light = rg.world->lights;
	color[3] = 0.0f; // we're never gonna mess with the alpha
	
	for ( y = 0; y < rg.world->height; y++ ) {
		for ( x = 0; x < rg.world->width; x++ ) {
			VectorClear( worldPos );
			VectorSet2( worldPos, x, y );
			
			VectorClear( color );
			color[3] = 1.0f;
			
			for ( i = 0; i < rg.world->numLights; i++ ) {
				R_LightForPoint( worldPos, &light[i], color );
			}
			VectorCopy4( rg.world->tiles[ y * rg.world->width + x ].color, color );
		}
	}
}

void R_LightEntity( renderEntityDef_t *refEntity )
{
	uint32_t i;
	const maplight_t *light;
	vec3_t origin;
	
	if ( r_lightingQuality->i > 0 ) {
		return; // done in the gpu
	}
	
	light = rg.world->lights;
	for ( i = 0; i < rg.world->numLights; i++ ) {
		VectorCopy2( origin, light[i].origin );
		origin[2] = 0.0f;
		if ( disBetweenOBJ( origin, refEntity->e.origin ) > light[i].range ) {
			continue; // don't waste cycles on something that isn't in the light's range
		}
		
		R_LightForPoint( refEntity->e.origin, &light[ i ], refEntity->ambientColor );
		( (byte *)&refEntity->ambientLightInt )[ 0 ] = (int)( refEntity->ambientColor[ 0 ] );
		( (byte *)&refEntity->ambientLightInt )[ 1 ] = (int)( refEntity->ambientColor[ 1 ] );
		( (byte *)&refEntity->ambientLightInt )[ 2 ] = (int)( refEntity->ambientColor[ 2 ] );
		( (byte *)&refEntity->ambientLightInt )[ 3 ] = 0xff;
	}
}

void R_ApplyLighting( const dlight_t *dl, shaderLight_t *gpuLight )
{
	uint64_t i;
	uint32_t y, x;
	vec3_t worldPos, color;
	uvec2_t begin, end;
	renderEntityDef_t *refEntity;
	
	if ( !r_dynamiclight->i ) {
		return; // not enabled
	}
	
	if ( r_lightingQuality->i == 0 ) {
		const float half = dl->range / 2.0f;
		
		begin[0] = ceil( dl->origin[0] ) - half;
		begin[1] = ceil( dl->origin[1] ) - half;
		
		end[0] = floor( dl->origin[0] ) + half;
		end[1] = floor( dl->origin[1] ) + half;
		
		// do all the lighting in software
		for ( i = 0; i < backend.refdef.numEntities; i++ ) {
			refEntity = &backend.refdef.entities[ i ];
			R_DlightForPoint( refEntity->e.origin, dl, refEntity->ambientColor );
		}
		for ( y = begin[1]; y != end[1]; y++ ) {
			for ( x = begin[0]; x != end[0]; x++ ) {
				VectorSet( worldPos, x, y, 0.0f );
				R_DlightForPoint( worldPos, dl, color );

				rg.world->tiles[ y * rg.world->width + x ].color[0] *= color[0];
				rg.world->tiles[ y * rg.world->width + x ].color[1] *= color[1];
				rg.world->tiles[ y * rg.world->width + x ].color[2] *= color[2];
				rg.world->tiles[ y * rg.world->width + x ].color[3] *= color[3];
			}
		}
	} else {
		memset( gpuLight, 0, sizeof( *gpuLight ) );
		VectorCopy( gpuLight->origin, dl->origin );
		VectorCopy( gpuLight->color, dl->color );
		
		gpuLight->brightness = dl->brightness;
		gpuLight->range = dl->range;
		gpuLight->type = dl->ltype;
	}
}