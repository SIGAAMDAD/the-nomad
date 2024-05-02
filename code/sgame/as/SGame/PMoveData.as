namespace TheNomad::SGame {
    ///
	/// PMoveData
	/// a class to buffer user input per frame
	///
	class PMoveData {
		PMoveData() {
		}
		
		private void WalkMove( PlayrObject@ ent ) {
			vec3 vel;
			
			if ( !groundPlane ) {
				return;
			}
			
			vel = ent.GetVelocity();
			if ( northmove > 0 ) {
				vel.y -= northmove / sgame_BaseSpeed.GetFloat();
			}
			if ( southmove > 0 ) {
				vel.y += southmove / sgame_BaseSpeed.GetFloat();
			}
			if ( westmove > 0 ) {
				vel.x -= westmove / sgame_BaseSpeed.GetFloat();
			}
			if ( eastmove > 0 ) {
				vel.x += eastmove / sgame_BaseSpeed.GetFloat();
			}
			
			// clamp that shit, then apply friction
			for ( uint i = 0; i < 2; i++ ) {
				vel[i] = TheNomad::Util::Clamp( vel[i], 0.0f, sgame_MaxSpeed.GetFloat() );
				vel[i] -= ( sgame_GroundFriction.GetFloat() * TheNomad::GameSystem::GameManager.GetDeltaTics() );
			}
			
			ent.SetVelocity( vel );
		}
		
		private void AirMove() {
			
		}
		
		void RunTic() {
			PlayrObject@ obj;
			uvec2 mousePos;
			int screenWidth, screenHeight;
			float angle;
			TheNomad::GameSystem::DirType dir;
			
			@obj = @GetPlayerObject();
			mousePos = TheNomad::GameSystem::GameManager.GetMousePos();
			screenWidth = TheNomad::GameSystem::GameManager.GetGPUConfig().screenWidth;
			screenHeight = TheNomad::GameSystem::GameManager.GetGPUConfig().screenHeight;
			
			northmove = obj.key_MoveNorth.active ? obj.key_MoveNorth.msec / sgame_MaxFps.GetInt() : 0;
			southmove = obj.key_MoveSouth.active ? obj.key_MoveSouth.msec / sgame_MaxFps.GetInt() : 0;
			eastmove = obj.key_MoveEast.active ? obj.key_MoveEast.msec / sgame_MaxFps.GetInt() : 0;
			westmove = obj.key_MoveWest.active ? obj.key_MoveWest.msec / sgame_MaxFps.GetInt() : 0;
			upmove = obj.key_Jump.active ? obj.key_Jump.msec / sgame_MaxFps.GetInt() : 0;
			
			// set leg sprite
			if ( eastmove > westmove ) {
				obj.SetLegFacing( 0 );
			} else if ( westmove > eastmove ) {
				obj.SetLegFacing( 1 );
			}
			
			// set torso direction
			angle = atan2( ( screenWidth / 2 ) - mousePos.x, ( screenHeight / 2 ) - mousePos.y );
			dir = TheNomad::Util::Angle2Dir( angle );
			
			switch ( dir ) {
			case TheNomad::GameSystem::DirType::North:
			case TheNomad::GameSystem::DirType::South:
				break; // not implemented for now
			case TheNomad::GameSystem::DirType::NorthEast:
			case TheNomad::GameSystem::DirType::SouthEast:
			case TheNomad::GameSystem::DirType::East:
				obj.SetFacing( 0 );
				break;
			case TheNomad::GameSystem::DirType::NorthWest:
			case TheNomad::GameSystem::DirType::SouthWest:
			case TheNomad::GameSystem::DirType::West:
				obj.SetFacing( 1 );
				break;
			default:
				break;
			};
			
			if ( obj.key_Jump.active && obj.GetOrigin().z > 0.0f ) {
				obj.SetFlags( obj.GetFlags() | PF_DOUBLEJUMP );
			}
			
			groundPlane = upmove == 0;
		}
		
		uint northmove = 0;
		uint southmove = 0;
		uint eastmove = 0;
		uint westmove = 0;
		uint upmove = 0;
		
		bool groundPlane = false;
	};
};