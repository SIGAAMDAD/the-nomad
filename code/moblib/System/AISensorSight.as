namespace moblib {
	class AISensorSight {
		AISensorSight() {
		}
		
		bool DoCheck( TheNomad::SGame::MobObject@ mob ) const {
			vec3 delta, pos, p, target;
			TheNomad::SGame::EntityObject@ ent = null;
			const TheNomad::SGame::InfoSystem::MobInfo@ info = @mob.GetMobInfo();

			const float angle = mob.GetAngle();
			const float cosine = cos( info.sightRadius );
			const vec3 origin = mob.GetOrigin();
			delta.x = cos( angle );
			delta.y = sin( angle );
			delta.z = 0.0f;

			TheNomad::SGame::EntityObject@ activeEnts = @TheNomad::SGame::EntityManager.GetActiveEnts();
			for ( @ent = @activeEnts.m_Next; @ent !is @activeEnts; @ent = @ent.m_Next ) {
				target = ent.GetOrigin();
				if ( ent.GetType() == TheNomad::GameSystem::EntityType::Playr
					&& TheNomad::Util::Distance( target, origin ) < info.sightRange )
				{
					pos = target - origin;
					p = TheNomad::Util::VectorNormalize( pos );
					if ( TheNomad::Util::DotProduct( pos, delta ) > cosine ) {
						continue;
					}
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
			ray.m_nEntityNumber = ENTITYNUM_INVALID;
			ray.m_nAngle = mob.GetAngle();

			ray.Cast();
			
			if ( ray.m_nEntityNumber == ENTITYNUM_INVALID || ray.m_nEntityNumber == ENTITYNUM_WALL ) {
				return false;
			} else if ( ray.m_nEntityNumber >= TheNomad::SGame::EntityManager.NumEntities() ) {
				GameError( "MobObject::SightCheck: ray entity number is out of range (" + ray.m_nEntityNumber + ")"  );
			}
			
			mob.SetTarget( @TheNomad::SGame::EntityManager.GetEntityForNum( ray.m_nEntityNumber ) );

			return true;
		}
	};
};