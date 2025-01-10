namespace moblib {
	class AISensorSight {
		AISensorSight() {
		}
		
		bool DoCheck( TheNomad::SGame::MobObject@ mob ) const {
//			TheNomad::SGame::EntityObject@ ent = null;
			const TheNomad::SGame::InfoSystem::MobInfo@ info = @mob.GetMobInfo();
			const vec3 origin = mob.GetOrigin();
			TheNomad::GameSystem::RayCast ray;

			const vec3 target = TheNomad::SGame::EntityManager.GetActivePlayer().GetOrigin();
			if ( @mob.GetTarget() is null ) {
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

				TheNomad::GameSystem::BBox bounds;
				bounds.m_nMins = mins;
				bounds.m_nMaxs = maxs;

				/* NOTE: this will really only matter once we implement factions
				TheNomad::SGame::EntityObject@ activeEnts = @TheNomad::SGame::EntityManager.GetActiveEnts();
				if ( @ent is @activeEnts ) {
					return false;
				}
				*/
				if ( !bounds.IntersectsPoint( target ) ) {
					return false;
				}
				
				//
				// make sure that the line of sight isn't obstructed
				//
				ray.m_nLength = info.sightRange;
				ray.m_Start = origin;
				ray.m_nAngle = mob.GetAngle();
				ray.m_nOwner = mob.GetEntityNum();

				if ( ray.m_nAngle < 0.0f ) {
					const float tmp = TheNomad::Util::RAD2DEG( ray.m_nAngle ) + 360.0f;
					ray.m_nAngle = TheNomad::Util::DEG2RAD( tmp );
				}

				ray.Cast( target );

				if ( ray.m_nEntityNumber != TheNomad::SGame::EntityManager.GetActivePlayer().GetEntityNum() ) {
					return false;
				} else if ( ray.m_nEntityNumber >= TheNomad::SGame::EntityManager.NumEntities() ) {
					GameError( "MobObject::SightCheck: ray entity number is out of range (" + ray.m_nEntityNumber + ")"  );
				}
			}
			else {
				mob.SetAngle( -atan2( origin.y - target.y, target.x - origin.x ) );

				ray.m_nLength = info.sightRange;
				ray.m_Start = origin;
				ray.m_nAngle = mob.GetAngle();
				ray.m_nOwner = mob.GetEntityNum();

				ray.Cast();

				if ( ray.m_nEntityNumber != TheNomad::SGame::EntityManager.GetActivePlayer().GetEntityNum() ) {
					return false;
				} else if ( ray.m_nEntityNumber >= TheNomad::SGame::EntityManager.NumEntities() ) {
					GameError( "MobObject::SightCheck: ray entity number is out of range (" + ray.m_nEntityNumber + ")"  );
				}
			}
			
			mob.SetTarget( @TheNomad::SGame::EntityManager.GetActivePlayer() );
			
			return true;
		}
	};
};