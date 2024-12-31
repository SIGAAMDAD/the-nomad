namespace moblib {
	class AISensorSight {
		AISensorSight() {
		}
		
		bool DoCheck( TheNomad::SGame::MobObject@ mob ) const {
			vec3 delta, pos, p, target;
			TheNomad::SGame::EntityObject@ ent = null;
			const TheNomad::SGame::InfoSystem::MobInfo@ info = @mob.GetMobInfo();
			TheNomad::Engine::Physics::Bounds bounds;

			const vec3 origin = mob.GetOrigin();

			vec3 mins;
			vec3 maxs;

			// very hardcody but it WORKS
			switch ( mob.GetDirection() ) {
			case TheNomad::GameSystem::DirType::South:
				mins.x = origin.x - ( info.sightRange / 2 );
				mins.y = origin.y;

				maxs.x = origin.x + ( info.sightRange / 2 );
				maxs.y = origin.y + info.sightRange;
				break;
			case TheNomad::GameSystem::DirType::North:
				mins.x = origin.x - ( info.sightRange / 2 );
				mins.y = origin.y - info.sightRange;

				maxs.x = origin.x + ( info.sightRange / 2 );
				maxs.y = origin.y;
				break;
			};
			mins.x = origin.x - ( info.sightRange / 2 );
				mins.y = origin.y;

				maxs.x = origin.x + ( info.sightRange / 2 );
				maxs.y = origin.y + info.sightRange;
			
			ConsolePrint( "mins: " + mins.x + ", " + mins.y+ "\n" );
			ConsolePrint( "maxs: " + maxs.x + ", " + maxs.y+ "\n" );
			ConsolePrint( "sightRange: " + info.sightRange + "\n" );

			bounds.m_nMins = mins;
			bounds.m_nMaxs = maxs;

			TheNomad::SGame::EntityObject@ activeEnts = @TheNomad::SGame::EntityManager.GetActiveEnts();
			for ( @ent = @activeEnts.m_Next; @ent !is @activeEnts; @ent = @ent.m_Next ) {
				if ( bounds.IntersectsPoint( ent.GetOrigin() ) && ent.GetType() == TheNomad::GameSystem::EntityType::Playr ) {
					break;
				}
			}
			if ( @ent is @activeEnts ) {
				return false;
			}

			DebugPrint( "Target in range of sight.\n" );
			
			//
			// make sure that the line of sight isn't obstructed
			//
			TheNomad::GameSystem::RayCast ray;

			ray.m_nLength = info.sightRange;
			ray.m_Start = origin;
			ray.m_nEntityNumber = ENTITYNUM_INVALID;
			ray.m_nAngle = mob.GetAngle();

			ray.Cast();
			
			if ( ray.m_nEntityNumber == ENTITYNUM_INVALID || ray.m_nEntityNumber == ENTITYNUM_WALL ) {
				DebugPrint( "Target obscured.\n" );
				return false;
			} else if ( ray.m_nEntityNumber >= TheNomad::SGame::EntityManager.NumEntities() ) {
				GameError( "MobObject::SightCheck: ray entity number is out of range (" + ray.m_nEntityNumber + ")"  );
			}
			
			mob.SetTarget( @TheNomad::SGame::EntityManager.GetEntityForNum( ray.m_nEntityNumber ) );

			return true;
		}
	};
};