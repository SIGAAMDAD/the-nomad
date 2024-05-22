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
			//ApplyFriction();
			
			fmove = forward;
			smove = side;
			
			vel = m_EntityData.GetPhysicsObject().GetAcceleration();
			velocity = Util::VectorLength( vel );
			ClipVelocity( vel, vec3( 1.0f ), OVERCLIP );
			Util::VectorNormalize( vel );
			
			wishvel[0] = northmove * fmove + eastmove * smove;
			wishvel[1] = southmove * fmove + westmove * smove;
			wishvel[2] = upmove;

			accelerate = sgame_AirSpeed.GetFloat();
			
			wishdir = wishvel;
			wishspeed = Util::VectorNormalize( wishdir );
			wishspeed *= accelerate;
			
			Accelerate( wishdir, wishspeed, accelerate );
			Util::VectorScale( vel, velocity, vel );
			m_EntityData.GetPhysicsObject().SetAcceleration( vel );
		}
		
		private void WalkMove() {
			vec3 vel, wishvel, wishdir;
			const uint gameTic = TheNomad::GameSystem::GameManager.GetGameTic();
			float smove, fmove;
			float velocity;
			float wishspeed;
			float accelerate;
			
			if ( !groundPlane ) {
				return;
			}
			//ApplyFriction();
			
			fmove = forward;
			smove = side;
			
			vel = m_EntityData.GetPhysicsObject().GetAcceleration();
			velocity = Util::VectorLength( vel );
			ClipVelocity( vel, vec3( 1.0f ), OVERCLIP );
			Util::VectorNormalize( vel );
			
			wishvel[0] = northmove * fmove + eastmove * smove;
			wishvel[1] = southmove * fmove + westmove * smove;
			wishvel[2] = upmove;

			accelerate = sgame_BaseSpeed.GetFloat();
			
			wishdir = wishvel;
			wishspeed = Util::VectorNormalize( wishdir );
			wishspeed *= accelerate;
			
			// clamp the speed lower if wading or walking on the bottom
			if ( m_EntityData.GetWaterLevel() > 0 ) {
				float waterScale;
				
				waterScale = m_EntityData.GetWaterLevel() / 3.0f;
				waterScale = 1.0f - ( 1.0f - sgame_SwimSpeed.GetFloat() ) * waterScale;
				if ( wishspeed > vel * waterScale ) {
					wishspeed = sgame_SwimSpeed.GetFloat() * waterScale;
				}
			}
			
//			Accelerate( wishdir, wishspeed, accelerate );
//			Util::VectorScale( vel, velocity, vel );
			m_EntityData.GetPhysicsObject().SetAcceleration( vel );
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
		
		private void SetMovementDir() {
			if ( forward > side ) {
				if ( northmove > southmove ) {
					m_EntityData.SetLegFacing( FACING_FORWARD );
				}
				else if ( southmove > northmove ) {
					m_EntityData.SetLegFacing( FACING_BACKWARD );
				}
			}
			else if ( side > forward ) {
				if ( eastmove > westmove ) {
					m_EntityData.SetLegFacing( FACING_RIGHT );
				}
				else if ( westmove > eastmove ) {
					m_EntityData.SetLegFacing( FACING_LEFT );
				}
			}
		}
		
		void RunTic() {
			uvec2 mousePos;
			int screenWidth, screenHeight;
			float angle;
			TheNomad::GameSystem::DirType dir;
			const uint gameTic = TheNomad::GameSystem::GameManager.GetGameTic();
			
			mousePos = TheNomad::GameSystem::GameManager.GetMousePos();
			screenWidth = TheNomad::GameSystem::GameManager.GetGPUConfig().screenWidth;
			screenHeight = TheNomad::GameSystem::GameManager.GetGPUConfig().screenHeight;

			frame_msec = Game_FrameTime - old_frame_msec;

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
			old_frame_msec = Game_FrameTime;
			
			/*
			northmove = m_EntityData.key_MoveNorth.msec;
			southmove = m_EntityData.key_MoveSouth.msec;
			eastmove = m_EntityData.key_MoveEast.msec;
			westmove = m_EntityData.key_MoveWest.msec;
			upmove = m_EntityData.key_Jump.msec;
			*/
			KeyMove();
			
			SetMovementDir();
			
			if ( upmove < 1.0f ) {
				// not holding jump
				flags &= ~PMF_JUMP_HELD;
			}
			CheckJump();

			groundPlane = upmove == 0;
			
			if ( m_EntityData.GetWaterLevel() > 1 ) {
//				WaterMove();
			} else if ( groundPlane ) {
				vec3 accel = m_EntityData.GetPhysicsObject().GetAcceleration();

				accel.y -= northmove;
				accel.y += southmove;
				accel.x -= westmove;
				accel.x += eastmove;

				m_EntityData.GetPhysicsObject().SetAcceleration( accel );
			} else {
				AirMove();
			}
			
			// set torso direction
			angle = atan2( ( screenWidth / 2 ) - mousePos.x, ( screenHeight / 2 ) - mousePos.y );
			dir = Util::Angle2Dir( angle );
			
			switch ( dir ) {
			case TheNomad::GameSystem::DirType::North:
			case TheNomad::GameSystem::DirType::South:
				break; // not implemented for now
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
				break;
			};

			ImGui::Begin( "Debug Player Movement" );
			ImGui::Text( "NorthMove: " + northmove );
			ImGui::Text( "SouthMove: " + southmove );
			ImGui::Text( "EastMove: " + eastmove );
			ImGui::Text( "WestMove: " + westmove );
			ImGui::Text( "Velocity: [ " + m_EntityData.GetVelocity().x + ", " + m_EntityData.GetVelocity().y + " ]" );
			ImGui::Separator();
			ImGui::Text( "North MSec: " + m_EntityData.key_MoveNorth.msec );
			ImGui::Text( "South MSec: " + m_EntityData.key_MoveSouth.msec );
			ImGui::Text( "East MSec: " + m_EntityData.key_MoveEast.msec );
			ImGui::Text( "West MSec: " + m_EntityData.key_MoveWest.msec );
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
			key.msec = 0;
			
			if ( key.active ) {
				// still down
				if ( key.downtime <= 0 ) {
					msec = Game_FrameTime;
				} else {
					msec += Game_FrameTime - key.downtime;
				}
				key.downtime = Game_FrameTime;
			}

			val = Util::Clamp( float( msec ) / float( frame_msec ), float( 0 ), float( 1 ) );

			return val;
		}

		private void KeyMove() {
			const int movespeed = 4;
			
			forward = 0;
			side = 0;
			up = 0;
			
			// FIXME: limiting movement to only if the key is directly held might
			// lead to not being able to quickly inverse directional movement
			
			side += int( movespeed * KeyState( m_EntityData.key_MoveEast ) );
			side -= int( movespeed * KeyState( m_EntityData.key_MoveWest ) );
			
			up += int( movespeed * KeyState( m_EntityData.key_Jump ) );
			
			forward += int( movespeed * KeyState( m_EntityData.key_MoveNorth ) );
			forward -= int( movespeed * KeyState( m_EntityData.key_MoveSouth ) );
			
			northmove = southmove = 0.0f;
			if ( forward > 0 ) {
				northmove = Util::Clamp( KeyState( m_EntityData.key_MoveNorth ), -sgame_MaxSpeed.GetFloat(), sgame_MaxSpeed.GetFloat() );
			} else if ( forward < 0 ) {
				southmove = Util::Clamp( KeyState( m_EntityData.key_MoveSouth ), -sgame_MaxSpeed.GetFloat(), sgame_MaxSpeed.GetFloat() );
			}

			westmove = eastmove = 0.0f;
			if ( side > 0 ) {
				eastmove = Util::Clamp( KeyState( m_EntityData.key_MoveEast ), -sgame_MaxSpeed.GetFloat(), sgame_MaxSpeed.GetFloat() );
			} else if ( side < 0 ) {
				westmove = Util::Clamp( KeyState( m_EntityData.key_MoveWest ), -sgame_MaxSpeed.GetFloat(), sgame_MaxSpeed.GetFloat() );
			}
		}
		
		PlayrObject@ m_EntityData = null;
		
		int forward = 0;
		int side = 0;
		int up = 0;
		float northmove = 0.0f;
		float southmove = 0.0f;
		float eastmove = 0.0f;
		float westmove = 0.0f;
		float upmove = 0.0f;
		
		uint flags = 0;

		uint frame_msec = 0;
		int old_frame_msec = 0;
		
		bool groundPlane = false;
	};
};