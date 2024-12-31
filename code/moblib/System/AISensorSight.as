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
			const float half = info.sightRange / 2;
			switch ( mob.GetDirection() ) {
			case TheNomad::GameSystem::DirType::North:
				mins.x = origin.x - half;
				mins.y = origin.y - info.sightRange;

				maxs.x = origin.x + half;
				maxs.y = origin.y;
				break;
			/*
			case TheNomad::GameSystem::DirType::NorthEast:
				mins.x = origin.x;
				mins.y = origin.y - info.sightRange;

				maxs.x = origin.x + info.sightRange;
				maxs.y = origin.y;
				break;
			*/
			case TheNomad::GameSystem::DirType::East:
				mins.x = origin.x;
				mins.y = origin.y - half;

				maxs.x = origin.x + info.sightRange;
				maxs.y = origin.y + half;
				break;
			/*
			case TheNomad::GameSystem::DirType::SouthEast:
				mins.x = origin.x;
				mins.y = origin.y;

				maxs.x = origin.x + info.sightRange;
				maxs.y = origin.y + info.sightRange;
				break;
			*/
			case TheNomad::GameSystem::DirType::South:
				mins.x = origin.x - half;
				mins.y = origin.y;

				maxs.x = origin.x + half;
				maxs.y = origin.y + info.sightRange;
				break;
			/*
			case TheNomad::GameSystem::DirType::SouthWest:
				mins.x = origin.x + info.sightRange;
				mins.y = origin.y - half;

				maxs.x = origin.x;
				maxs.y = origin.y + half;
				break;
			*/
			case TheNomad::GameSystem::DirType::West:
				mins.x = origin.x - info.sightRange;
				mins.y = origin.y - half;

				maxs.x = origin.x;
				maxs.y = origin.y - half;
				break;
			/*
			case TheNomad::GameSystem::DirType::NorthWest:
				mins.x = origin.x - ;
				mins.y = origin.y - info.sightRange;

				maxs.x = origin.x;
				maxs.y = origin.y;
				break;
			*/
			};
			
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
			
			//
			// make sure that the line of sight isn't obstructed
			//
			TheNomad::GameSystem::RayCast ray;

			ray.m_nLength = info.sightRange;
			ray.m_Start = origin;
			ray.m_nAngle = TheNomad::Util::Dir2Angle( mob.GetDirection() );
			ray.m_nOwner = mob.GetEntityNum();

			ray.Cast( ent.GetOrigin() );
			
			if ( ray.m_nEntityNumber == ENTITYNUM_INVALID || ray.m_nEntityNumber == ENTITYNUM_WALL ) {
				return false;
			}
			else if ( ray.m_nEntityNumber >= TheNomad::SGame::EntityManager.NumEntities() ) {
				GameError( "MobObject::SightCheck: ray entity number is out of range (" + ray.m_nEntityNumber + ")"  );
			}

//			mob.SetTarget( @TheNomad::SGame::EntityManager.GetEntityForNum( ray.m_nEntityNumber ) );
			
			return true;
		}
	};
};