#include "snd_local.h"
#include "../game/g_world.h"

bool CSoundWorld::LoadOcclusionGeometry( void )
{
	union {
		void *v;
		char *b;
	} f;
	uint64_t nLength;
	char path[ MAX_NPATH ];

	COM_StripExtension( Cvar_VariableString( "mapname" ), path, sizeof( path ) );

	nLength = FS_LoadFile( va( "Cache/occlusion/%s.fsb.geom", COM_SkipPath( path ) ), &f.v );
	if ( !nLength || !f.v ) {
		return false;
	}

	Con_Printf( "...Loaded geometry occlusion file\n" );

	CSoundSystem::GetCoreSystem()->loadGeometry( f.v, nLength, &m_pGeomtryBuffer );

	FS_FreeFile( f.v );

	return true;
}

void CSoundWorld::Shutdown( void )
{
	if ( m_pGeomtryBuffer ) {
		m_pGeomtryBuffer->release();
	}
}

channel_t *CSoundWorld::AllocateChannel( CSoundSource *pSource )
{
	uint32_t i;
	FMOD_STUDIO_PLAYBACK_STATE state;
	uint64_t current;
	channel_t *oldest;

	current = Sys_Milliseconds();

	oldest = NULL;
	for ( i = 0; i < snd_maxChannels->i; i++ ) {
		if ( !m_pszChannels[ i ].event ) {
			m_pszChannels[ i ].event = pSource->AllocEvent();
			m_pszChannels[ i ].timestamp = current;
			return &m_pszChannels[ i ];
		}
		
		m_pszChannels[ i ].event->getPlaybackState( &state );
		if ( state != FMOD_STUDIO_PLAYBACK_STOPPED ) {
			if ( !oldest ) {
				oldest = &m_pszChannels[ i ];
			} else if ( current - oldest->timestamp > m_pszChannels[ i ].timestamp ) {
				oldest = &m_pszChannels[ i ];
			}
		} else {
			// we have a free event, take it
			m_pszChannels[ i ].event = pSource->AllocEvent();
			m_pszChannels[ i ].timestamp = current;
			return &m_pszChannels[ i ];
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

void CSoundWorld::Init( void )
{
	const maptile_t *tiles;
	uint32_t y, x;
	int i;
	int numVertices, numPolys, wallCount;
	int polyIndex;
	int savedGeometrySize;
	void *savedGeometry;
	FMOD_VECTOR vertices[ 4 ];
	char path[ MAX_NPATH ];

	m_pszEmitters = (emitter_t *)Hunk_Alloc( sizeof( *m_pszEmitters ) * MAX_ENTITIES, h_high );
	m_EmitterList.prev =
	m_EmitterList.next =
		&m_EmitterList;
	
	m_nEmitterCount = 0;

	m_pszChannels = (channel_t *)Hunk_Alloc( sizeof( *m_pszChannels ) * snd_maxChannels->i, h_high );

	if ( !LoadOcclusionGeometry() ) {
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

		CSoundSystem::GetCoreSystem()->createGeometry( numPolys, numVertices, &m_pGeomtryBuffer );

		for ( y = 0; y < gi.mapCache.info.width; y++ ) {
			for ( x = 0; x < gi.mapCache.info.height; x++ ) {
				if ( IsWall( DIR_NORTH, tiles[ y * gi.mapCache.info.width + x ].flags ) ) {
					memset( vertices, 0, sizeof( vertices ) );
					vertices[0] = { -1.0f,  1.0f, 0.0f };
					vertices[1] = {  1.0f,  0.0f, 0.0f };
					vertices[2] = {  1.0f,  1.0f, 0.0f };
					vertices[3] = { -1.0f, -1.0f, 0.0f };

					ERRCHECK( m_pGeomtryBuffer->addPolygon( 1.0f, 1.0f, IsDoubleSidedWall( DIR_NORTH, tiles[ y * gi.mapCache.info.width + x ].flags ),
						4, vertices, &polyIndex ) );
				}
				if ( IsWall( DIR_EAST, tiles[ y * gi.mapCache.info.width + x ].flags ) ) {
					memset( vertices, 0, sizeof( vertices ) );
					vertices[0] = { -1.0f,  1.0f, 0.0f };
					vertices[1] = {  1.0f,  0.0f, 0.0f };
					vertices[2] = {  1.0f,  1.0f, 0.0f };
					vertices[3] = { -1.0f, -1.0f, 0.0f };

					ERRCHECK( m_pGeomtryBuffer->addPolygon( 1.0f, 1.0f, IsDoubleSidedWall( DIR_EAST, tiles[ y * gi.mapCache.info.width + x ].flags ),
						4, vertices, &polyIndex ) );
				}
				if ( IsWall( DIR_SOUTH, tiles[ y * gi.mapCache.info.width + x ].flags ) ) {
					memset( vertices, 0, sizeof( vertices ) );
					vertices[0] = { -1.0f,  1.0f, 0.0f };
					vertices[1] = {  1.0f,  0.0f, 0.0f };
					vertices[2] = {  1.0f,  1.0f, 0.0f };
					vertices[3] = { -1.0f, -1.0f, 0.0f };

					ERRCHECK( m_pGeomtryBuffer->addPolygon( 1.0f, 1.0f, IsDoubleSidedWall( DIR_SOUTH, tiles[ y * gi.mapCache.info.width + x ].flags ),
						4, vertices, &polyIndex ) );
				}
				if ( IsWall( DIR_WEST, tiles[ y * gi.mapCache.info.width + x ].flags ) ) {
					memset( vertices, 0, sizeof( vertices ) );
					vertices[0] = { -1.0f,  1.0f, 0.0f };
					vertices[1] = {  1.0f,  0.0f, 0.0f };
					vertices[2] = {  1.0f,  1.0f, 0.0f };
					vertices[3] = { -1.0f, -1.0f, 0.0f };

					ERRCHECK( m_pGeomtryBuffer->addPolygon( 1.0f, 1.0f, IsDoubleSidedWall( DIR_WEST, tiles[ y * gi.mapCache.info.width + x ].flags ),
						4, vertices, &polyIndex ) );
				}
			}
		}

		COM_StripExtension( Cvar_VariableString( "mapname" ), path, sizeof( path ) );

		ERRCHECK( m_pGeomtryBuffer->save( NULL, &savedGeometrySize ) );
		savedGeometry = Hunk_AllocateTempMemory( savedGeometrySize );
		ERRCHECK( m_pGeomtryBuffer->save( savedGeometry, &savedGeometrySize ) );
		FS_WriteFile( va( "Cache/occlusion/%s.fsb.geom", COM_SkipPath( path ) ), savedGeometry, savedGeometrySize );
		Hunk_FreeTempMemory( savedGeometry );
	}
}

void CSoundWorld::Update( void )
{
	emitter_t *em;
	int i;
	FMOD_3D_ATTRIBUTES attribs;
	FMOD_STUDIO_PLAYBACK_STATE state;
	FMOD_VECTOR up, forward, vel, pos;

	memset( &attribs, 0, sizeof( attribs ) );

	for ( em = m_EmitterList.next; em != &m_EmitterList; em = em->next ) {
		memcpy( &attribs.position, em->link->origin, sizeof( vec3_t ) );

		if ( em->channel ) {
			em->channel->event->set3DAttributes( &attribs );
			em->channel->event->setListenerMask( em->listenerMask );
			em->channel->event->getPlaybackState( &state );

			if ( state == FMOD_STUDIO_PLAYBACK_STOPPED ) {
				em->channel->event->release();
				em->channel = NULL;
			}
		}
	}

	CSoundSystem::GetCoreSystem()->set3DNumListeners( m_nListenerCount );
	for ( i = 0; i < m_nListenerCount; i++ ) {
		memcpy( &pos, m_szListeners[ i ].link->origin, sizeof( vec3_t ) );
		memset( &up, 0, sizeof( up ) );
		memset( &forward, 0, sizeof( forward ) );
		memset( &vel, 0, sizeof( vel ) );

		CSoundSystem::GetCoreSystem()->set3DListenerAttributes( i, &pos, &vel, &forward, &up );
	}
}

void CSoundWorld::PlayEmitterSound( nhandle_t hEmitter, float nVolume, uint32_t nListenerMask, sfxHandle_t hSfx )
{
	emitter_t *em;
	CSoundSource *pSource;
	FMOD_3D_ATTRIBUTES attribs;
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

	memset( &attribs, 0, sizeof( attribs ) );
	memcpy( &attribs.position, em->link->origin, sizeof( vec3_t ) );

	em->channel->event->set3DAttributes( &attribs );
	em->channel->event->setListenerMask( nListenerMask );
	em->channel->event->setVolume( snd_effectsVolume->f / 100.0f );
	em->channel->event->start();
}

void CSoundWorld::SetListenerVolume( nhandle_t hListener, float nVolume )
{
}

void CSoundWorld::SetListenerAudioMask( nhandle_t hListener, uint32_t nMask )
{
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
	if ( !em->channel ) {
		return;
	}

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
}

void CSoundWorld::SetEmitterVolume( nhandle_t hEmitter, float nVolume )
{
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
		if ( ent->id == nEntityNumber ) {
			break;
		}
	}
	if ( ent == g_world->GetEntities() ) {
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
		N_Error( ERR_DROP, "CSoundWorld::RemoveEmitter: invalid emitterID %i", hEmitter );
	}

	em = &m_pszEmitters[ hEmitter ];
	em->prev->next = em->next;
	em->next->prev = em->prev;
	m_nEmitterCount--;
}