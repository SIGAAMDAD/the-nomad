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
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Melee_Down_f ), "+melee", true );
			TheNomad::Engine::CommandSystem::CmdManager.AddCommand(
				TheNomad::Engine::CommandSystem::CommandFunc( @this.Melee_Up_f ), "-melee", true );
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

		void MoveNorth_Down_f() { GetPlayerIndex().key_MoveNorth.Down(); }
		void MoveNorth_Up_f() { GetPlayerIndex().key_MoveNorth.Up(); }
		void MoveEast_Down_f() { GetPlayerIndex().key_MoveEast.Down(); }
		void MoveEast_Up_f() { GetPlayerIndex().key_MoveEast.Up(); }

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

		void UseWeapon_Down_f() { GetPlayerIndex().SetUsingWeapon( true ); }
		void UseWeapon_Up_f() { GetPlayerIndex().SetUsingWeapon( false ); }
		void AltUseWeapon_Down_f() { GetPlayerIndex().SetUsingWeaponAlt( true ); }
		void AltUseWeapon_Up_f() { GetPlayerIndex().SetUsingWeaponAlt( false ); }
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

			switch ( obj.GetHandsUsed() ) {
			case 0:
				obj.SwitchWeaponWielding( obj.GetLeftHandMode(), obj.GetRightHandMode(),
					@obj.GetLeftHandWeapon(), @obj.GetRightHandWeapon() );
				break;
			case 1:
			case 2:
				obj.SwitchWeaponWielding( obj.GetRightHandMode(), obj.GetLeftHandMode(),
					@obj.GetRightHandWeapon(), @obj.GetLeftHandWeapon() );
				break;
			};
		}
		void SwitchWeaponMode_Down_f() {
			PlayrObject@ obj = GetPlayerIndex();

			obj.weaponChangeModeSfx.Play();
			switch ( obj.GetHandsUsed() ) {
			case 0:
				obj.SwitchWeaponMode( obj.GetLeftHandMode(), @obj.GetLeftHandWeapon() );
				break;
			case 1:
				obj.SwitchWeaponMode( obj.GetRightHandMode(), @obj.GetRightHandWeapon() );
				break;
			case 2: {
				const InfoSystem::WeaponProperty bits = obj.GetLeftHandMode();
				obj.SwitchWeaponMode( obj.GetLeftHandMode(), @obj.GetLeftHandWeapon() );
				if ( bits == obj.GetLeftHandMode() ) {
					obj.SwitchWeaponMode( obj.GetRightHandMode(), @obj.GetRightHandWeapon() );
				}
				break; }
			default:
				break;
			};
		}
		void SwitchHand_Down_f() {
			PlayrObject@ obj = GetPlayerIndex();
			
			obj.weaponChangeHandSfx.Play();
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
		void Dash_Down_f() {
			PlayrObject@ obj = GetPlayerIndex();

			// wait at little bit before launching another dash
			if ( obj.GetTimeSinceLastDash() < 1000 ) {
				return;
			}

			obj.dashSfx.Play();
			Util::HapticRumble( obj.GetPlayerIndex(), 0.40f, 700 );
			obj.ResetDash();
			obj.SetDashing( true );
		}
		void Dash_Up_f() {
			PlayrObject@ obj = GetPlayerIndex();
			
			obj.SetDashing( false );
		}
		void Crouch_Down_f() {
			PlayrObject@ obj = GetPlayerIndex();

			if ( obj.key_MoveNorth.active || obj.key_MoveSouth.active || obj.key_MoveEast.active || obj.key_MoveWest.active ) {
				// wait at little bit before launching another slide
				if ( obj.GetTimeSinceLastSlide() < 1000 ) {
					return;
				}

				if ( ( Util::PRandom() & 1 ) == 1 ) {
					obj.slideSfx0.Play();
				} else {
					obj.slideSfx1.Play();
				}
				
				Util::HapticRumble( obj.GetPlayerIndex(), 0.40f, 500 );
				obj.ResetSlide();
				obj.SetSliding( true );
			} else {
				obj.crouchDownSfx.Play();
				obj.SetState( @StateManager.GetStateForNum( StateNum::ST_PLAYR_CROUCHING ) );
			}
		}
		void Crouch_Up_f() {
			PlayrObject@ obj = GetPlayerIndex();
			
			if ( obj.IsCrouching() || obj.IsSliding() ) {
				obj.crouchUpSfx.Play();
			}
			obj.SetState( @StateManager.GetStateForNum( StateNum::ST_PLAYR_IDLE ) );
		}
		
		void Melee_Down_f() {
			PlayrObject@ obj = GetPlayerIndex();
			
			obj.meleeSfx.Play();
			obj.SetParryBoxWidth( 0.0f );
			obj.SetState( @StateManager.GetStateForNum( StateNum::ST_PLAYR_MELEE ) );
		}
		void Melee_Up_f() {
			// technically this allows animation canceling
			GetPlayerIndex().SetState( @StateManager.GetStateForNum( StateNum::ST_PLAYR_IDLE ) );
		}
		void NextWeapon_f() {
			PlayrObject@ obj = GetPlayerIndex();
			
			obj.SetCurrentWeapon( obj.GetCurrentWeaponIndex() + 1 );
			if ( obj.GetCurrentWeaponIndex() >= int( obj.m_WeaponSlots.Count() ) ) {
				obj.SetCurrentWeapon( 0 );
			}
			cast<const InfoSystem::WeaponInfo@>( @obj.m_WeaponSlots[ obj.GetCurrentWeaponIndex() ].GetInfo() ).equipSfx.Play();
		}
		void PrevWeapon_f() {
			PlayrObject@ obj = GetPlayerIndex();
			
			obj.SetCurrentWeapon( obj.GetCurrentWeaponIndex() - 1 );
			if ( obj.GetCurrentWeaponIndex() < 0 ) {
				obj.SetCurrentWeapon( obj.m_WeaponSlots.Count() - 1 );
			}
			cast<const InfoSystem::WeaponInfo@>( @obj.m_WeaponSlots[ obj.GetCurrentWeaponIndex() ].GetInfo() ).equipSfx.Play();
		}
		
		private void RenderScene( const uvec2& in scenePos, const uvec2& in sceneSize, const vec3& in origin ) {
			const uint flags = RSF_ORTHO_TYPE_WORLD;
			
			// snap to the player's position
			// if we're in photomode, ignore
//			if ( TheNomad::Engine::CvarVariableInteger( "r_debugCamera" ) == 0 ) 
			if ( origin.x >= LevelManager.GetMapData().GetWidth() / 2 ) {
				Game_CameraPos.x = origin.x + 9;
			} else {
				Game_CameraPos.x = origin.x;
			}
			if ( origin.y >= LevelManager.GetMapData().GetHeight() / 2 ) {
				Game_CameraPos.y = LevelManager.GetMapData().GetHeight() - ( origin.y + 10 );
			} else {
				Game_CameraPos.y = LevelManager.GetMapData().GetHeight() - origin.y;
			}
//			}
			// technically no z coordinate because it's 2D
			Game_CameraZoom = 0.070f;
			
			TheNomad::Engine::Renderer::ClearScene();
			for ( uint i = 0; i < TheNomad::GameSystem::GameSystems.Count(); i++ ) {
				TheNomad::GameSystem::GameSystems[i].OnRenderScene();
			}
			TheNomad::Engine::Renderer::RenderScene( scenePos.x, scenePos.y, sceneSize.x, sceneSize.y, flags, 
				uint( TheNomad::GameSystem::GameManager.GetGameTic() * 0.1f ) );
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

		uint GetPlayerCount() const {
			return m_nPlayerCount;
		}
		PlayrObject@ GetPlayerAt( uint nIndex ) {
			return @m_PlayerData[ nIndex ];
		}
		const PlayrObject@ GetPlayerAt( uint nIndex ) const {
			return @m_PlayerData[ nIndex ];
		}
		
		private PlayrObject@[] m_PlayerData( 4 );
		private uint m_nPlayerCount = 0;
	};
	
	SplitScreen ScreenData;
};