#include "nomadmain/SGame/InfoSystem/InfoDataManager.as"
#include "moblib/MobScript.as"

namespace moblib {
	class AISystem : TheNomad::GameSystem::GameObject {
		AISystem() {
		}
		
		uint GetMobStateOffset( uint nType ) const {
			return uint( TheNomad::SGame::StateNum::ST_MOB_IDLE ) + nType;
		}
		void AllocScript( TheNomad::SGame::MobObject@ mob ) {
			MobScript@ script = cast<MobScript@>( @TheNomad::Util::AllocateExternalScriptClass( "moblib",
				cast<TheNomad::SGame::InfoSystem::MobInfo@>( @mob.GetInfo() ).className ) );
			mob.LinkScript( @script );
			m_ScriptFactory.Add( @script );
		}
		
		const string& GetName() const {
			return "AIManager";
		}
		void OnInit() {
		}
		void OnShutdown() {
		}
		void OnLevelStart() {
			dictionary@ mobs = @TheNomad::SGame::InfoSystem::InfoManager.GetMobInfos();
			const array<TheNomad::SGame::InfoSystem::EntityData>@ types = @TheNomad::SGame::InfoSystem::InfoManager.GetMobTypes();
			
			ConsolePrint( "Linking mob scripts...\n" );

			TheNomad::SGame::EntityManager.ForEachEntity( function( ref@ thisData, TheNomad::SGame::EntityObject@ ent ){
				if ( ent.GetType() != TheNomad::GameSystem::EntityType::Mob ) {
					return;
				}
				
				AISystem@ this = cast<AISystem@>( @thisData );
				
				TheNomad::SGame::MobObject@ mob = cast<TheNomad::SGame::MobObject@>( @ent );
				
				this.AllocScript( @mob );
			}, @this );
		}
		void OnLevelEnd() {
			m_ScriptFactory.Clear();
		}
		void OnSave() const {
		}
		void OnLoad() {
		}
		void OnPlayerDeath( int ) {
		}
		void OnCheckpointPassed( uint ) {
		}
		void OnRunTic() {
		}

		void OnRenderScene() {
		}
		
		private array<MobScript@> m_ScriptFactory;
	};
	
	AISystem@ AIManager = null;
};