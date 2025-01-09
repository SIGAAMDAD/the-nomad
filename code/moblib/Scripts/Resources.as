namespace moblib::Script {
	/*
	class Bark {
		Bark() {
		}
		Bark( const string& in sfx, int count ) {
			m_SfxList.Reserve( count );
			for ( uint i = 0; i < count; i++ ) {
				m_SfxList.Add( TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/" + sfx + "_" + i ) );
			}
		}
		const TheNomad::Engine::SoundSystem::SoundEffect& opImplConv( uint nIndex ) const {
			return m_SfxList[  ];
		}

		private array<TheNomad::Engine::SoundSystem::SoundEffect> m_SfxList;
	};
	*/

	class Resources : TheNomad::GameSystem::GameObject {
		Resources() {
		}

		void OnLevelStart() {
			@ShottyAimState = TheNomad::SGame::StateManager.GetStateById( "mob_merc_shotgunner_aim" );
			@ShottyDieLowState = TheNomad::SGame::StateManager.GetStateById( "mob_merc_shotgunner_die_low" );
			@ShottyDieHighState = TheNomad::SGame::StateManager.GetStateById( "mob_merc_shotgunner_die_high" );
			ShottyAttackSfx = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/mercenary/shotty_attacking" );
			ShottyAimSfx = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/mercenary/shotty_aim" );
			ShottyDieHighSfx = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/pain/die_high" );
			ShottyDieLowSfx = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/pain/die_low" );
			ShottyManDown2Sfx = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/combat/man_down_2_callout_0" );
			ShottyManDown3Sfx = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/combat/man_down_3_callout_0" );
			ShottyScaredSfx = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/combat/scared" );

			for ( uint i = 0; i < ShottyManDownSfx.Count(); i++ ) {
				ShottyManDownSfx[i] = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/combat/man_down_" + i );
			}
			for ( uint i = 0; i < ShottyTargetSpottedSfx.Count(); i++ ) {
				ShottyTargetSpottedSfx[i] = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/patrol/target_spotted_" + i );
			}
			for ( uint i = 0; i < ShottyConfusionSfx.Count(); i++ ) {
				ShottyConfusionSfx[i] = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/patrol/confusion_" + i );
			}
			for ( uint i = 0; i < ShottyTargetEscapedSfx.Count(); i++ ) {
				ShottyTargetEscapedSfx[i] = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/patrol/target_escaped_" + i );
			}
			for ( uint i = 0; i < ShottyTargetRunningSfx.Count(); i++ ) {
				ShottyTargetRunningSfx[i] = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/combat/target_running_" + i );
			}
			for ( uint i = 0; i < ShottyAlertSfx.Count(); i++ ) {
				ShottyAlertSfx[i] = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/patrol/alert_" + i );
			}
			for ( uint i = 0; i < ShottyOutOfTheWay.Count(); i++ ) {
				ShottyOutOfTheWay[i] = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/combat/out_of_the_way_" + i );
			}
			for ( uint i = 0; i < ShottyHelpMe.Count(); i++ ) {
				ShottyHelpMe[i] = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/help_" + i );
			}
			for ( uint i = 0; i < ShottyCeasfire.Count(); i++ ) {
				ShottyCeasfire[i] = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/combat/ceasefire_" + i );
			}
			for ( uint i = 0; i < ShottyPain.Count(); i++ ) {
				ShottyPain[i] = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/pain/in_pain_" + i );
			}
			for ( uint i = 0; i < ShottyDieSfx.Count(); i++ ) {
				ShottyDieSfx[i] = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/pain/dying_" + i );
			}
			for ( uint i = 0; i < ShottyCurse.Count(); i++ ) {
				ShottyCurse[i] = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/barks/combat/curse_" + i );
			}

			@MercGatlingFightMissileState = TheNomad::SGame::StateManager.GetStateById( "mob_merc_gatling_fight" );
			@MercGatlingIdleState = TheNomad::SGame::StateManager.GetStateById( "mob_merc_gatling_idle" );
			@MercGatlingParryState = TheNomad::SGame::StateManager.GetStateById( "mob_merc_gatling_parry" );
			@MercGatlingDieHighState = TheNomad::SGame::StateManager.GetStateById( "mob_merc_gatling_die_high" );
			@MercGatlingDieLowState = TheNomad::SGame::StateManager.GetStateById( "mob_merc_gatling_die_high" );
			MercGatlingAttackSfx = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/mercenary/gatling_attacking" );
			MercGatlingDisappointedSfx = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/mercenary/gatling_disappointed" );
			MercGatlingParrySfx = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/mercenary/gatling_can_parry" );

			GruntScreamSecretSfx = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/grunt_scream_1" );
			GruntScreamSfx = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/grunt_scream_0" );
			GruntExplosionSfx = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/mobs/grunt_explosion" );

			switch ( TheNomad::Engine::CvarVariableInteger( "sgame_Difficulty" ) ) {
			case TheNomad::GameSystem::GameDifficulty::Easy:
				ShottyAimState.SetTics( 2075 );
				break;
			case TheNomad::GameSystem::GameDifficulty::Normal:
				break;
			case TheNomad::GameSystem::GameDifficulty::Hard:
				ShottyAimState.SetTics( 940 );
				break;
			case TheNomad::GameSystem::GameDifficulty::VeryHard:
				ShottyAimState.SetTics( 700 );
				break;
			case TheNomad::GameSystem::GameDifficulty::Insane:	
				ShottyAimState.SetTics( 500 );
				break;
			};
		}
		void OnLevelEnd() {
			@ShottyAimState = null;
		}
		const string& GetName() const {
			return "moblib_ResourceCache";
		}

		void OnInit() {
		}
		void OnShutdown() {
		}
		void OnRunTic() {
			GlobalSquad.Update();
		}
		void OnRenderScene() {
		}
		void OnPlayerDeath( int ) {
			GlobalSquad.SquadBark( ShottyKilledTargetSfx[ TheNomad::Util::PRandom() & ( ShottyKilledTargetSfx.Count() - 1 ) ] );
		}
		void OnCheckpointPassed( uint ) {
		}
		void OnSave() const {
		}
		void OnLoad() {
		}

		TheNomad::SGame::EntityState@ ShottyAimState = null;
		TheNomad::SGame::EntityState@ ShottyDieLowState = null;
		TheNomad::SGame::EntityState@ ShottyDieHighState = null;
		int ShottyScaredSfx = FS_INVALID_HANDLE;
		int ShottyAttackSfx = FS_INVALID_HANDLE;
		int ShottyAimSfx = FS_INVALID_HANDLE;
		int ShottyDieHighSfx = FS_INVALID_HANDLE;
		int ShottyDieLowSfx = FS_INVALID_HANDLE;
		int ShottyManDown2Sfx = FS_INVALID_HANDLE;
		int ShottyManDown3Sfx = FS_INVALID_HANDLE;
		int[] ShottyTargetSpottedSfx( 10 );
		int[] ShottyTargetEscapedSfx( 7 );
		int[] ShottyTargetRunningSfx( 3 );
		int[] ShottyConfusionSfx( 4 );
		int[] ShottyAlertSfx( 3 );
		int[] ShottyOutOfTheWay( 3 );
		int[] ShottyHelpMe( 3 );
		int[] ShottyCeasfire( 4 );
		int[] ShottyDieSfx( 5 );
		int[] ShottyPain( 5 );
		int[] ShottyCurse( 3 );
		int[] ShottyKilledTargetSfx( 3 );
		int[] ShottyManDownSfx( 2 );

		TheNomad::SGame::EntityState@ MercGatlingFightMissileState = null;
		TheNomad::SGame::EntityState@ MercGatlingParryState = null;
		TheNomad::SGame::EntityState@ MercGatlingIdleState = null;
		TheNomad::SGame::EntityState@ MercGatlingDieHighState = null;
		TheNomad::SGame::EntityState@ MercGatlingDieLowState = null;
		int MercGatlingDisappointedSfx = FS_INVALID_HANDLE;
		int MercGatlingAttackSfx = FS_INVALID_HANDLE;
		int MercGatlingParrySfx = FS_INVALID_HANDLE;

		int GruntScreamSecretSfx = FS_INVALID_HANDLE;
		int GruntScreamSfx = FS_INVALID_HANDLE;
		int GruntExplosionSfx = FS_INVALID_HANDLE;
	};

	Resources@ ResourceCache = null;
};