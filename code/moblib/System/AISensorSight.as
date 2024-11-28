namespace moblib {
	class AISensorSight {
		AISensorSight() {
		}
		
		bool DoCheck( TheNomad::SGame::MobObject@ mob ) const {
			TheNomad::GameSystem::RayCast ray;
			vec3 delta, pos, p;
			TheNomad::SGame::EntityObject@ ent = null;
			const TheNomad::SGame::InfoSystem::MobInfo@ info = cast<TheNomad::SGame::InfoSystem::MobInfo@>( @mob.GetInfo() );

			delta.x = cos( mob.GetAngle() );
			delta.y = sin( mob.GetAngle() );
			delta.z = 0.0f;
			
			pos = mob.GetTarget().GetOrigin() - mob.GetOrigin();
			
			p = TheNomad::Util::VectorNormalize( pos );
			if ( TheNomad::Util::DotProduct( pos, delta ) > cos( info.sightRadius ) ) {
				return false;
			}
			
			//
			// make sure that the line of sight isn't obstructed
			//
			ray.m_nLength = info.sightRange;
			ray.m_Start = mob.GetOrigin();
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