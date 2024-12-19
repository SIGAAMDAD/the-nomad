#include "SGame/SpriteSheet.as"
#include "SGame/EntityState.as"

namespace TheNomad::Engine::Renderer {
	class RenderEntity {
		RenderEntity() {
		}

		void Draw() const {
			AddEntityToScene( sheetNum, spriteId, renderfx, vec3( 0.0f ), origin, 0, flags, color, shaderTexCoord, 0.0f,
				radius, rotation, scale );
		}

		vec3 origin = vec3( 0.0f );

		// texturing
		int sheetNum = 0;  // sprite sheet index
		int spriteId = 0;  // sprite id
		int renderfx = 0;

		uint		flags = 0;

		// misc
		uint		color;
		vec2		shaderTexCoord = 0.0f; // texture coordinates used by tcMod entity modifiers

		// extra sprite information
		float		radius = 0.0f;
		float		rotation = 0.0f;
		vec2		scale = vec2( 0.0f );
	};

	uint GetSpriteId( TheNomad::SGame::SpriteSheet@ sheet, TheNomad::SGame::EntityState@ state ) {
		const uint offset = state.GetSpriteOffset().y * sheet.GetSpriteCountX() + state.GetSpriteOffset().x;
		return offset + state.GetAnimation().GetFrame();
	}
	vec2 GetFacing( int facing, const vec2& in size = vec2( 2.0f ) ) {
		switch ( facing ) {
		case TheNomad::SGame::FACING_LEFT:	return vec2( -size.x, size.y );
		case TheNomad::SGame::FACING_RIGHT:	return vec2(  size.x, size.y );
		default:
			break;
		};
		GameError( "GetFacing: bad facing " + facing );
		return Vec2Origin;
	}
};