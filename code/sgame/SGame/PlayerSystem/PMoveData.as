namespace TheNomad::SGame {
	const uint PMF_JUMP_HELD      = 0x01;
	const uint PMF_BACKWARDS_JUMP = 0x02;

	const uint DASH_DURATION = 50;
	const uint SLIDE_DURATION = 700;
	
	const float JUMP_VELOCITY = 3.5f;
	const float OVERCLIP = 1.5f;
	
    //
	// PMoveData
	// a class to buffer user input per frame
	//
	class PMoveData {
		PMoveData( PlayrObject@ ent ) {
			@m_EntityData = @ent;

			moveGravel0 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_gravel_0" );
			moveGravel1 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_gravel_1" );
			moveGravel2 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_gravel_2" );
			moveGravel3 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_gravel_3" );

			moveWater0 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_water_0" );
			moveWater1 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_water_1" );

			moveMetal0 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_metal_0" );
			moveMetal1 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_metal_1" );
			moveMetal2 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_metal_2" );
			moveMetal3 = TheNomad::Engine::SoundSystem::RegisterSfx( "event:/sfx/env/world/move_metal_3" );
		}
		PMoveData() {
		}
		
		private void AirMove() {
			vec3 vel, wishvel, wishdir;
			const uint gameTic = TheNomad::GameSystem::GameTic;
			float smove, fmove;
			float velocity;
			float wishspeed;
			float accelerate;

			if ( !groundPlane ) {
				return;
			}
		}
		
		private void WalkMove() {
			const uint gameTic = TheNomad::GameSystem::GameTic;
			vec3 accel = m_EntityData.GetPhysicsObject().GetAcceleration();

			const bool isSliding = ( m_EntityData.Flags & PF_SLIDING ) != 0;
			const bool isDashing = ( m_EntityData.Flags & PF_DASHING ) != 0;

			if ( !( isSliding || isDashing ) ) {
				// if we're dashing or sliding, we won't really have
				// nearly as much control over where we're going
				KeyMove();
			}
			accel.x += side;
			accel.y += forward;
			if ( ( flags & PMF_JUMP_HELD ) != 0 ) {
				accel.z += up;
			}

			if ( isDashing ) {
				accel.y += 4.50f * forward;
				accel.x += 4.50f * side;
				m_EntityData.SetDashing(
					( TheNomad::GameSystem::GameTic - m_EntityData.DashEndTime ) * TheNomad::GameSystem::DeltaTic < DASH_DURATION
				);
			} else {
				m_EntityData.DashEndTime = 0;
			}
			if ( isSliding ) {
				accel.y += 0.15f * forward;
				accel.x += 0.15f * side;
				m_EntityData.SetSliding(
					( TheNomad::GameSystem::GameTic - m_EntityData.SlideEndTime ) * TheNomad::GameSystem::DeltaTic < SLIDE_DURATION
				);
			} else {
				m_EntityData.SlideEndTime = 0;
			}
			
			if ( !isSliding ) {
				const uint64 tile = LevelManager.GetMapData().GetTile( m_EntityData.GetOrigin(), m_EntityData.GetBounds() );
				if ( accel.x != 0.0f || accel.y != 0.0f ) {
					// sync the extra particles and sounds with the actual animation
					const uint lerpTime = m_EntityData.GetLegState().GetAnimation().GetLerpTime();

					uint moveTime = TheNomad::GameSystem::GameTic - move_toggle;
					if ( EntityManager.GetActivePlayer().InReflex() ) {
						moveTime *= TheNomad::GameSystem::DeltaTic;
					}
					if ( moveTime >= lerpTime && m_EntityData.GetOrigin().z == 0.0f ) {
						// we can mix in different surfaceparm sound effects for more complex environments
						float volume = 10.0f;
						if ( ( tile & SURFACEPARM_WATER ) != 0 ) {
							switch ( TheNomad::Util::PRandom() & 1 ) {
							case 0:
								m_EntityData.EmitSound( moveWater0, 10.0f, 0xff );
								break;
							case 1:
								m_EntityData.EmitSound( moveWater1, 10.0f, 0xff );
								break;
							};
							volume = 4.5f;

							// it'll look weird if we're drawing ripples right as the player exits the water
							// because it looks like the dirt is rippling
							const vec3 origin = m_EntityData.GetOrigin();
							const vec3 tmp = origin + accel;
							TheNomad::GameSystem::BBox bounds;
							bounds.m_nWidth = sgame_PlayerWidth.GetFloat();
							bounds.m_nHeight = sgame_PlayerHeight.GetFloat();
							bounds.MakeBounds( tmp );

							const uint64 next = LevelManager.GetMapData().GetTile( tmp, bounds );
							
							if ( ( next & SURFACEPARM_WATER ) != 0 ) {
								GfxManager.AddWaterWake( m_EntityData.GetOrigin(), 800, Util::VectorLength( accel ) );
							}
						}
						if ( ( tile & SURFACEPARM_METAL ) != 0 ) {
							switch ( TheNomad::Util::PRandom() & 3 ) {
							case 0:
								m_EntityData.EmitSound( moveMetal0, 10.0f, 0xff );
								break;
							case 1:
								m_EntityData.EmitSound( moveMetal1, 10.0f, 0xff );
								break;
							case 2:
								m_EntityData.EmitSound( moveMetal2, 10.0f, 0xff );
								break;
							case 3:
								m_EntityData.EmitSound( moveMetal3, 10.0f, 0xff );
								break;
							};
							volume = 4.5f;
						}
						switch ( TheNomad::Util::PRandom() & 3 ) {
						case 0:
							m_EntityData.EmitSound( moveGravel0, volume, 0xff );
							break;
						case 2:
							m_EntityData.EmitSound( moveGravel1, volume, 0xff );
							break;
						case 1:
							m_EntityData.EmitSound( moveGravel2, volume, 0xff );
							break;
						case 3:
							m_EntityData.EmitSound( moveGravel3, volume, 0xff );
							break;
						};
						move_toggle = TheNomad::GameSystem::GameTic;

						if ( m_EntityData.GetWaterLevel() == 0 ) {
							// dont kick up much dust underwater
							vec3 origin = m_EntityData.GetOrigin();

							switch ( m_EntityData.GetFacing() ) {
							case FACING_LEFT:
								origin.x += 0.15f;
								break;
							case FACING_RIGHT:
								origin.x -= 0.15f;
								break;
							};

							GfxManager.AddDustPuff( origin, m_EntityData.GetFacing() );
						}
					}
				}
			}
			
			m_EntityData.GetPhysicsObject().SetAcceleration( accel );
		}
		
		private bool CheckJump() {
			vec3 accel = m_EntityData.GetPhysicsObject().GetAcceleration();
			
			if ( m_EntityData.key_Jump.msec == 0 ) {
				// no holding jump
				accel.z = 0;
				flags &= ~PMF_JUMP_HELD;
				m_EntityData.GetPhysicsObject().SetAcceleration( accel );
				return false;
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
		
		void SetMovementDir() {
		#if _NOMAD_DEBUG
			if ( TheNomad::Engine::CvarVariableInteger( "sgame_DebugMode" ) ) {
				TheNomad::Engine::ProfileBlock block( "PMoveData::SetMovementDir" );
			}
		#endif

			// set movement direction
			if ( side > 0 ) {
				m_EntityData.SetFacing( FACING_RIGHT );
				m_EntityData.SetLegsFacing( FACING_RIGHT );
				m_EntityData.LeftArm.Facing = FACING_RIGHT;
				m_EntityData.RightArm.Facing = FACING_RIGHT;
			} else if ( side < 0 ) {
				m_EntityData.SetFacing( FACING_LEFT );
				m_EntityData.SetLegsFacing( FACING_LEFT );
				m_EntityData.LeftArm.Facing = FACING_LEFT;
				m_EntityData.RightArm.Facing = FACING_LEFT;
			}

			//
			// set torso direction
			//
			
			// mouse & keyboard
			// position torso facing wherever the mouse is
			m_nArmsAngle = atan2( TheNomad::GameSystem::HalfScreenHeight - float( TheNomad::GameSystem::MousePosition.y ),
				TheNomad::GameSystem::HalfScreenWidth - float( TheNomad::GameSystem::MousePosition.x ) );
			
			if ( TheNomad::GameSystem::MousePosition.x < TheNomad::GameSystem::HalfScreenWidth ) {
				/*
				if ( @m_EntityData.GetLeftHandWeapon() !is null ) {
					m_EntityData.SetLeftArmFacing( FACING_LEFT );
				}
				if ( @m_EntityData.GetRightHandWeapon() !is null ) {
					m_EntityData.SetRightArmFacing( FACING_LEFT );
				}
				*/
				m_nArmsAngle = -m_nArmsAngle;
			} else if ( TheNomad::GameSystem::MousePosition.x > TheNomad::GameSystem::HalfScreenWidth ) {
				/*
				if ( @m_EntityData.GetLeftHandWeapon() !is null ) {
					m_EntityData.SetLeftArmFacing( FACING_RIGHT );
				}
				if ( @m_EntityData.GetRightHandWeapon() !is null ) {
					m_EntityData.SetRightArmFacing( FACING_RIGHT );
				}
				*/
			}
		}
		
		void RunTic() {
		#if _NOMAD_DEBUG
			if ( sgame_DebugMode.GetBool() ) {
				TheNomad::Engine::ProfileBlock block( "PMoveData::OnRunTic" );
			}
		#endif
			
			frame_msec = TheNomad::GameSystem::GameTic - old_frame_msec;
			
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
			old_frame_msec = TheNomad::GameSystem::GameTic;
			
			groundPlane = m_EntityData.GetWaterLevel() < 1;

			WalkMove();
			AirMove();

			SetMovementDir();

			TheNomad::Engine::UserInterface::SetActiveFont( TheNomad::Engine::UserInterface::Font_RobotoMono );

			ImGui::Begin( "Debug Player Movement", null, ImGuiWindowFlags::AlwaysAutoResize );
			ImGui::SetWindowPos( vec2( 16, 128 ) );
			ImGui::Text( "Origin: [ " + m_EntityData.GetOrigin().x + ", " + m_EntityData.GetOrigin().y + ", " + m_EntityData.GetOrigin().z + " ]" );
			ImGui::Text( "Velocity: [ " + m_EntityData.GetVelocity().x + ", " + m_EntityData.GetVelocity().y + ", " + m_EntityData.GetVelocity().z + " ]" );
			ImGui::Text( "CameraPos: [ " + Game_CameraWorldPos.x + ", " + Game_CameraWorldPos.y + " ]" );
			ImGui::Text( "Forward: " + forward );
			ImGui::Text( "Side: " + side );
			ImGui::Separator();
			ImGui::Text( "DashTime: " + m_EntityData.DashEndTime );
			ImGui::Text( "DashCounter: " + m_EntityData.DashCounter );
			ImGui::Separator();
			ImGui::Text( "North MSec: " + m_EntityData.key_MoveNorth.msec );
			ImGui::Text( "South MSec: " + m_EntityData.key_MoveSouth.msec );
			ImGui::Text( "East MSec: " + m_EntityData.key_MoveEast.msec );
			ImGui::Text( "West MSec: " + m_EntityData.key_MoveWest.msec );
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
			ImGui::Text( "Arm Angle: " + m_nArmsAngle );
			ImGui::Separator();
			ImGui::Text( "LegState: " + m_EntityData.GetLegState().GetName() );
			ImGui::Text( "LegAnimation:" );
			ImGui::Text( "  Frame: " + m_EntityData.GetLegState().GetAnimation().GetFrame() );
			ImGui::Text( "  NumFrames: " + m_EntityData.GetLegState().GetAnimation().NumFrames() );
			ImGui::End();

			m_EntityData.GetPhysicsObject().OnRunTic();
		}
		
		private float KeyState( KeyBind@ key ) {
			int msec = key.msec;
			key.msec = 0;
			
			if ( key.active ) {
				// still down
				if ( key.downtime <= 0 ) {
					msec = TheNomad::GameSystem::GameTic;
				} else {
					msec += TheNomad::GameSystem::GameTic - key.downtime;
				}
				key.downtime = TheNomad::GameSystem::GameTic;
			}

			return Util::Clamp( float( msec ) / float( frame_msec ), float( 0 ), float( 1 ) );
		}

		void KeyMove() {
			float base = 1.25f;

			if ( m_EntityData.GetWaterLevel() > 0 ) {
				base = 1.05;
			}

			forward = 0.0f;
			side = 0.0f;
			up = 0.0f;
			
			side += base * KeyState( @m_EntityData.key_MoveEast );
			side -= base * KeyState( @m_EntityData.key_MoveWest );

			const float jump = KeyState( @m_EntityData.key_Jump );
			if ( m_EntityData.GetOrigin().z == 0.0f ) {
				up += base * jump;
			}
			
			forward -= base * KeyState( @m_EntityData.key_MoveNorth );
			forward += base * KeyState( @m_EntityData.key_MoveSouth );
		}

		ivec2 m_JoystickPosition = ivec2( 0 );
		
		PlayrObject@ m_EntityData = null;

		float m_nArmsAngle = 0.0f;
		
		float forward = 0.0f;
		float side = 0.0f;
		float up = 0.0f;
		float upmove = 0.0f;
		
		uint flags = 0;

		uint frame_msec = 0;
		int old_frame_msec = 0;

		uint move_toggle = 0;
		uint jump_toggle = 0;
		
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