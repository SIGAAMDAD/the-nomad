#include "SGame/PlayerSystem/GoreManager.as"

namespace TheNomad::Engine::Renderer {
	enum EffectType {
		Effect_None = 0,
		Effect_Blood,
		Effect_Dust,
		Effect_WaterRipple,
	};

	class LocalEntity {
		LocalEntity() {
		}

		void Spawn( const vec3& in origin, const vec3& in velocity, uint lifeTime, int hShader, float scale = 1.5f,
			bool bGravity = false, float grow = 0.0f )
		{
			m_Origin = origin;
			m_Velocity = velocity;
			m_nStartTime = TheNomad::GameSystem::GameManager.GetGameTic();
			m_nLifeTime = lifeTime;
			m_hShader = hShader;
			m_nScale = scale;
			m_bGravity = bGravity;
			m_nRotation = 0.0f;
			m_nGrowAmount = grow;
			m_nType = EffectType::Effect_None;
		}

		void RunTic() {

			const uint frame_msec = TheNomad::GameSystem::GameManager.GetGameTic() - m_nLastFrameTime;
			m_nLastFrameTime = TheNomad::GameSystem::GameManager.GetGameTic();

			m_Origin += m_Velocity;
			m_nScale += m_nGrowAmount * Util::Clamp( float( TheNomad::GameSystem::GameManager.GetGameTic() ) / float( frame_msec ),
				float( 0 ), float( 1 ) );

			if ( m_bGravity ) {
				m_Velocity.y = 0.01f;
				m_Velocity.z = Util::Clamp( m_Velocity.z - TheNomad::SGame::sgame_Gravity.GetFloat(), -0.2f, m_Velocity.z );
				if ( m_Origin.z < 0.0f ) {
					m_Velocity.z = 0.0f;
					m_Origin.z = 0.0f;
				}
			}

			TheNomad::Engine::Renderer::RenderEntity refEntity;
			refEntity.rotation = m_nRotation;
			if ( refEntity.rotation != 0.0f ) {
				refEntity.rotation += Util::RAD2DEG( Util::PRandom() & 16 );
			}

			refEntity.origin = m_Origin;
			refEntity.scale = m_nScale;
			refEntity.sheetNum = -1;
			refEntity.spriteId = m_hShader;
			refEntity.Draw();
		}

		vec3 m_Origin = vec3( 0.0f );
		vec3 m_Velocity = vec3( 0.0f );
		LocalEntity@ m_Next = null;
		LocalEntity@ m_Prev = null;
		uint m_nStartTime = 0;
		uint m_nLifeTime = 0;
		float m_nRotation = 0.0f;
		int m_hShader = FS_INVALID_HANDLE;
		float m_nScale = 1.0f;
		float m_nGrowAmount = 0.0f;
		private uint m_nLastFrameTime = 0;
		EffectType m_nType = EffectType::Effect_None;
		bool m_bGravity = false;
	};
};