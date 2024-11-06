#include "SGame/PlayerSystem/ScreenShake.as"

namespace TheNomad::SGame {
	//
	// DamageSystem:
	// manages special damage effects, haptic feedback, and screen shake
	// makes combat feel punchy
	//
	class DamageSystem {
		DamageSystem() {
		}

		void LoadResources() {
			m_hImpactShader = TheNomad::Engine::ResourceCache.GetShader( "gfx/effects/impact" );
			m_hBulletMarkShader = TheNomad::Engine::ResourceCache.GetShader( "gfx/effects/bullet_mark" );
		}

		void AddExplosion( const vec3& in origin, float range, float weight ) {
			
		}

		private int m_hImpactShader = -1;
		private int m_hBulletMarkShader = -1;
		private int m_hSmokePuffShader = -1;
	};
};