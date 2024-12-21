#include "SGame/SpriteSheet.as"

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

		void Spawn( const vec3& in origin, const vec3& in velocity, uint lifeTime, int hShader, const vec2& in scale = vec2( 1.5f ),
			bool bGravity = false, TheNomad::SGame::SpriteSheet@ SpriteSheet = null,
			const uvec2& in nSpriteOffset = uvec2( 0 ) )
		{
			m_Origin = origin;
			m_Velocity = velocity;
			m_nEndTime = TheNomad::GameSystem::GameTic + lifeTime;
			m_hShader = hShader;
			@m_SpriteSheet = @SpriteSheet;
			m_nScale = scale;
			m_bGravity = bGravity;
			m_nRotation = 0.0f;
			m_nSpriteOffset = nSpriteOffset;
			m_EffectAnimation = TheNomad::SGame::Animation();
		}

		void RunTic() {
			if ( Util::Distance( TheNomad::SGame::EntityManager.GetActivePlayer().GetOrigin(), m_Origin ) > 16.0f ) {
				return;
			}

			m_Origin.x += ( m_Velocity.x * TheNomad::GameSystem::DeltaTic );
			m_Origin.y += ( m_Velocity.y * TheNomad::GameSystem::DeltaTic );
			m_Origin.z += ( m_Velocity.z * TheNomad::GameSystem::DeltaTic );

			if ( m_bGravity ) {
				m_Velocity.y = 0.01f;
				m_Velocity.z = Util::Clamp( m_Velocity.z - TheNomad::SGame::sgame_Gravity.GetFloat(), -0.4f, m_Velocity.z );
				if ( m_Origin.z < 0.0f ) {
					m_Velocity.z = 0.0f;
					m_Origin.z = 0.0f;
				}
			}

			TheNomad::Engine::Renderer::RenderEntity refEntity;
			refEntity.rotation = m_nRotation;
			refEntity.origin = m_Origin;
			refEntity.scale = m_nScale;
			if ( @m_SpriteSheet !is null ) {
				m_EffectAnimation.Run();

				refEntity.sheetNum = m_SpriteSheet.GetShader();
				refEntity.spriteId = m_nSpriteOffset.y * m_SpriteSheet.GetSpriteCountX() + m_nSpriteOffset.x + m_EffectAnimation.GetFrame();
			} else {
				refEntity.sheetNum = -1;
				refEntity.spriteId = m_hShader;
			}
			refEntity.Draw();
		}

		vec3 m_Origin = vec3( 0.0f );
		vec3 m_Velocity = vec3( 0.0f );

		vec2 m_nScale = vec2( 1.0f );
		uvec2 m_nSpriteOffset = uvec2( 0 );

		TheNomad::SGame::SpriteSheet@ m_SpriteSheet = null;
		LocalEntity@ m_Next = null;
		LocalEntity@ m_Prev = null;
		
		TheNomad::SGame::Animation m_EffectAnimation;

		uint m_nEndTime = 0;

		float m_nRotation = 0.0f;
		int m_hShader = FS_INVALID_HANDLE;
		bool m_bGravity = false;
	};
};