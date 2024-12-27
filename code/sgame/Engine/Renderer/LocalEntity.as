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
			m_Origin = vec2( origin.x, origin.y + origin.z );
			m_Velocity = vec2( velocity.x, velocity.y + velocity.z );
			m_nEndTime = TheNomad::GameSystem::GameTic + lifeTime;
			m_hShader = hShader;
			m_nScale = scale;
			m_bGravity = bGravity;
			m_nRotation = 0.0f;
			if ( @SpriteSheet !is null ) {
				m_bSpriteSheet = true;
				m_hShader = SpriteSheet.GetShader();
				m_nSpriteOffset = nSpriteOffset.y * SpriteSheet.GetSpriteCountX() + nSpriteOffset.x;
			} else {
				m_bSpriteSheet = false;
			}
			m_EffectAnimation = TheNomad::SGame::Animation();
		}

		void RunTic() {
			const vec3 origin = vec3( m_Origin.x, m_Origin.y, 0.0f );
			if ( Util::Distance( TheNomad::SGame::EntityManager.GetActivePlayer().GetOrigin(), origin ) > 16.0f ) {
				return;
			}

			if ( m_Velocity != Vec2Origin ) {
				m_Origin.x += ( m_Velocity.x * TheNomad::GameSystem::DeltaTic );
				m_Origin.y += ( m_Velocity.y * TheNomad::GameSystem::DeltaTic );
			}
			if ( m_bGravity ) {
				m_Velocity.y = Util::Clamp( m_Velocity.y - TheNomad::SGame::sgame_Gravity.GetFloat(), -0.4f, m_Velocity.y );
				if ( m_Origin.y < 0.0f ) {
					m_Velocity.y = 0.0f;
					m_Origin.y = 0.0f;
				}
			}

			TheNomad::Engine::Renderer::RenderEntity refEntity;

			refEntity.rotation = m_nRotation;
			refEntity.origin = origin;
			refEntity.scale = m_nScale;
			if ( m_bSpriteSheet ) {
				m_EffectAnimation.Run();

				refEntity.sheetNum = m_hShader;
				refEntity.spriteId = m_nSpriteOffset + m_EffectAnimation.GetFrame();
			} else {
				refEntity.sheetNum = -1;
				refEntity.spriteId = m_hShader;
			}
			refEntity.Draw();
		}

		vec2 m_Origin = vec2( 0.0f );
		vec2 m_Velocity = vec2( 0.0f );

		vec2 m_nScale = vec2( 1.0f );

		LocalEntity@ m_Next = null;
		LocalEntity@ m_Prev = null;
		
		TheNomad::SGame::Animation m_EffectAnimation;

		uint m_nEndTime = 0;
		uint m_nSpriteOffset = 0;

		float m_nRotation = 0.0f;
		int m_hShader = FS_INVALID_HANDLE;
		bool m_bSpriteSheet = false;
		bool m_bGravity = false;
	};
};