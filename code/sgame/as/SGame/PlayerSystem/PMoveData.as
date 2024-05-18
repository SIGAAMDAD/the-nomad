namespace TheNomad::SGame {
    ///
	/// PMoveData
	/// a class to buffer user input per frame
	///
	class PMoveData {
		PMoveData( PlayrObject@ ent ) {
			@m_EntityData = @ent;
		}
		PMoveData() {
		}
		
		private void WalkMove( PlayrObject@ ent ) {
			vec3 vel;
			const uint gameTic = TheNomad::GameSystem::GameManager.GetGameTic();
			
			if ( !groundPlane ) {
				return;
			}
			
			vel = ent.GetVelocity();
			if ( northmove > 0 ) {
				vel.y -= northmove / gameTic;
			}
			if ( southmove > 0 ) {
				vel.y += southmove / gameTic;
			}
			if ( westmove > 0 ) {
				vel.x -= westmove / gameTic;
			}
			if ( eastmove > 0 ) {
				vel.x += eastmove / gameTic;
			}

			vel.x = Util::Clamp( vel.x, -sgame_MaxSpeed.GetFloat(), sgame_MaxSpeed.GetFloat() );
			vel.y = Util::Clamp( vel.y, -sgame_MaxSpeed.GetFloat(), sgame_MaxSpeed.GetFloat() );

			ent.GetPhysicsObject().SetAcceleration( vel );
		}
		
		private void AirMove() {
			
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
			int movespeed = 4;
			int side = 0;
			int forward = 0;
			int up = 0;

			side += uint( movespeed * KeyState( m_EntityData.key_MoveEast ) );
			side -= uint( movespeed * KeyState( m_EntityData.key_MoveWest ) );

			up += uint( movespeed * KeyState( m_EntityData.key_Jump ) );

			forward += uint( movespeed * KeyState( m_EntityData.key_MoveNorth ) );
			forward -= uint( movespeed * KeyState( m_EntityData.key_MoveSouth ) );

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

			// set leg sprite
			if ( eastmove > westmove ) {
				m_EntityData.SetLegFacing( 0 );
			} else if ( westmove > eastmove ) {
				m_EntityData.SetLegFacing( 1 );
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
				m_EntityData.SetFacing( 0 );
				break;
			case TheNomad::GameSystem::DirType::NorthWest:
			case TheNomad::GameSystem::DirType::SouthWest:
			case TheNomad::GameSystem::DirType::West:
				m_EntityData.SetFacing( 1 );
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
			
			if ( m_EntityData.key_Jump.active && m_EntityData.GetOrigin().z > 0.0f ) {
				// pressed jump again or we're falling, trigger jump
				m_EntityData.SetState( @StateManager.GetStateForNum( StateNum::ST_PLAYR_DOUBLEJUMP ) );
			}
			
			groundPlane = upmove == 0;

			WalkMove( @m_EntityData );

			m_EntityData.key_MoveNorth.msec = 0;
			m_EntityData.key_MoveSouth.msec = 0;
			m_EntityData.key_MoveEast.msec = 0;
			m_EntityData.key_MoveWest.msec = 0;

			m_EntityData.GetPhysicsObject().OnRunTic();
		}
		
		PlayrObject@ m_EntityData = null;
		float northmove = 0.0f;
		float southmove = 0.0f;
		float eastmove = 0.0f;
		float westmove = 0.0f;
		float upmove = 0.0f;

		uint frame_msec = 0;
		int old_frame_msec = 0;
		
		bool groundPlane = false;
	};
};