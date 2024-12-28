#include "SGame/PlayerSystem/PMoveData.as"
#include "Engine/CommandSystem/CommandSystem.as"

namespace TheNomad::SGame {
	//
	// SplitScreen
	// maximum of 4 players
	// renders an individual scene per player
	//
	class SplitScreen {
		SplitScreen() {
		}
		
		private void ModifyOrigin( const vec3& in bounds, vec3& out origin, const vec2& in offset ) {
			// add just a little bit of an offset so that we aren't clipping into each other
			origin.x += ( bounds.x / 2.0f ) + offset.x;
			origin.y += ( bounds.y / 2.0f ) + offset.y;
		}

		private void CacheSfx() {
			m_SlowMoOff = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/slowmo_off" );
			m_SlowMoOn = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/slowmo_on" );

			m_DieSfx0 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/death1" );
			m_DieSfx1 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/death2" );
			m_DieSfx2 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/death3" );

			m_PainSfx0 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/pain_scream_0" );
			m_PainSfx1 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/pain_scream_1" );
			m_PainSfx2 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/pain_scream_2" );

			m_SlideSfx0 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/slide_0" );
			m_SlideSfx1 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/slide_1" );

			m_DashSfx0 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/use_jumpkit_0" );
			m_DashSfx1 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/use_jumpkit_1" );

			m_MeleeSfx = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/melee" );

			m_WeaponChangeHandSfx = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/weapon_change_hand" );
			m_WeaponChangeModeSfx = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/weapon_change_mode" );

			m_CrouchDownSfx = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/cloth_foley_0" );
			m_CrouchUpSfx = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/player/cloth_foley_1" );
		}

		void InitPlayers() {
			ConsolePrint( "Initializing screen data...\n" );
			/*

			m_nPlayerCount = int( TheNomad::Engine::CvarVariableInteger( "in_numInputDevices" ) );
			vec3 pos = EntityManager.GetEntityForNum( 0 ).GetOrigin(); // the first entity will always be the player

			if ( m_nPlayerCount > 4 || m_nPlayerCount < 0 ) {
				GameError( "SplitScreen::Init: in_numInputDevices is greater than 4 or less than 0" );
			}

			m_ScreenSize = uvec2( TheNomad::GameSystem::GPUConfig.screenWidth, TheNomad::GameSystem::GPUConfig.screenHeight );
			
			@m_PlayerData = cast<PlayrObject@>( @EntityManager.GetEntityForNum( 0 ) );

			for ( uint i = 0; i < m_nPlayerCount; i++ ) {
				m_PlayerData[i].SetPlayerIndex( i );
			}
			*/

			m_ScreenSize = uvec2( TheNomad::GameSystem::GPUConfig.screenWidth, TheNomad::GameSystem::GPUConfig.screenHeight );

			@m_PlayerData = cast<PlayrObject@>( @EntityManager.GetEntityForNum( 0 ) );
			m_PlayerData.SetPlayerIndex( 0 );

			CacheSfx();
		}
		void Init() {
			//
			// add keybind commands
			//
			
			// these specific movement commands MUST NOT CHANGE as they are hardcoded into the engine
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveNorth_Down_f ), "+north" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveNorth_Up_f ), "-north" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveSouth_Down_f ), "+south" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveSouth_Up_f ), "-south" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveWest_Down_f ), "+west" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveWest_Up_f ), "-west" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveEast_Down_f ), "+east" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveEast_Up_f ), "-east" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Jump_Down_f ), "+jump" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Jump_Up_f ), "-jump" );
			
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Crouch_Down_f ), "+crouch" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Crouch_Up_f ), "-crouch" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Slide_Down_f ), "+slide" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Slide_Up_f ), "-slide" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Dash_Down_f ), "+dash" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Dash_Up_f ), "-dash" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Melee_Down_f ), "+melee" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Melee_Up_f ), "-melee" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.UseWeapon_Down_f ), "+useweap" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.UseWeapon_Down_f ), "-useweap" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.AltUseWeapon_Down_f ), "+altuseweap" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.AltUseWeapon_Up_f ), "-altuseweap" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Quickshot_f ), "quickshot" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.SwitchWeaponWielding_f ), "switchwielding" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.SwitchWeaponMode_f ), "switchmode" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.SwitchHand_f ), "switchhand" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.NextWeapon_f ), "weapnext" );
			TheNomad::Engine::CommandSystem::CmdManager.AddKeyBind(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.PrevWeapon_f ), "weapprev" );
		}
		void Shutdown() {
		}

		/*
		private PlayrObject@ GetPlayerIndex() const {
			if ( TheNomad::Engine::CmdArgc() == 4 ) {
				// its a co-op input, get the player index
				const int index = Convert().ToInt( TheNomad::Engine::CmdArgv( 3 ) );
				if ( index > 4 || index < 0 ) {
					GameError( "SplitScreen: invalid player index " + index );
				}
				
				return @m_PlayerData[ index ];
			}
			return @m_PlayerData[ 0 ];
		}
		*/

		void MoveNorth_Down_f() {
			m_PlayerData.key_MoveNorth.Down();
		}
		void MoveNorth_Up_f() {
			m_PlayerData.key_MoveNorth.Up();
		}
		void MoveSouth_Down_f() {
			m_PlayerData.key_MoveSouth.Down();
		}
		void MoveSouth_Up_f() {
			m_PlayerData.key_MoveSouth.Up();
		}
		void MoveEast_Down_f() {
			m_PlayerData.key_MoveEast.Down();
		}
		void MoveEast_Up_f() {
			m_PlayerData.key_MoveEast.Up();
		}
		void MoveWest_Down_f() {
			m_PlayerData.key_MoveWest.Down();
		}
		void MoveWest_Up_f() {
			m_PlayerData.key_MoveWest.Up();
		}
		void Jump_Down_f() {
			m_PlayerData.key_Jump.Down();
		}
		void Jump_Up_f() {
			m_PlayerData.key_Jump.Up();
		}
		void UseWeapon_Down_f() {
			m_PlayerData.Flags |= PF_USING_WEAPON;
		}
		void UseWeapon_Up_f() {
			m_PlayerData.Flags &= ~PF_USING_WEAPON;
		}
		void AltUseWeapon_Down_f() {
			m_PlayerData.Flags |= PF_USING_WEAPON_ALT;
		}
		void AltUseWeapon_Up_f() {
			m_PlayerData.Flags &= ~PF_USING_WEAPON_ALT;
		}
		void Quickshot_f() {
			if ( m_PlayerData.InReflex() ) {
				m_PlayerData.SetReflexMode( false );
				m_SlowMoOff.Play();
				TheNomad::GameSystem::TIMESTEP = 1.0f / 60.0f;
			} else {
				m_PlayerData.SetReflexMode( true );
				m_SlowMoOn.Play();
				TheNomad::GameSystem::TIMESTEP = 1.0f / 800.0f;
			}
		}
		void Dash_Down_f() {
			// wait at little bit before launching another dash
			if ( ( TheNomad::GameSystem::GameTic - m_PlayerData.DashEndTime ) * TheNomad::GameSystem::DeltaTic < DASH_DURATION ) {
				return;
			}

			if ( ( Util::PRandom() & 1 ) == 1 ) {
				m_PlayerData.EmitSound( m_DashSfx0, 10.0f, 0xff );
			} else {
				m_PlayerData.EmitSound( m_DashSfx1, 10.0f, 0xff );
			}

			m_PlayerData.DashEndTime = TheNomad::GameSystem::GameTic;
			m_PlayerData.Flags |= PF_DASHING;
			
			vec3 origin = m_PlayerData.GetOrigin();
			origin.y -= 1.5f;
			switch ( m_PlayerData.GetFacing() ) {
			case FACING_LEFT:
				origin.x += 1.5f;
				break;
			case FACING_RIGHT:
				origin.x -= 1.5f;
				break;
			};

			GfxManager.AddDustTrail( origin, m_PlayerData.GetFacing() );
			Util::HapticRumble( 0, 0.80f, 400 );
		}
		void Dash_Up_f() {
		}

		void Slide_Down_f() {
			// TODO: ground slam?
			if ( ( m_PlayerData.Flags & PF_CROUCHING ) != 0 || m_PlayerData.GetOrigin().z > 0.0f
				|| ( TheNomad::GameSystem::GameTic - m_PlayerData.SlideEndTime ) * TheNomad::GameSystem::DeltaTic < SLIDE_DURATION  )
			{
				return;
			}

			// we need a little bit of momentum to engage in a slide
			if ( ( m_PlayerData.key_MoveNorth.active || m_PlayerData.key_MoveSouth.active || m_PlayerData.key_MoveEast.active
				|| m_PlayerData.key_MoveWest.active ) || ( m_PlayerData.Flags & PF_DASHING ) != 0 )
			{
				if ( ( Util::PRandom() & 1 ) == 1 ) {
					m_PlayerData.EmitSound( m_SlideSfx0, 10.0f, 0xff );
				} else {
					m_PlayerData.EmitSound( m_SlideSfx1, 10.0f, 0xff );
				}
				
				m_PlayerData.SlideEndTime = TheNomad::GameSystem::GameTic;
				m_PlayerData.Flags |= PF_SLIDING;

				vec3 origin = m_PlayerData.GetOrigin();
				origin.y -= 1.5f;
				switch ( m_PlayerData.GetFacing() ) {
				case FACING_LEFT:
					origin.x += 1.5f;
					break;
				case FACING_RIGHT:
					origin.x -= 1.5f;
					break;
				};

				GfxManager.AddDustTrail( origin, m_PlayerData.GetFacing() );
				Util::HapticRumble( 0, 0.40f, 200 );
			}
		}
		void Slide_Up_f() {
			m_PlayerData.Flags &= ~PF_SLIDING;
		}
		void Melee_Down_f() {	
			if ( @m_PlayerData.GetLeftArmState() is @StateManager.GetStateForNum( StateNum::ST_PLAYR_ARMS_MELEE ) ) {
				return;
			}
			m_PlayerData.EmitSound( m_MeleeSfx, 10.0f, 0xff );
			m_PlayerData.SetParryBoxWidth( 0.0f );
			m_PlayerData.SetArmsState( StateNum::ST_PLAYR_ARMS_MELEE );
		}
		void Melee_Up_f() {
		}
		
		void SwitchWeaponWielding_f() {
			m_PlayerData.SwitchWeaponWielding();
		}
		void SwitchWeaponMode_f() {
			m_PlayerData.EmitSound( m_WeaponChangeModeSfx, 10.0f, 0xff );
			m_PlayerData.SwitchWeaponMode();
		}
		void SwitchHand_f() {
			m_PlayerData.EmitSound( m_WeaponChangeHandSfx, 10.0f, 0xff );
			m_PlayerData.SwitchUsedHand();
		}
		void Crouch_Down_f() {
			if ( m_PlayerData.IsCrouching() ) {
				return;
			}
			m_PlayerData.EmitSound( m_CrouchDownSfx, 10.0f, 0xff );
			m_PlayerData.SetCrouching( true );
		}
		void Crouch_Up_f() {
			if ( !m_PlayerData.IsCrouching() ) {
				return;
			}
			m_PlayerData.EmitSound( m_CrouchUpSfx, 10.0f, 0xff );
			m_PlayerData.SetCrouching( false );
			m_PlayerData.SetState( @StateManager.GetStateForNum( StateNum::ST_PLAYR_IDLE ) );
		}

		void NextWeapon_f() {
			m_PlayerData.SetCurrentWeapon( m_PlayerData.GetCurrentWeaponIndex() + 1 );
			if ( m_PlayerData.GetCurrentWeaponIndex() >= NUM_WEAPON_SLOTS ) {
				m_PlayerData.SetCurrentWeapon( 0 );
			}
			m_PlayerData.EmitSound(
				cast<const InfoSystem::WeaponInfo@>( @m_PlayerData.GetCurrentWeapon().GetInfo() ).equipSfx,
				10.0f,
				0xff
			);
		}
		void PrevWeapon_f() {
			m_PlayerData.SetCurrentWeapon( NUM_WEAPON_SLOTS - 1 );
			if ( m_PlayerData.GetCurrentWeaponIndex() < 0 ) {
				m_PlayerData.SetCurrentWeapon( NUM_WEAPON_SLOTS - 1 );
			}
			m_PlayerData.EmitSound(
				cast<const InfoSystem::WeaponInfo@>( @m_PlayerData.GetCurrentWeapon().GetInfo() ).equipSfx,
				10.0f,
				0xff
			);
		}

		private void RenderScene( const uvec2& in scenePos, const uvec2& in sceneSize, const vec3& in origin ) {
			if ( @LevelManager.GetMapData() is null ) {
				return;
			}

			// technically no z coordinate because it's 2D
			Game_CameraZoom = TheNomad::Engine::CvarVariableInteger( "sgame_CameraZoom" ) * 0.001f;

			// snap to the player's position
			// if we're in photomode, ignore
			Game_CameraWorldPos.x = ( origin.x - 0.812913357f ) * 10.0f;
			Game_CameraWorldPos.y = ( LevelManager.GetMapData().GetHeight() - ( origin.y + 0.812913357f ) ) * 10.0f;
			Game_PlayerPos = origin;

			TheNomad::Engine::Renderer::ClearScene();
			for ( uint i = 0; i < TheNomad::GameSystem::GameSystems.Count(); ++i ) {
				TheNomad::GameSystem::GameSystems[i].OnRenderScene();
			}
			TheNomad::Engine::Renderer::RenderScene( scenePos.x, scenePos.y, sceneSize.x, sceneSize.y, RSF_ORTHO_TYPE_WORLD, 
				TheNomad::GameSystem::GameTic );
		}

		/*
		uint GetPlayerIndex( PlayrObject@ obj ) {
			int index = m_PlayerData.Find( @obj );
			if ( index == -1 ) {
				GameError( "SplitScreen::GetPlayerIndex: bad index" );
			}
			return uint( index );
		}
		*/
		
		void Draw() {
			RenderScene( m_ScreenPosition, m_ScreenSize, m_PlayerData.GetOrigin() );
			/*
			uvec2 pos, size;
			
			pos = uvec2( 0, 0 );
			size = uvec2( TheNomad::GameSystem::GameManager.GetGPUConfig().screenWidth,
				TheNomad::GameSystem::GameManager.GetGPUConfig().screenHeight );

			switch ( m_nPlayerCount ) {
			case 0:
			case 1:
				RenderScene( pos, size, m_PlayerData.GetOrigin() );
				break;
			case 2:
				size.x /= 2;
				RenderScene( pos, size, m_PlayerData.GetOrigin() );
				
				pos.x += size.x;
				RenderScene( pos, size, m_PlayerData[1].GetOrigin() );
				break;
			case 3:
				size.x /= 2;
				size.y /= 2;
				RenderScene( pos, size, m_PlayerData.GetOrigin() );
				
				pos.x += size.x;
				RenderScene( pos, size, m_PlayerData[1].GetOrigin() );
				
				pos.x = 0;
				pos.y += size.y;
				size.x *= 2;
				RenderScene( pos, size, m_PlayerData[2].GetOrigin() );
				break;
			case 4:
				size.x /= 2;
				size.y /= 2;
				RenderScene( pos, size, m_PlayerData.GetOrigin() );
				
				pos.x += size.x;
				RenderScene( pos, size, m_PlayerData[1].GetOrigin() );
				
				pos.x = 0;
				pos.y += size.y;
				RenderScene( pos, size, m_PlayerData[2].GetOrigin() );
				
				pos.x += size.x;
				RenderScene( pos, size, m_PlayerData[3].GetOrigin() );
				break;
			default:
				GameError( "SplitScreen::Init: in_numInputDevices is greater than 4 or less than 0" );
			};
			*/
		}

		/*
		uint GetPlayerCount() const {
			return m_nPlayerCount;
		}
		PlayrObject@ GetPlayerAt( uint nIndex ) {
			return @m_PlayerData[ nIndex ];
		}
		const PlayrObject@ GetPlayerAt( uint nIndex ) const {
			return @m_PlayerData[ nIndex ];
		}
		*/
		const PlayrObject@ GetPlayer() const {
			return @m_PlayerData;
		}
		PlayrObject@ GetPlayer() {
			return @m_PlayerData;
		}

		private uvec2 m_ScreenPosition = uvec2( 0, 0 );
		private uvec2 m_ScreenSize = uvec2( 0, 0 );
		
		private PlayrObject@ m_PlayerData = null;
//		private PlayrObject@[] m_PlayerData( 4 );
//		private uint m_nPlayerCount = 0;

		private TheNomad::Engine::SoundSystem::SoundEffect m_CrouchDownSfx;
		private TheNomad::Engine::SoundSystem::SoundEffect m_CrouchUpSfx;
		private TheNomad::Engine::SoundSystem::SoundEffect m_DashSfx0;
		private TheNomad::Engine::SoundSystem::SoundEffect m_DashSfx1;
		private TheNomad::Engine::SoundSystem::SoundEffect m_SlideSfx0;
		private TheNomad::Engine::SoundSystem::SoundEffect m_SlideSfx1;
		private TheNomad::Engine::SoundSystem::SoundEffect m_SlowMoOn;
		TheNomad::Engine::SoundSystem::SoundEffect m_SlowMoOff;
		TheNomad::Engine::SoundSystem::SoundEffect m_DieSfx0;
		TheNomad::Engine::SoundSystem::SoundEffect m_DieSfx1;
		TheNomad::Engine::SoundSystem::SoundEffect m_DieSfx2;
		TheNomad::Engine::SoundSystem::SoundEffect m_PainSfx0;
		TheNomad::Engine::SoundSystem::SoundEffect m_PainSfx1;
		TheNomad::Engine::SoundSystem::SoundEffect m_PainSfx2;
		TheNomad::Engine::SoundSystem::SoundEffect m_ParrySfx;
		private TheNomad::Engine::SoundSystem::SoundEffect m_WeaponChangeHandSfx;
		private TheNomad::Engine::SoundSystem::SoundEffect m_WeaponChangeModeSfx;
		private TheNomad::Engine::SoundSystem::SoundEffect m_MeleeSfx;
	};
	
	SplitScreen ScreenData;
};