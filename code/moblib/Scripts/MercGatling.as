#include "moblib/MobScript.as"

namespace moblib {
	class MercGatling : MobScript {
		MercGatling() {
		}
		
		//
		// TargetBehindCover:
		// returns true if there's something obstructing line of sight or bullets
		// between the attacker and the target
		//
		// NOTE: could probably just be a utility function for all mobs
		//
		private bool TargetBehindCover( TheNomad::SGame::EntityObject@ target ) const {
			TheNomad::SGame::EntityObject@ attacker = @m_EntityData;
			const vec3& origin = attacker.GetOrigin();
			const vec3& cover = target.GetOrigin();
			
			TheNomad::GameSystem::RayCast ray;
			
			ray.m_Start = attacker.GetOrigin();
			ray.m_nLength = TheNomad::Util::Distance( origin, cover );
			ray.m_nAngle = atan2( origin.x - cover.x, origin.y - cover.y );
			ray.Cast();
			
			if ( ray.m_nEntityNumber == ENTITYNUM_WALL ) {
				return true;
			}
			return false;
		}
		
		
		
		private bool m_bSentryMode = false;
	};
};