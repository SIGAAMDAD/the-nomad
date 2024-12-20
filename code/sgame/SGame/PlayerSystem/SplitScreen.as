#include "SGame/PlayerSystem/PMoveData.as"

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

		void InitPlayers() {
			ConsolePrint( "Initializing screen data...\n" );

			m_nPlayerCount = int( TheNomad::Engine::CvarVariableInteger( "in_numInputDevices" ) );
			vec3 pos = EntityManager.GetEntityForNum( 0 ).GetOrigin(); // the first entity will always be the player

			if ( m_nPlayerCount > 4 || m_nPlayerCount < 0 ) {
				GameError( "SplitScreen::Init: in_numInputDevices is greater than 4 or less than 0" );
			}

			m_ScreenSize = uvec2( TheNomad::GameSystem::GameManager.GetGPUConfig().screenWidth,
				TheNomad::GameSystem::GameManager.GetGPUConfig().screenHeight );
			
			@m_PlayerData[0] = cast<PlayrObject@>( @EntityManager.GetEntityForNum( 0 ) );

			for ( uint i = 0; i < m_nPlayerCount; i++ ) {
				m_PlayerData[i].SetPlayerIndex( i );
			}
		}
		void Init() {
			//
			// add keybind commands
			//
			
			// these specific movement commands MUST NOT CHANGE as they are hardcoded into the engine
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveNorth_Down_f ), "+north", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveNorth_Up_f ), "-north", true );
			
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveSouth_Down_f ), "+south", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveSouth_Up_f ), "-south", true );
			
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveWest_Down_f ), "+west", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveWest_Up_f ), "-west", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveEast_Down_f ), "+east", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveEast_Up_f ), "-east", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Jump_Down_f ), "+jump", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Jump_Up_f ), "-jump", true );
			
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Crouch_Down_f ), "+crouch", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Crouch_Up_f ), "-crouch", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Slide_Down_f ), "+slide", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Slide_Up_f ), "-slide", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Dash_Down_f ), "+dash", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Dash_Up_f ), "-dash", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Melee_Down_f ), "+melee", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Melee_Up_f ), "-melee", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.UseWeapon_Down_f ), "+useweap", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.UseWeapon_Down_f ), "-useweap", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.AltUseWeapon_Down_f ), "+altuseweap", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.AltUseWeapon_Up_f ), "-altuseweap", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Quickshot_Down_f ), "+quickshot", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Quickshot_Up_f ), "-quickshot", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.SwitchWeaponWielding_f ), "switchwielding", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.SwitchWeaponMode_f ), "switchmode", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.SwitchHand_f ), "switchhand", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.NextWeapon_f ), "weapnext", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.PrevWeapon_f ), "weapprev", true );
		}
		void Shutdown() {
		}
		
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

		void MoveNorth_Down_f() {
			m_PlayerData[0].key_MoveNorth.Down();
		}
		void MoveNorth_Up_f() {
			m_PlayerData[0].key_MoveNorth.Up();
		}
		void MoveSouth_Down_f() {
			m_PlayerData[0].key_MoveSouth.Down();
		}
		void MoveSouth_Up_f() {
			m_PlayerData[0].key_MoveSouth.Up();
		}
		void MoveEast_Down_f() {
			m_PlayerData[0].key_MoveEast.Down();
		}
		void MoveEast_Up_f() {
			m_PlayerData[0].key_MoveEast.Up();
		}
		void MoveWest_Down_f() {
			m_PlayerData[0].key_MoveWest.Down();
		}
		void MoveWest_Up_f() {
			m_PlayerData[0].key_MoveWest.Up();
		}
		void Jump_Down_f() {
			m_PlayerData[0].key_Jump.Down();
		}
		void Jump_Up_f() {
			m_PlayerData[0].key_Jump.Up();
		}

		void UseWeapon_Down_f() {
			m_PlayerData[0].SetUsingWeapon( true );
		}
		void UseWeapon_Up_f() {
			m_PlayerData[0].SetUsingWeapon( false );
		}
		void AltUseWeapon_Down_f() {
			m_PlayerData[0].SetUsingWeaponAlt( true );
		}
		void AltUseWeapon_Up_f() {
			m_PlayerData[0].SetUsingWeaponAlt( false );
		}
		void Quickshot_Down_f() {
		}
		void Quickshot_Up_f() {
		}
		void Dash_Down_f() {
			PlayrObject@ obj = @m_PlayerData[0];

			// wait at little bit before launching another dash
			if ( obj.GetTimeSinceLastDash() < 1000 ) {
				return;
			}

			obj.EmitSound( obj.dashSfx, 10.0f, 0xff );
			//Util::HapticRumble( obj.GetPlayerIndex(), 0.40f, 700 );
			obj.ResetDash();
			obj.SetDashing( true );
				
			vec3 origin = obj.GetOrigin();
			origin.y -= 1.5f;
			switch ( obj.GetFacing() ) {
			case FACING_LEFT:
				origin.x += 1.5f;
				break;
			case FACING_RIGHT:
				origin.x -= 1.5f;
				break;
			};

			GfxManager.AddDustTrail( origin, obj.GetFacing() );
			Util::HapticRumble( 0, 0.80f, 400 );
		}
		void Dash_Up_f() {
			m_PlayerData[0].SetDashing( false );
		}

		void Slide_Down_f() {
			PlayrObject@ obj = @m_PlayerData[0];

			// TODO: ground slam?
			if ( obj.IsCrouching() || obj.GetOrigin().z > 0.0f || obj.GetTimeSinceLastSlide() < SLIDE_DURATION ) {
				return;
			}

			// we need a little bit of momentum to engage in a slide
			if ( ( obj.key_MoveNorth.active || obj.key_MoveSouth.active || obj.key_MoveEast.active || obj.key_MoveWest.active ) ||
				obj.IsDashing() )
			{
				if ( ( Util::PRandom() & 1 ) == 1 ) {
					obj.EmitSound( obj.slideSfx0, 10.0f, 0xff );
				} else {
					obj.EmitSound( obj.slideSfx1, 10.0f, 0xff );
				}
				
				//Util::HapticRumble( obj.GetPlayerIndex(), 0.40f, 500 );
				obj.ResetSlide();
				obj.SetSliding( true );

				vec3 origin = obj.GetOrigin();
				origin.y -= 1.5f;
				switch ( obj.GetFacing() ) {
				case FACING_LEFT:
					origin.x += 1.5f;
					break;
				case FACING_RIGHT:
					origin.x -= 1.5f;
					break;
				};

				GfxManager.AddDustTrail( origin, obj.GetFacing() );
				Util::HapticRumble( 0, 0.40f, 200 );
			}
		}
		void Slide_Up_f() {
			m_PlayerData[0].SetSliding( false );
		}
		void Melee_Down_f() {
			PlayrObject@ obj = @m_PlayerData[0];
	
			if ( @obj.GetLeftArmState() is @StateManager.GetStateForNum( StateNum::ST_PLAYR_ARMS_MELEE ) ) {
				return;
			}
			obj.EmitSound( obj.meleeSfx, 10.0f, 0xff );
			obj.SetParryBoxWidth( 0.0f );
			obj.SetArmsState( StateNum::ST_PLAYR_ARMS_MELEE );
		}
		void Melee_Up_f() {
		}
		
		void SwitchWeaponWielding_f() {
			PlayrObject@ obj = @m_PlayerData[0];

			switch ( obj.GetHandsUsed() ) {
			case 0:
				obj.SwitchWeaponWielding( @obj.GetLeftArm(), @obj.GetRightArm(),
					@obj.GetLeftHandWeapon(), @obj.GetRightHandWeapon() );
				break;
			case 1:
			case 2:
				obj.SwitchWeaponWielding( @obj.GetRightArm(), @obj.GetLeftArm(),
					@obj.GetRightHandWeapon(), @obj.GetLeftHandWeapon() );
				break;
			};
		}
		void SwitchWeaponMode_f() {
			PlayrObject@ obj = @m_PlayerData[0];

			obj.EmitSound( obj.weaponChangeModeSfx, 10.0f, 0xff );
			switch ( obj.GetHandsUsed() ) {
			case 0:
				obj.SwitchWeaponMode( @obj.GetLeftArm(), @obj.GetLeftHandWeapon() );
				break;
			case 1:
				obj.SwitchWeaponMode( @obj.GetRightArm(), @obj.GetRightHandWeapon() );
				break;
			case 2: {
				const InfoSystem::WeaponProperty bits = obj.GetLeftHandMode();
				obj.SwitchWeaponMode( @obj.GetLeftArm(), @obj.GetLeftHandWeapon() );
				if ( bits == obj.GetLeftHandMode() ) {
					obj.SwitchWeaponMode( @obj.GetRightArm(), @obj.GetRightHandWeapon() );
				}
				break; }
			default:
				break;
			};
		}
		void SwitchHand_f() {
			PlayrObject@ obj = @m_PlayerData[0];
			
			obj.EmitSound( obj.weaponChangeHandSfx, 10.0f, 0xff );
			switch ( obj.GetHandsUsed() ) {
			case 0:
				obj.SetHandsUsed( 1 );
				break;
			case 1:
				obj.SetHandsUsed( 0 );
				break;
			case 2:
			default:
				break; // can't switch if we're using both hands for one weapon
			};
		}
		void Crouch_Down_f() {
			PlayrObject@ obj = @m_PlayerData[0];

			if ( obj.IsCrouching() ) {
				return;
			}

			obj.EmitSound( obj.crouchDownSfx, 10.0f, 0xff );
			obj.SetCrouching( true );
		}
		void Crouch_Up_f() {
			PlayrObject@ obj = @m_PlayerData[0];

			if ( !obj.IsCrouching() ) {
				return;
			}
			obj.EmitSound( obj.crouchUpSfx, 10.0f, 0xff );
			obj.SetCrouching( false );
			obj.SetState( @StateManager.GetStateForNum( StateNum::ST_PLAYR_IDLE ) );
		}

		void NextWeapon_f() {
			PlayrObject@ obj = GetPlayerIndex();
			
			obj.SetCurrentWeapon( obj.GetCurrentWeaponIndex() + 1 );
			if ( obj.GetCurrentWeaponIndex() >= int( obj.m_WeaponSlots.Count() ) ) {
				obj.SetCurrentWeapon( 0 );
			}
			obj.EmitSound(
				cast<const InfoSystem::WeaponInfo@>( @obj.m_WeaponSlots[ obj.GetCurrentWeaponIndex() ].GetData().GetInfo() ).equipSfx,
				10.0f,
				0xff
			);
		}
		void PrevWeapon_f() {
			PlayrObject@ obj = GetPlayerIndex();
			
			obj.SetCurrentWeapon( obj.GetCurrentWeaponIndex() - 1 );
			if ( obj.GetCurrentWeaponIndex() < 0 ) {
				obj.SetCurrentWeapon( obj.m_WeaponSlots.Count() - 1 );
			}
			obj.EmitSound(
				cast<const InfoSystem::WeaponInfo@>( @obj.m_WeaponSlots[ obj.GetCurrentWeaponIndex() ].GetData().GetInfo() ).equipSfx,
				10.0f,
				0xff
			);
		}

		private void RenderScene( const uvec2& in scenePos, const uvec2& in sceneSize, const vec3& in origin ) {
			const uint flags = RSF_ORTHO_TYPE_WORLD;

			if ( @LevelManager.GetMapData() is null ) {
				return;
			}

			// technically no z coordinate because it's 2D
			Game_CameraZoom = TheNomad::Engine::CvarVariableInteger( "sgame_CameraZoom" ) * 0.001f;

			// snap to the player's position
			// if we're in photomode, ignore
			Game_CameraWorldPos.x = ( origin.x - 0.812913357f ) * 10.0f;
			Game_CameraWorldPos.y = ( LevelManager.GetMapData().GetHeight() - ( origin.y + 0.812913357f ) ) * 10.0f;
//			Game_CameraWorldPos.x = ( origin.x - log( Game_CameraZoom ) ) * 10.0f;
//			Game_CameraWorldPos.y = ( LevelManager.GetMapData().GetHeight() - ( origin.y + log( Game_CameraZoom ) ) ) * 10.0f;
			Game_PlayerPos = origin;

			TheNomad::Engine::Renderer::ClearScene();
			for ( uint i = 0; i < TheNomad::GameSystem::GameSystems.Count(); i++ ) {
				TheNomad::GameSystem::GameSystems[i].OnRenderScene();
			}
			TheNomad::Engine::Renderer::RenderScene( scenePos.x, scenePos.y, sceneSize.x, sceneSize.y, flags, 
				TheNomad::GameSystem::GameTic );
			
			m_bCommandReset = true;
		}

		uint GetPlayerIndex( PlayrObject@ obj ) {
			int index = m_PlayerData.Find( @obj );
			if ( index == -1 ) {
				GameError( "SplitScreen::GetPlayerIndex: bad index" );
			}
			return uint( index );
		}
		
		void Draw() {
			RenderScene( m_ScreenPosition, m_ScreenSize, m_PlayerData[0].GetOrigin() );
			/*
			uvec2 pos, size;
			
			pos = uvec2( 0, 0 );
			size = uvec2( TheNomad::GameSystem::GameManager.GetGPUConfig().screenWidth,
				TheNomad::GameSystem::GameManager.GetGPUConfig().screenHeight );

			switch ( m_nPlayerCount ) {
			case 0:
			case 1:
				RenderScene( pos, size, m_PlayerData[0].GetOrigin() );
				break;
			case 2:
				size.x /= 2;
				RenderScene( pos, size, m_PlayerData[0].GetOrigin() );
				
				pos.x += size.x;
				RenderScene( pos, size, m_PlayerData[1].GetOrigin() );
				break;
			case 3:
				size.x /= 2;
				size.y /= 2;
				RenderScene( pos, size, m_PlayerData[0].GetOrigin() );
				
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
				RenderScene( pos, size, m_PlayerData[0].GetOrigin() );
				
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

		uint GetPlayerCount() const {
			return m_nPlayerCount;
		}
		PlayrObject@ GetPlayerAt( uint nIndex ) {
			return @m_PlayerData[ nIndex ];
		}
		const PlayrObject@ GetPlayerAt( uint nIndex ) const {
			return @m_PlayerData[ nIndex ];
		}

		private uvec2 m_ScreenPosition = uvec2( 0, 0 );
		private uvec2 m_ScreenSize = uvec2( 0, 0 );
		
		private bool m_bCommandReset = true;
		private string m_CommandBuffer;

		private PlayrObject@[] m_PlayerData( 4 );
		private uint m_nPlayerCount = 0;
		private uint m_nLastJumpTime = 0;
	};
	
	SplitScreen ScreenData;
};