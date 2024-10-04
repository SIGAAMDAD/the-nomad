#include "snd_local.h"
#include "../game/g_world.h"

void CSoundWorld::Init( void )
{
	m_pszEmitters = (emitter_t *)Hunk_Alloc( sizeof( *m_pszEmitters ) * MAX_ENTITIES, h_high );
	m_EmitterList.prev =
	m_EmitterList.next =
		&m_EmitterList;
	
	m_nEmitterCount = 0;
}

void CSoundWorld::Update( void )
{
	emitter_t *em;
	FMOD_3D_ATTRIBUTES attribs;

	memset( &attribs, 0, sizeof( attribs ) );

	for ( em = m_EmitterList.next; em != &m_EmitterList; em = em->next ) {
		if ( em->event == NULL ) {
			continue;
		}

		memcpy( &attribs.position, em->link->origin, sizeof( vec3_t ) );

//		em->event->set3DAttributes( &attribs );
//		em->event->setListenerMask( em->listenerMask );
//		em->event->setVolume( em->volume );
	}
}

void CSoundWorld::PlayEmitterSound( nhandle_t hEmitter, float nVolume, uint32_t nListenerMask, sfxHandle_t hSfx )
{
	emitter_t *em;
	CSoundSource *pSource;
	FMOD_3D_ATTRIBUTES attribs;

	if ( hEmitter >= m_nEmitterCount || hEmitter < 0 ) {
		N_Error( ERR_DROP, "CSoundWorld::PlayEmitterSound: invalid emitter ID %i", hEmitter );
	}

	em = &m_pszEmitters[ hEmitter ];
	if ( em->event ) {
		em->event->release();
	}

	pSource = sndManager->GetSound( hSfx );
	if ( !pSource ) {
		return;
	}
	em->event = pSource->AllocEvent();

	em->listenerMask = nListenerMask;
	em->volume = nVolume;

	memset( &attribs, 0, sizeof( attribs ) );
	memcpy( &attribs.position, em->link->origin, sizeof( vec3_t ) );

//	em->event->set3DAttributes( &attribs );
	//em->event->setListenerMask( nListenerMask );
	em->event->setVolume( snd_effectsVolume->f / 100.0f );
}

void CSoundWorld::SetListenerVolume( nhandle_t hListener, float nVolume )
{
}

void CSoundWorld::SetListenerAudioMask( nhandle_t hListener, uint32_t nMask )
{
}
	
void CSoundWorld::SetEmitterPosition( nhandle_t hEmitter, const vec3_t origin )
{
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
//	if ( ent == g_world->GetEntities() ) {
//		N_Error( ERR_DROP, "CSoundWorld::RegisterEmitter: invalid entityNumber %u", nEntityNumber );
//	}

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