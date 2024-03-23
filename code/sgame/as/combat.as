#include "convar.as"
#include "game.as"
#include "entity.as"
#include "item.as"

namespace TheNomad::SGame {
	class CombatSoundSystem : TheNomad::GameSystem::GameObject {
		CombatSoundSystem() {
		}
		
		void OnLevelStart() {
			@m_Current = @LevelManager.GetCurrent();
			
			if ( !m_Current.bSupportsDynamicSoundtrack ) {
				ConsolePrint( "Current level doesn't support a dynamic soundtrack" );
				m_bLevelSupport = false;
				m_Current.m_AmbientTrack.SetLooping();
			} else {
				m_bLevelSupport = true;
			}
		}
		void OnLevelEnd() {
		}
		void OnLoad() {
		}
		void OnSave() const {
		}
		void OnRunTic() {
			if ( !m_bLevelSupport ) {
				return; // just let it run
			}
			
			if ( m_nDeltaTime < sgame_MusicChangeDelta.GetInt() ) {
				m_nDeltaTime++;
				return;
			}
			
			m_nDeltaTime = 0;
			
			// TODO: make this a bit more complex, i.e. add intensity scaling
			if ( EntityManager.GetMobCount() > 0 ) {
				m_Current.m_CombatTrack.SetLooping();
			} else {
				m_Current.m_AmbientTrack.SetLooping();
			}
		}
		const string& GetName() const {
			return "CombatSoundSystem";
		}
		
		private float m_nMusicVolume  0.0f;
		private uint m_nDeltaTime = 0;
		private bool m_bLevelSupport = true;
		private const LevelInfoData@ m_Current = null;
	};
	
	CombatSoundSystem@ dynamicMusic;
	
	void InitDynamicMusic() {
		if ( sgame_AdaptiveSoundtrack.GetInt() != 1 ) {
			ConsolePrint( "Dynamic combat soundtrack has been toggled off.\n" );
			return;
		}
		
		@sgame_MusicChangeDelta = @TheNomad::CvarManager.AddCvar( "sgame_MusicChangeDelta", "70", CVAR_LATCH | CVAR_TEMP, false );
		@dynamicMusic = cast<CombatSoundSystem>( TheNomad::GameSystem::AddSystem( CombatSoundSystem() ) );
	}
};