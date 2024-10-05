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
			m_nVolume = nVolume;
		}
		void SetListenerGroup( uint nMask ) {
			m_nListenerMask = nMask;
		}
		void SetPosition( const vec3& in origin, float forward, float up, float velocity ) {
			SetEmitterPosition( origin, forward, up, velocity );
		}

		void PlaySound( const TheNomad::Engine::SoundSystem::SoundEffect& in sfx, float nVolume, uint nListenerMask ) {
			m_nVolume = nVolume;
			m_nListenerMask = nListenerMask;

			PlayEmitterSound( m_hEmitter, m_nVolume, m_nListenerMask, sfx );
		}

		private int m_hEmitter = -1;
		private uint m_nListenerMask = 0xff;
		private float m_nVolume = 0.0f;
	};
};