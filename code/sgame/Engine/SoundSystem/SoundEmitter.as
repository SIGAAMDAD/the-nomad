#include "SGame/EntityObject.as"

namespace TheNomad::Engine::SoundSystem {
	class SoundEmitter {
		SoundEmitter() {
		}
		SoundEmitter() {
		}
		~SoundEmitter() {
			RemoveEmitter( m_hEmitter );
			m_hEmitter = -1;
		}

		void Register( TheNomad::SGame::EntityObject@ ent ) {
			if ( m_hEmitter != -1 ) {
				return;
			}
			m_hEmitter = RegisterEmitter( ent.GetEntityNum() );
		}

		void SetVolume( float nVolume ) {
			SetEmitterVolume( m_hEmitter, nVolume );
		}
		void SetListenerGroup( uint nMask ) {
			SetEmitterAudioMask( m_hEmitter, nMask );
		}
		void SetPosition( const vec3& in origin, float forward, float up, float velocity ) {
			SetEmitterPosition( m_hEmitter, origin, forward, up, velocity );
		}

		void PlaySound( const TheNomad::Engine::SoundSystem::SoundEffect& in sfx, float nVolume, uint nListenerMask ) {
			PlayEmitterSound( m_hEmitter, nVolume, nListenerMask, sfx );
		}

		private int m_hEmitter = -1;
	};
};