namespace TheNomad::SGame {
	const uint PMF_JUMP_HELD      = 0x01;
	const uint PMF_BACKWARDS_JUMP = 0x02;
	
	const float JUMP_VELOCITY = 2.5f;
	const float OVERCLIP = 1.5f;
	
    //
	// PMoveData
	// a class to buffer user input per frame
	//
	class PMoveData {
		PMoveData( PlayrObject@ ent ) {
			@m_EntityData = @ent;

			moveGravel0 = TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveGravel0.ogg" );
			moveGravel1 = TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveGravel1.ogg" );
			moveGravel2 = TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveGravel2.ogg" );
			moveGravel3 = TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveGravel3.ogg" );

			moveWater0 = TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveWater0.ogg" );
			moveWater1 = TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveWater1.ogg" );

			moveMetal0 = TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveMetal0.ogg" );
			moveMetal1 = TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveMetal1.ogg" );
			moveMetal2 = TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveMetal2.ogg" );
			moveMetal3 = TheNomad::Engine::ResourceCache.GetSfx( "sfx/players/moveMetal3.ogg" );
		}
		PMoveData() {
		}
		
		private void ClipVelocity( vec3& out vel, const vec3& in normal, float overbounce ) {
			float backoff;
			float change;
			int i;
			
			backoff = Util::DotProduct( vel, normal );
			if ( backoff < 0.0f ) {
				backoff *= overbounce;
			} else {
				backoff /= overbounce;
			}
			
			for ( i = 0; i < 3; i++ ) {
				change = normal[i] * backoff;
				vel[i] = vel[i] - change;
			}
		}
		
		private void AirMove() {
			vec3 vel, wishvel, wishdir;
			const uint gameTic = TheNomad::GameSystem::GameManager.GetGameTic();
			float smove, fmove;
			float velocity;
			float wishspeed;
			float accelerate;
			
			if ( !groundPlane ) {
				return;
			}
		}
		
		private void WalkMove() {
			const uint gameTic = TheNomad::GameSystem::GameManager.GetGameTic();
			vec3 accel = m_EntityData.GetPhysicsObject().GetAcceleration();
			
			KeyMove();
			
//			if ( forward != 0.0f && side != 0.0f ) {
//				if ( ( move_toggle % ( 16 + ( TheNomad::Engine::CvarVariableInteger( "com_maxfps" ) / 10 ) ) ) == 0.0f ) {
					accel.y += sgame_BaseSpeed.GetFloat() * forward;
					accel.x += sgame_BaseSpeed.GetFloat() * side;
//				}
//			}

			if ( m_EntityData.IsDashing() ) {
				accel.y += 5.5f * forward;
				accel.x += 5.5f * side;
				m_EntityData.dashSfx.Play();
				m_EntityData.SetDashing( false );
			}
			else if ( m_EntityData.IsSliding() ) {
				accel.y += 2.25f * forward;
				accel.x += 2.25f * side;
			}

			const uint tile = LevelManager.GetMapData().GetTile( m_EntityData.GetOrigin(), m_EntityData.GetBounds() );
			// it sound like a machine gun if it isn't spaced out
			if ( ( tile & SURFACEPARM_WATER ) != 0 ) {
				switch ( TheNomad::Util::PRandom() & 2 ) {
				case 0:
					moveWater0.Play();
					break;
				case 1:
					moveWater1.Play();
					break;
				};
			}
			if ( ( tile & SURFACEPARM_METAL ) != 0 ) {
				switch ( TheNomad::Util::PRandom() & 3 ) {
				case 0:
					moveMetal0.Play();
					break;
				case 1:
					moveMetal1.Play();
					break;
				case 2:
					moveMetal2.Play();
					break;
				case 3:
					moveMetal3.Play();
					break;
				};
			}
			if ( accel.x != 0.0f || accel.y != 0.0f ) {
				if ( ( move_toggle % ( 16 + ( TheNomad::Engine::CvarVariableInteger( "com_maxfps" ) / 10 ) ) ) == 0.0f ) {
					switch ( TheNomad::Util::PRandom() & 3 ) {
					case 0:
					case 2:
						moveGravel0.Play();
						break;
					case 1:
					case 3:
						moveGravel1.Play();
						break;
					};
				}
				move_toggle++;
			}
			
			m_EntityData.GetPhysicsObject().SetAcceleration( accel );
		}
		
		private void WaterMove() {
			const uint gameTic = TheNomad::GameSystem::GameManager.GetGameTic();
			vec3 accel = m_EntityData.GetPhysicsObject().GetAcceleration();
			
			KeyMove();
		}
		
		private bool CheckJump() {
			vec3 accel = m_EntityData.GetPhysicsObject().GetAcceleration();
			
			if ( upmove == 0.0f ) {
				// no holding jump
				accel.z = 0;
				flags &= ~PMF_JUMP_HELD;
				m_EntityData.GetPhysicsObject().SetAcceleration( accel );
				return false;
			}
			
			if ( ( flags & PMF_JUMP_HELD ) != 0 ) {
				// double jump
				m_EntityData.SetState( @StateManager.GetStateForNum( StateNum::ST_PLAYR_DOUBLEJUMP ) );
			}
			
			flags |= PMF_JUMP_HELD;
			accel.z += JUMP_VELOCITY;
			groundPlane = false;
			
			if ( forward >= 0 ) {
				flags &= ~PMF_BACKWARDS_JUMP;
			} else {
				flags |= PMF_BACKWARDS_JUMP;
			}
			
			m_EntityData.GetPhysicsObject().SetAcceleration( accel );
			
			return true;
		}
		
		void Accelerate( const vec3& in wishdir, float wishspeed, float accel ) {
			vec3 vel = m_EntityData.GetPhysicsObject().GetAcceleration();
			
			float currentspeed = Util::DotProduct( vel, vel );
			float addspeed = wishspeed - currentspeed;
			if ( addspeed <= 0.0f ) {
				return;
			}
			float accelspeed = accel * TheNomad::GameSystem::GameManager.GetGameTic() * wishspeed;
			if ( accelspeed > addspeed ) {
				accelspeed = addspeed;
			}
			
			for ( int i = 0; i < 3; i++ ) {
				vel[i] += accelspeed * wishdir[i];
			}
			m_EntityData.GetPhysicsObject().SetAcceleration( vel );
		}
		
		//
		// CmdScale: returns the scale factor to appply to cmd movements
		// this allows the user to use axial -127 to 127 values for all directions
		// without getting a sqrt(2) distortion in speed.
		//
		/*
		void CmdScale( int& out forward, int& out side, int& out upmove ) {
			int max;
			float total;
			float scale;
			
			max = abs( forward );
			if ( abs( side ) > max ) {
				max = abs( side );
			}
			if ( abs( upmove ) > max ) {
				max = abs( upmove );
			}
			if ( max < 0.0f ) {
				reutrn 0;
			}
			total = sqrt( forward * forward + side * side + upmove * upmove );
			scale = float( m_Speed ) * float( max ) / ( 127.0f * total );
			
			return scale;
		}
		*/
		
		void SetMovementDir() {
			// set legs direction
			if ( side > 0 ) {
				m_EntityData.SetLegsFacing( FACING_RIGHT );
			} else if ( side < 0 ) {
				m_EntityData.SetLegsFacing( FACING_LEFT );
			}

			//
			// set torso direction
			//
			if ( TheNomad::Engine::CvarVariableInteger( "in_mode" ) == 0 ) {
				// mouse & keyboard
				// position torso facing wherever the mouse is
				const uvec2 mousePos = TheNomad::GameSystem::GameManager.GetMousePos();
				const int screenWidth = TheNomad::GameSystem::GameManager.GetGPUConfig().screenWidth;
				const int screenHeight = TheNomad::GameSystem::GameManager.GetGPUConfig().screenHeight;
				
				float angle = atan2( float( mousePos.y ) - float( screenHeight / 2 ),
					float( screenHeight / 2 ) - float( mousePos.x ) );
//				TheNomad::GameSystem::DirType dir = Util::Angle2Dir( angle );
				
				switch ( m_EntityData.GetDirection() ) {
				case TheNomad::GameSystem::DirType::North:
//					m_EntityData.SetFacing( FACING_UP );
					m_EntityData.SetFacing( FACING_RIGHT );
					break;
				case TheNomad::GameSystem::DirType::South:
//					m_EntityData.SetFacing( FACING_DOWN );
					m_EntityData.SetFacing( FACING_RIGHT );
					break;
				case TheNomad::GameSystem::DirType::NorthEast:
				case TheNomad::GameSystem::DirType::SouthEast:
				case TheNomad::GameSystem::DirType::East:
					m_EntityData.SetFacing( FACING_RIGHT );
					break;
				case TheNomad::GameSystem::DirType::NorthWest:
				case TheNomad::GameSystem::DirType::SouthWest:
				case TheNomad::GameSystem::DirType::West:
					m_EntityData.SetFacing( FACING_LEFT );
					break;
				default:
					GameError( "PMoveData::RunTic: Invalid DirType " + uint( m_EntityData.GetDirection() ) );
					break;
				};
			}
			else {
				if ( side > 0 ) {
					m_EntityData.SetFacing( FACING_RIGHT );
				} else if ( side < 0 ) {
					m_EntityData.SetFacing( FACING_LEFT );
				}

				if ( side > forward ) {
					if ( side > 0 ) {
						m_EntityData.SetDirection( GameSystem::DirType::East );
					} else if ( side < 0 ) {
						m_EntityData.SetDirection( GameSystem::DirType::West );
					}
				} else if ( forward > side ) {
					if ( forward > 0 ) {
						m_EntityData.SetDirection( GameSystem::DirType::North );
					} else if ( forward < 0 ) {
						m_EntityData.SetDirection( GameSystem::DirType::South );
					}
				}
			}
		}
		
		void RunTic() {
			float angle;
			TheNomad::GameSystem::DirType dir;
			const uint gameTic = TheNomad::GameSystem::GameManager.GetGameTic();
			
			frametime = uint( float( gameTic * 0.0001f ) );
			
			frame_msec = TheNomad::Engine::System::Milliseconds() - old_frame_msec;
			
			// if running over 1000fps, act as if each frame is 1ms
			// prevents divisions by zero
			if ( frame_msec < 1 ) {
				frame_msec = 1;
			}

			// if running less than 5fps, truncate the extra time to prevent
			// unexpected moves after a hitch
			if ( frame_msec > 200 ) {
				frame_msec = 200;
			}
			old_frame_msec = TheNomad::Engine::System::Milliseconds();
			
			if ( up < 1.0f ) {
				// not holding jump
				flags &= ~PMF_JUMP_HELD;
			}
			CheckJump();
			
			groundPlane = up == 0;

			if ( m_EntityData.GetWaterLevel() > 1 ) {
				WaterMove();
			} else if ( groundPlane ) {
				WalkMove();
			} else {
				AirMove();
			}

			SetMovementDir();

			TheNomad::Engine::UserInterface::SetActiveFont( TheNomad::Engine::UserInterface::Font_RobotoMono );
			ImGui::Begin( "Debug Player Movement", null, ImGuiWindowFlags::AlwaysAutoResize );
			ImGui::Text( "Velocity: [ " + m_EntityData.GetVelocity().x + ", " + m_EntityData.GetVelocity().y + " ]" );
			ImGui::Text( "CameraPos: [ " + Game_CameraPos.x + ", " + Game_CameraPos.y + " ]" );
			ImGui::Text( "Forward: " + forward );
			ImGui::Text( "Side: " + side );
			ImGui::Separator();
			ImGui::Text( "North MSec: " + m_EntityData.key_MoveNorth.msec );
			ImGui::Text( "South MSec: " + m_EntityData.key_MoveSouth.msec );
			ImGui::Text( "East MSec: " + m_EntityData.key_MoveEast.msec );
			ImGui::Text( "West MSec: " + m_EntityData.key_MoveWest.msec );
			ImGui::Separator();
			ImGui::Text( "Torso Direction: " + m_EntityData.GetFacing() );
			ImGui::Separator();
			ImGui::Text( "Bounding Box" );
			{
				const vec3 mins = m_EntityData.GetBounds().m_Mins;
				const vec3 maxs = m_EntityData.GetBounds().m_Maxs;

				ImGui::Text( "mins[0]: " + mins.x );
				ImGui::Text( "mins[1]: " + mins.y );
				ImGui::Text( "maxs[0]: " + maxs.x );
				ImGui::Text( "maxs[1]: " + maxs.y );
			}
			ImGui::Separator();
			ImGui::Text( "GameTic: " + gameTic );
			ImGui::End();

			m_EntityData.key_MoveNorth.msec = 0;
			m_EntityData.key_MoveSouth.msec = 0;
			m_EntityData.key_MoveEast.msec = 0;
			m_EntityData.key_MoveWest.msec = 0;

			m_EntityData.GetPhysicsObject().OnRunTic();
		}
		
		private float KeyState( KeyBind& in key ) {
			int msec;
			float val;

			msec = key.msec;
//			key.msec = 0;
			
			if ( key.active ) {
				// still down
				if ( key.downtime <= 0 ) {
					msec = TheNomad::GameSystem::GameManager.GetGameTic();
				} else {
					msec += TheNomad::GameSystem::GameManager.GetGameTic() - key.downtime;
				}
				key.downtime = TheNomad::GameSystem::GameManager.GetGameTic();
			}

			val = Util::Clamp( float( msec ) / float( frame_msec ), float( 0 ), float( 1 ) );

			return val;
		}

		void KeyMove() {
			float base = 1.25f;

			forward = 0.0f;
			side = 0.0f;
			up = 0.0f;
			
			side += KeyState( m_EntityData.key_MoveEast ) * base;
			side -= KeyState( m_EntityData.key_MoveWest ) * base;
			
			up += 1.25f * KeyState( m_EntityData.key_Jump );
			
			forward -= base * KeyState( m_EntityData.key_MoveNorth );
			forward += base * KeyState( m_EntityData.key_MoveSouth );

			northmove = southmove = 0.0f;
			if ( forward > 0 ) {
				northmove = Util::Clamp( forward * KeyState( m_EntityData.key_MoveNorth ), -sgame_MaxSpeed.GetFloat(), sgame_MaxSpeed.GetFloat() );
			} else if ( forward < 0 ) {
				southmove = Util::Clamp( forward * KeyState( m_EntityData.key_MoveSouth ), -sgame_MaxSpeed.GetFloat(), sgame_MaxSpeed.GetFloat() );
			}

			eastmove = westmove = 0.0f;
			if ( side > 0 ) {
				eastmove = Util::Clamp( side * KeyState( m_EntityData.key_MoveEast ), -sgame_MaxSpeed.GetFloat(), sgame_MaxSpeed.GetFloat() );
			} else if ( side < 0 ) {
				westmove = Util::Clamp( side * KeyState( m_EntityData.key_MoveWest ), -sgame_MaxSpeed.GetFloat(), sgame_MaxSpeed.GetFloat() );
			}
		}
		
		PlayrObject@ m_EntityData = null;
		
		float forward = 0.0f;
		float side = 0.0f;
		float up = 0.0f;
		float upmove = 0.0f;
		float northmove = 0.0f;
		float southmove = 0.0f;
		float eastmove = 0.0f;
		float westmove = 0.0f;
		
		uint flags = 0;
		uint frametime = 0;

		uint frame_msec = 0;
		int old_frame_msec = 0;

		int move_toggle = 0;
		
		TheNomad::Engine::SoundSystem::SoundEffect moveGravel0;
		TheNomad::Engine::SoundSystem::SoundEffect moveGravel1;
		TheNomad::Engine::SoundSystem::SoundEffect moveGravel2;
		TheNomad::Engine::SoundSystem::SoundEffect moveGravel3;

		TheNomad::Engine::SoundSystem::SoundEffect moveWater0;
		TheNomad::Engine::SoundSystem::SoundEffect moveWater1;

		TheNomad::Engine::SoundSystem::SoundEffect moveMetal0;
		TheNomad::Engine::SoundSystem::SoundEffect moveMetal1;
		TheNomad::Engine::SoundSystem::SoundEffect moveMetal2;
		TheNomad::Engine::SoundSystem::SoundEffect moveMetal3;
		
		bool groundPlane = false;
	};
};