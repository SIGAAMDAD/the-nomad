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
			
			@m_PlayerData[0] = cast<PlayrObject@>( @EntityManager.GetEntityForNum( 0 ) );
			if ( m_nPlayerCount > 1 ) {
				ModifyOrigin( m_PlayerData[0].GetBounds().m_Maxs, pos, vec2( 0.5f, 0.0f ) );
				@m_PlayerData[1] = cast<PlayrObject@>( @EntityManager.Spawn( TheNomad::GameSystem::EntityType::Playr, 0, pos,
					vec2( sgame_PlayerWidth.GetFloat(), sgame_PlayerHeight.GetFloat() ) ) );
			}
			if ( m_nPlayerCount > 2 ) {
				ModifyOrigin( m_PlayerData[1].GetBounds().m_Maxs, pos, vec2( 0.5f, 0.5f ) );
				@m_PlayerData[2] = cast<PlayrObject@>( @EntityManager.Spawn( TheNomad::GameSystem::EntityType::Playr, 0, pos,
					vec2( sgame_PlayerWidth.GetFloat(), sgame_PlayerHeight.GetFloat() ) ) );
			}
			if ( m_nPlayerCount > 3 ) {
				ModifyOrigin( m_PlayerData[2].GetBounds().m_Maxs, pos, vec2( 0.0f, 0.5f ) );
				@m_PlayerData[3] = cast<PlayrObject@>( @EntityManager.Spawn( TheNomad::GameSystem::EntityType::Playr, 0, pos,
					vec2( sgame_PlayerWidth.GetFloat(), sgame_PlayerHeight.GetFloat() ) ) );
			}

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
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveNorth_f ), "+north", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveNorth_f ), "-north", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveSouth_f ), "+south", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveSouth_f ), "-south", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveWest_f ), "+west", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveWest_f ), "-west", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveEast_f ), "+east", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.MoveEast_f ), "-east", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Jump_f ), "+jump", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Jump_f ), "-jump", true );
			
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Crouch_Down_f ), "+crouch", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Crouch_Up_f ), "-crouch", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Dash_Down_f ), "+dash", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Dash_Up_f ), "-dash", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.UseWeapon_Down_f ), "+useweap", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.UseWeapon_Up_f ), "-useweap", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.AltUseWeapon_Down_f ), "+altuseweap", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.AltUseWeapon_Up_f ), "-altuseweap", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Quickshot_Down_f ), "+quickshot", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Quickshot_Up_f ), "-quickshot", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.SwitchWeaponWielding_Down_f ), "+switchwielding", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.DummyFn_Up_f ), "-switchwielding", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.SwitchWeaponMode_Down_f ), "+switchmode", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.DummyFn_Up_f ), "-switchmode", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.SwitchHand_Down_f ), "+switchhand", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.DummyFn_Up_f ), "-switchhand", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.NextWeapon_f ), "+weapnext", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.PrevWeapon_f ), "+weapprev", true );
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
		
		void Dash_Down_f() {
			PlayrObject@ obj = GetPlayerIndex();
			const int time = TheNomad::GameSystem::GameManager.GetGameTic();

			// wait at little bit before launching another dash
			if ( obj.m_nTimeSinceDash - time < 500 ) {
				obj.m_nTimeSinceDash += time;
				return;
			}

			obj.m_nTimeSinceDash = 0;
			obj.m_bDashing = true;
		}
		void Dash_Up_f() {
			PlayrObject@ obj = GetPlayerIndex();
			
			obj.m_bDashing = false;
		}

		void MoveNorth_f() {
			PlayrObject@ obj = @GetPlayerIndex();
			if ( TheNomad::Engine::CmdArgv( 0 )[0] == '+' ) {
				obj.key_MoveNorth.Down();
			} else {
				obj.key_MoveNorth.Up();
			}
		}
		void MoveSouth_f() {
			PlayrObject@ obj = @GetPlayerIndex();
			if ( TheNomad::Engine::CmdArgv( 0 )[0] == '+' ) {
				obj.key_MoveSouth.Down();
			} else {
				obj.key_MoveSouth.Up();
			}
		}
		void MoveEast_f() {
			PlayrObject@ obj = @GetPlayerIndex();
			if ( TheNomad::Engine::CmdArgv( 0 )[0] == '+' ) {
				obj.key_MoveEast.Down();
			} else {
				obj.key_MoveEast.Up();
			}
		}
		void MoveWest_f() {
			PlayrObject@ obj = @GetPlayerIndex();
			if ( TheNomad::Engine::CmdArgv( 0 )[0] == '+' ) {
				obj.key_MoveWest.Down();
			} else {
				obj.key_MoveWest.Up();
			}
		}
		void Jump_f() {
			PlayrObject@ obj = @GetPlayerIndex();
			if ( TheNomad::Engine::CmdArgv( 0 )[0] == '+' ) {
				obj.key_Jump.Down();
			} else {
				obj.key_Jump.Up();
			}
		}

		void UseWeapon_Down_f() { GetPlayerIndex().m_bUseWeapon = true; }
		void UseWeapon_Up_f() { GetPlayerIndex().m_bUseWeapon = false; }
		void AltUseWeapon_Down_f() { GetPlayerIndex().m_bAltUseWeapon = true; }
		void AltUseWeapon_Up_f() { GetPlayerIndex().m_bAltUseWeapon = false; }
		void Quickshot_Down_f() {
			PlayrObject@ obj = GetPlayerIndex();
			
			obj.SetState( @StateManager.GetStateForNum( StateNum::ST_PLAYR_IDLE ) );
			obj.beginQuickshotSfx.Play();
		}
		void Quickshot_Up_f() {
			PlayrObject@ obj = GetPlayerIndex();

			obj.SetState( @StateManager.GetStateForNum( StateNum::ST_PLAYR_IDLE ) );
			obj.endQuickshotSfx.Play();
		}
		
		void DummyFn_Up_f() {
			// we use a dummy function for key ups when we don't want
			// a key to just infinitely repeat
		}
		
		void SwitchWeaponWielding_Down_f() {
			PlayrObject@ obj = GetPlayerIndex();

			obj.weaponFancySfx.Play();
			switch ( obj.m_nHandsUsed ) {
			case 0:
				obj.SwitchWeaponWielding( obj.m_LeftHandMode, obj.m_RightHandMode, @obj.m_LeftHandWeapon, @obj.m_RightHandWeapon );
				break;
			case 1:
			case 2:
				obj.SwitchWeaponWielding( obj.m_RightHandMode, obj.m_LeftHandMode, @obj.m_RightHandWeapon, @obj.m_LeftHandWeapon );
				break;
			};
		}
		void SwitchWeaponMode_Down_f() {
			PlayrObject@ obj = GetPlayerIndex();

			obj.weaponFancySfx.Play();
			switch ( obj.m_nHandsUsed ) {
			case 0:
				obj.SwitchWeaponMode( obj.m_LeftHandMode, @obj.m_LeftHandWeapon );
				break;
			case 1:
				obj.SwitchWeaponMode( obj.m_RightHandMode, @obj.m_RightHandWeapon );
				break;
			case 2: {
				const InfoSystem::WeaponProperty bits = obj.m_LeftHandMode;
				obj.SwitchWeaponMode( obj.m_LeftHandMode, @obj.m_LeftHandWeapon );
				if ( bits == obj.m_LeftHandMode ) {
					obj.SwitchWeaponMode( obj.m_RightHandMode, @obj.m_RightHandWeapon );
				}
				break; }
			default:
				break;
			};
		}
		void SwitchHand_Down_f() {
			PlayrObject@ obj = GetPlayerIndex();
			
			switch ( obj.m_nHandsUsed ) {
			case 0:
				obj.m_nHandsUsed = 1;
				break;
			case 1:
				obj.m_nHandsUsed = 0;
				break;
			case 2:
			default:
				break; // can't switch if we're using both hands for one weapon
			};
		}
		void Crouch_Down_f() {
			PlayrObject@ obj = GetPlayerIndex();
			if ( obj.key_MoveNorth.active || obj.key_MoveSouth.active || obj.key_MoveWest.active || obj.key_MoveEast.active ) {
				obj.beginSlidingSfx.Play();
				obj.SetState( @StateManager.GetStateForNum( StateNum::ST_PLAYR_SLIDING ) );
			} else {
				obj.crouchDownSfx.Play();
				obj.SetState( @StateManager.GetStateForNum( StateNum::ST_PLAYR_CROUCHING ) );
			}
		}
		void Crouch_Up_f() {
			PlayrObject@ obj = GetPlayerIndex();
			
			if ( obj.IsCrouching() ) {
				obj.crouchUpSfx.Play();
			}
			obj.SetState( @StateManager.GetStateForNum( StateNum::ST_PLAYR_IDLE ) );
		}
		
		void Melee_Down_f() {
			PlayrObject@ obj = GetPlayerIndex();
			
			obj.m_nParryBoxWidth = 0.0f;
			obj.SetState( @StateManager.GetStateForNum( StateNum::ST_PLAYR_MELEE ) );
		}
		void Melee_Up_f() {
			// technically this allows animation canceling
			GetPlayerIndex().SetState( @StateManager.GetStateForNum( StateNum::ST_PLAYR_IDLE ) );
		}
		void NextWeapon_f() {
			PlayrObject@ obj = GetPlayerIndex();
			
			obj.m_CurrentWeapon++;
			if ( obj.m_CurrentWeapon >= int( obj.m_WeaponSlots.Count() ) ) {
				obj.m_CurrentWeapon = 0;
			}
			cast<const InfoSystem::WeaponInfo@>( @obj.m_WeaponSlots[ obj.m_CurrentWeapon ].GetInfo() ).equipSfx.Play();
		}
		void PrevWeapon_f() {
			PlayrObject@ obj = GetPlayerIndex();
			
			obj.m_CurrentWeapon--;
			if ( obj.m_CurrentWeapon < 0 ) {
				obj.m_CurrentWeapon = obj.m_WeaponSlots.Count();
			}
			cast<const InfoSystem::WeaponInfo@>( @obj.m_WeaponSlots[ obj.m_CurrentWeapon ].GetInfo() ).equipSfx.Play();
		}
		
		private void RenderScene( const uvec2& in scenePos, const uvec2& in sceneSize, const vec3& in origin ) {
			const uint flags = RSF_ORTHO_TYPE_WORLD;
			
			// snap to the player's position
			// if we're in photomode, ignore
//			if ( TheNomad::Engine::CvarVariableInteger( "r_debugCamera" ) == 0 ) {
			Game_CameraPos.x = origin.x;
			if ( origin.x >= LevelManager.GetMapData().GetWidth() / 2 ) {
				Game_CameraPos.x += 5.25f;
			}
			Game_CameraPos.y = LevelManager.GetMapData().GetHeight() - origin.y;
//			}
			// technically no z coordinate because it's 2D
//			Game_CameraZoom = 0.070f;
			
			TheNomad::Engine::Renderer::ClearScene();
			for ( uint i = 0; i < TheNomad::GameSystem::GameSystems.Count(); i++ ) {
				TheNomad::GameSystem::GameSystems[i].OnRenderScene();
			}
			TheNomad::Engine::Renderer::RenderScene( scenePos.x, scenePos.y, sceneSize.x, sceneSize.y, flags, 
				TheNomad::GameSystem::GameManager.GetGameTic() * 0.1f );
		}

		uint GetPlayerIndex( PlayrObject@ obj ) {
			int index = m_PlayerData.Find( @obj );
			if ( index == -1 ) {
				GameError( "SplitScreen::GetPlayerIndex: bad index" );
			}
			return uint( index );
		}
		
		void Draw() {
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
		}
		
		private PlayrObject@[] m_PlayerData( 4 );
		private int m_nPlayerCount;
	};
	
	SplitScreen ScreenData;
};