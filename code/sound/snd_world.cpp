#include "snd_local.h"
#include "../game/g_world.h"

void CSoundWorld::ListEmitters_f( void )
{
	const emitter_t *em;
	int i;

	Con_Printf( "\nEMITTERS:\n" );
	for ( i = 0; i < s_SoundWorld->m_nEmitterCount; i++ ) {
		em = &s_SoundWorld->m_pszEmitters[i];

		Con_Printf( "%-4u: %i (id) %i (type) 0x%x (mask) %f %f %f (origin)\n",
			em->link->entityNumber, em->link->id, em->link->type, em->listenerMask,
			em->link->origin[0], em->link->origin[1], em->link->origin[2] );
	}
}

void CSoundWorld::ListListeners_f( void )
{
	const listener_t *l;
	int i;

	Con_Printf( "\nLISTENERS:\n" );
	for ( i = 0; i < s_SoundWorld->m_nListenerCount; i++ ) {
		l = &s_SoundWorld->m_szListeners[i];

		Con_Printf( "%i: 0x%x (mask) %f %f %f (origin) %u (index)\n",
			i, l->listenerMask, l->link->origin[0], l->link->origin[1], l->link->origin[2], l->link->entityNumber );
	}
}

bool CSoundWorld::LoadOcclusionGeometry( void )
{
	union {
		void *v;
		char *b;
	} f;
	uint64_t nLength;
	char path[ MAX_NPATH ];

	COM_StripExtension( Cvar_VariableString( "mapname" ), path, sizeof( path ) );

	nLength = FS_LoadFile( va( "Cache/occlusion/%sfsb.geom", COM_SkipPath( path ) ), &f.v );
	if ( !nLength || !f.v ) {
		return false;
	}

	Con_Printf( "...Loaded geometry occlusion file\n" );

	CSoundSystem::GetCoreSystem()->loadGeometry( f.v, nLength, &m_pGeometryBuffer );

	FS_FreeFile( f.v );

	return true;
}

/*
* CSoundWorld::AllocateChannel: finds the first free or oldest channel and
* returns that to be used
*/
channel_t *CSoundWorld::AllocateChannel( CSoundSource *pSource )
{
	int32_t i;
	FMOD_STUDIO_PLAYBACK_STATE state;
	uint64_t current;
	channel_t *oldest;

	current = Sys_Milliseconds();

	oldest = NULL;
	for ( i = 0; i < snd_maxChannels->i; i++ ) {
		if ( !m_szChannels[ i ].event ) {
			m_szChannels[ i ].event = pSource->AllocEvent();
			m_szChannels[ i ].timestamp = current;
			return &m_szChannels[ i ];
		}
		
		m_szChannels[ i ].event->getPlaybackState( &state );
		if ( state != FMOD_STUDIO_PLAYBACK_STOPPED ) {
			if ( !oldest ) {
				oldest = &m_szChannels[ i ];
			} else if ( current - oldest->timestamp > m_szChannels[ i ].timestamp ) {
				oldest = &m_szChannels[ i ];
			}
		} else {
			// we have a free event, take it
			m_szChannels[ i ].event = pSource->AllocEvent();
			m_szChannels[ i ].timestamp = current;
			return &m_szChannels[ i ];
		}
	}

	if ( oldest != NULL ) {
		oldest->event->release();
		oldest->timestamp = current;
	}

	oldest->event = pSource->AllocEvent();
	return oldest;
}

void CSoundWorld::ReleaseChannel( channel_t *pChannel )
{
	if ( pChannel->event ) {
		pChannel->event->release();
		pChannel->timestamp = 0;
	}
}

void CSoundWorld::Shutdown( void )
{
	if ( m_pGeometryBuffer ) {
		m_pGeometryBuffer->release();
	}
	memset( m_szChannels, 0, sizeof( m_szChannels ) );

	if ( m_pszEmitters ) {
		Mem_Free( m_pszEmitters );
	}

	m_pGeometryBuffer = NULL;
	m_pszEmitters = NULL;
	m_nListenerCount = 0;
	m_nEmitterCount = 0;

	Cmd_RemoveCommand( "snd.show_listeners" );
	Cmd_RemoveCommand( "snd.show_emitters" );
}

void CSoundWorld::AllocateGeometry( void )
{
	/*
	const maptile_t *tiles;
	uint32_t y, x;
	int i;
	int numVertices, numPolys, wallCount;
	int polyIndex;
	int savedGeometrySize;
	void *savedGeometry;
	FMOD_VECTOR vertices[ 4 ];
	char path[ MAX_NPATH ];
	int versionMajor, versionMinor;
	IPLContextSettings settings;
	IPLSimulationSettings simulation;
	IPLDistanceAttenuationModel distanceModel;

	distanceModel.minDistance = 0.25f;
	distanceModel.type = IPL_DISTANCEATTENUATIONTYPE_DEFAULT;
	distanceModel.callback = IPL_DistanceCallback;

	settings.allocateCallback = IPL_Alloc;
	settings.freeCallback = IPL_Free;

	if ( SDL_HasSSE41() || SDL_HasSSE42() ) {
		settings.simdLevel = IPL_SIMDLEVEL_SSE4;
	} else if ( SDL_HasSSE2() ) {
		settings.simdLevel = IPL_SIMDLEVEL_SSE2;
	} else if ( SDL_HasAVX2() ) {
		settings.simdLevel = IPL_SIMDLEVEL_AVX2;
	} else if ( SDL_HasAVX512F() ) {
		settings.simdLevel = IPL_SIMDLEVEL_AVX512;
	} else if ( SDL_HasAVX() ) {
		settings.simdLevel = IPL_SIMDLEVEL_AVX;
	} else if ( SDL_HasNEON() ) {
		settings.simdLevel = IPL_SIMDLEVEL_NEON;
	}

	settings.flags = IPL_CONTEXTFLAGS_VALIDATION;

	iplContextCreate( &settings, &m_SteamAudio );
	*/

	/*
	if ( LoadOcclusionGeometry() ) {
		return;
	}

	wallCount = 0;
	numVertices = 0;
	numPolys = 0;

	tiles = gi.mapCache.info.tiles;
		
	for ( y = 0; y < gi.mapCache.info.width; y++ ) {
		for ( x = 0; x < gi.mapCache.info.height; x++ ) {
			for ( i = 0; i < NUMDIRS; i++ ) {
				if ( IsWall( i, tiles[ y * gi.mapCache.info.width + x ].flags ) ) {
					wallCount++;
					numPolys++;
					numVertices += NUMDIRS * 2;
				}
			}
		}
	}

	CSoundSystem::GetCoreSystem()->createGeometry( numPolys, numVertices, &m_pGeometryBuffer );

	for ( y = 0; y < gi.mapCache.info.width; y++ ) {
		for ( x = 0; x < gi.mapCache.info.height; x++ ) {
			if ( IsWall( DIR_NORTH, tiles[ y * gi.mapCache.info.width + x ].flags ) ) {
				vertices[0] = {  0.5f,  0.5f, 0.0f };
				vertices[1] = {  0.5f, -0.5f, 0.0f };
				vertices[2] = { -0.5f, -0.5f, 0.0f };
				vertices[3] = { -0.5f,  0.5f, 0.0f };

				ERRCHECK( m_pGeometryBuffer->addPolygon( 1.0f, 1.0f, IsDoubleSidedWall( DIR_NORTH, tiles[ y * gi.mapCache.info.width + x ].flags ),
					4, vertices, &polyIndex ) );
			}
			if ( IsWall( DIR_EAST, tiles[ y * gi.mapCache.info.width + x ].flags ) ) {
				memset( vertices, 0, sizeof( vertices ) );
				vertices[0] = { -1.0f,  1.0f, 0.0f };
				vertices[1] = {  1.0f,  0.0f, 0.0f };
				vertices[2] = {  1.0f,  1.0f, 0.0f };
				vertices[3] = { -1.0f, -1.0f, 0.0f };

				ERRCHECK( m_pGeometryBuffer->addPolygon( 1.0f, 1.0f, IsDoubleSidedWall( DIR_EAST, tiles[ y * gi.mapCache.info.width + x ].flags ),
					4, vertices, &polyIndex ) );
			}
			if ( IsWall( DIR_SOUTH, tiles[ y * gi.mapCache.info.width + x ].flags ) ) {
				memset( vertices, 0, sizeof( vertices ) );
				vertices[0] = { -1.0f,  1.0f, 0.0f };
				vertices[1] = {  1.0f,  0.0f, 0.0f };
				vertices[2] = {  1.0f,  1.0f, 0.0f };
				vertices[3] = { -1.0f, -1.0f, 0.0f };

				ERRCHECK( m_pGeometryBuffer->addPolygon( 1.0f, 1.0f, IsDoubleSidedWall( DIR_SOUTH, tiles[ y * gi.mapCache.info.width + x ].flags ),
					4, vertices, &polyIndex ) );
			}
			if ( IsWall( DIR_WEST, tiles[ y * gi.mapCache.info.width + x ].flags ) ) {
				memset( vertices, 0, sizeof( vertices ) );
				vertices[0] = { -1.0f,  1.0f, 0.0f };
				vertices[1] = {  1.0f,  0.0f, 0.0f };
				vertices[2] = {  1.0f,  1.0f, 0.0f };
				vertices[3] = { -1.0f, -1.0f, 0.0f };

				ERRCHECK( m_pGeometryBuffer->addPolygon( 1.0f, 1.0f, IsDoubleSidedWall( DIR_WEST, tiles[ y * gi.mapCache.info.width + x ].flags ),
					4, vertices, &polyIndex ) );
			}
		}
	}

	COM_StripExtension( Cvar_VariableString( "mapname" ), path, sizeof( path ) );

	ERRCHECK( m_pGeometryBuffer->save( NULL, &savedGeometrySize ) );
	savedGeometry = Hunk_AllocateTempMemory( savedGeometrySize );
	ERRCHECK( m_pGeometryBuffer->save( savedGeometry, &savedGeometrySize ) );
	FS_WriteFile( va( "Cache/occlusion/%sfsb.geom", COM_SkipPath( path ) ), savedGeometry, savedGeometrySize );
	Hunk_FreeTempMemory( savedGeometry );
	*/
}

void CSoundWorld::Init( void )
{
	m_pszEmitters = (emitter_t *)Mem_ClearedAlloc( sizeof( *m_pszEmitters ) * MAX_ENTITIES );
	m_EmitterList.prev =
	m_EmitterList.next =
		&m_EmitterList;
	
	m_nEmitterCount = 0;

	memset( m_szChannels, 0, sizeof( m_szChannels ) );

	Cmd_AddCommand( "snd.show_listeners", CSoundWorld::ListListeners_f );
	Cmd_AddCommand( "snd.show_emitters", CSoundWorld::ListEmitters_f );
}

void CSoundWorld::Update( void )
{
	emitter_t *em;
	int i;
	float volume;
	FMOD_3D_ATTRIBUTES attribs;
	FMOD_STUDIO_PLAYBACK_STATE state;
	FMOD_VECTOR up, forward, vel, pos;
	qboolean setVolume;

	memset( &attribs, 0, sizeof( attribs ) );

	for ( em = m_EmitterList.next; em != &m_EmitterList; em = em->next ) {
		if ( !em->channel ) {
			continue;
		}

		ERRCHECK( em->channel->event->setListenerMask( em->listenerMask ) );
		ERRCHECK( em->channel->event->getPlaybackState( &state ) );

		volume = DotProduct( em->link->origin, m_szListeners[0].link->origin ) / 100.0f;
		em->channel->event->setVolume( em->volume + volume );

		if ( state == FMOD_STUDIO_PLAYBACK_STOPPED ) {
			em->channel->event->release();
			em->channel = NULL;
		}
	}

	/*
	CSoundSystem::GetCoreSystem()->set3DNumListeners( m_nListenerCount );
	for ( i = 0; i < m_nListenerCount; i++ ) {
		memcpy( &pos, m_szListeners[ i ].link->origin, sizeof( vec3_t ) );
		memset( &up, 0, sizeof( up ) );
		memset( &forward, 0, sizeof( forward ) );
		memset( &vel, 0, sizeof( vel ) );

		CSoundSystem::GetCoreSystem()->set3DListenerAttributes( i, &pos, &vel, &forward, &up );
	}
	*/
}

void CSoundWorld::PlayEmitterSound( nhandle_t hEmitter, float nVolume, uint32_t nListenerMask, sfxHandle_t hSfx )
{
	emitter_t *em;
	CSoundSource *pSource;
	FMOD_STUDIO_PLAYBACK_STATE state;

	if ( hEmitter >= m_nEmitterCount || hEmitter < 0 ) {
		N_Error( ERR_DROP, "CSoundWorld::PlayEmitterSound: invalid emitter ID %i", hEmitter );
	}

	em = &m_pszEmitters[ hEmitter ];

	pSource = sndManager->GetSound( hSfx );
	if ( !pSource ) {
		Con_DPrintf( "CSoundWorld::PlayEmitterSound: no sound for handle %i\n", hSfx );
		return;
	}

	em->channel = AllocateChannel( pSource );
	em->listenerMask = nListenerMask;
	em->volume = nVolume * ( snd_effectsVolume->f / 100.0f );

	em->channel->event->setListenerMask( nListenerMask );
	em->channel->event->setVolume( em->volume );
	em->channel->event->start();
}

void CSoundWorld::SetListenerVolume( nhandle_t hListener, float nVolume )
{
	listener_t *listener;

	if ( hListener >= m_nListenerCount || hListener < 0 ) {
		N_Error( ERR_DROP, "CSoundWorld::SetListenerVolume: invalid listener ID %i", hListener );
	}

	listener = &m_szListeners[ hListener ];

	listener->volume = nVolume;
}

void CSoundWorld::SetListenerAudioMask( nhandle_t hListener, uint32_t nMask )
{
	listener_t *listener;

	if ( hListener >= m_nListenerCount || hListener < 0 ) {
		N_Error( ERR_DROP, "CSoundWorld::SetListenerAudioMask: invalid listener ID %i", hListener );
	}

	listener = &m_szListeners[ hListener ];

	listener->listenerMask = nMask;
}

void CSoundWorld::SetEmitterPosition( nhandle_t hEmitter, const vec3_t origin, float forward, float up, float speed )
{
	emitter_t *em;
	FMOD_3D_ATTRIBUTES attribs;
	vec3_t vec;

	if ( hEmitter >= m_nEmitterCount || hEmitter < 0 ) {
		N_Error( ERR_DROP, "CSoundWorld::SetEmitterPosition: invalid emitter ID %i", hEmitter );
	}

	em = &m_pszEmitters[ hEmitter ];
	VectorCopy( em->link->origin, origin );

	if ( !em->channel ) {
		return;
	}
	return;

	memset( &attribs, 0, sizeof( attribs ) );
	memcpy( &attribs.position, em->link->origin, sizeof( vec3_t ) );
	attribs.forward.x = attribs.forward.z = forward;
	VectorNormalize( (vec_t *)&attribs.forward );

	attribs.up.x = attribs.up.y = attribs.up.z = up;
	VectorNormalize( (vec_t *)&attribs.up );

	if ( !VectorCompare( (vec_t *)&attribs.up, vec3_origin ) ) {
		PerpendicularVector( vec, (vec_t *)&attribs.up );
		memcpy( &attribs.up, vec, sizeof( vec ) );
	}

	attribs.velocity.x = attribs.velocity.y = attribs.velocity.z = speed;

	em->channel->event->set3DAttributes( &attribs );
}

void CSoundWorld::SetEmitterAudioMask( nhandle_t hEmitter, uint32_t nMask )
{
	emitter_t *em;

	if ( hEmitter >= m_nEmitterCount || hEmitter < 0 ) {
		N_Error( ERR_DROP, "CSoundWorld::SetEmitterAudioMask: invalid emitter ID %i", hEmitter );
	}

	em = &m_pszEmitters[ hEmitter ];
	em->listenerMask = nMask;

	if ( em->channel ) {
		em->channel->event->setListenerMask( nMask );
	}
}

void CSoundWorld::SetEmitterVolume( nhandle_t hEmitter, float nVolume )
{
	emitter_t *em;

	if ( hEmitter >= m_nEmitterCount || hEmitter < 0 ) {
		N_Error( ERR_DROP, "CSoundWorld::SetEmitterVolume: invalid emitter ID %i", hEmitter );
	}

	em = &m_pszEmitters[ hEmitter ];
	em->volume = nVolume;

	if ( em->channel ) {
//		em->channel->event->setVolume( nVolume );
	}
}
	
nhandle_t CSoundWorld::PushListener( uint32_t nEntityNumber )
{
	nhandle_t hListener;
	listener_t *l;
	linkEntity_t *ent;

	if ( nEntityNumber >= MAX_ENTITIES ) {
		N_Error( ERR_DROP, "CSoundWorld::PushListener: invalid entityNumber %u", nEntityNumber );
	}
	if ( m_nListenerCount >= arraylen( m_szListeners ) ) {
		N_Error( ERR_DROP, "CSoundWorld::PushListener: too many listeners" );
	}

	for ( ent = g_world->GetEntities()->next; ent != g_world->GetEntities(); ent = ent->next ) {
		if ( ent->entityNumber == nEntityNumber ) {
			break;
		}
	}
	if ( ent == g_world->GetEntities() && g_world->NumEntities() > 1 ) {
		N_Error( ERR_DROP, "CSoundWorld::PushListener: invalid entityNumber %u", nEntityNumber );
	}

	hListener = m_nListenerCount;
	l = &m_szListeners[ hListener ];
	memset( l, 0, sizeof( *l ) );
	m_nListenerCount++;

	l->link = ent;
	l->listenerMask = 0xff;

	return hListener;
}

nhandle_t CSoundWorld::RegisterEmitter( uint32_t nEntityNumber )
{
	emitter_t *em;
	nhandle_t hEmitter;
	linkEntity_t *ent;

	if ( nEntityNumber >= MAX_ENTITIES ) {
		N_Error( ERR_DROP, "CSoundWorld::RegisterEmitter: invalid entityNumber %u", nEntityNumber );
	}
	if ( m_nEmitterCount >= MAX_ENTITIES ) {
		N_Error( ERR_DROP, "CSoundWorld::RegisterEmitter: too many emitters" );
	}

	for ( ent = g_world->GetEntities()->next; ent != g_world->GetEntities(); ent = ent->next ) {
		if ( ent->entityNumber == nEntityNumber ) {
			break;
		}
	}
	if ( ent == g_world->GetEntities() && g_world->NumEntities() > 1 ) {
		N_Error( ERR_DROP, "CSoundWorld::RegisterEmitter: invalid entityNumber %u", nEntityNumber );
	}

	hEmitter = m_nEmitterCount;
	em = &m_pszEmitters[ m_nEmitterCount ];
	memset( em, 0, sizeof( *em ) );
	m_nEmitterCount++;

	em->link = ent;

	m_EmitterList.prev->next = em;
	em->prev = m_EmitterList.prev;
	em->next = &m_EmitterList;

	return hEmitter;
}

void CSoundWorld::RemoveEmitter( nhandle_t hEmitter )
{
	emitter_t *em;

	if ( hEmitter >= m_nEmitterCount || hEmitter < 0 ) {
		Con_Printf( COLOR_RED "ERROR: CSoundWorld::RemoveEmitter: invalid emitterID %i\n", hEmitter );
		return;
	}

	em = &m_pszEmitters[ hEmitter ];
	if ( em->channel ) {
		em->channel->event->release();
	}
	em->prev->next = em->next;
	em->next->prev = em->prev;
	m_nEmitterCount--;
}