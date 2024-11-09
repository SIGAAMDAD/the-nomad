#include "SGame/SpriteSheet.as"
#include "SGame/EntityState.as"

namespace TheNomad::Engine::Renderer {
	class RenderEntity {
		RenderEntity() {
		}

		void Draw() const {
			AddEntityToScene( sheetNum, spriteId, renderfx, lightingOrigin, origin, frame, flags, color, shaderTexCoord, shaderTime,
				radius, rotation, scale );
		}

		// texturing
		int sheetNum = 0;  // sprite sheet index
		int spriteId = 0;  // sprite id

		int renderfx = 0;

		vec3 lightingOrigin = vec3( 0.0f ); // RF_LIGHTING_ORIGIN
		vec3 origin = vec3( 0.0f );
		uint64 frame = 0;

		uint32 flags = 0;

		// misc
		uint32		color;
		vec2		shaderTexCoord = 0.0f; // texture coordinates used by tcMod entity modifiers

		// subtracted from refdef time to control effect start times
		float		shaderTime = 0.0f;

		// extra sprite information
		float		radius = 0.0f;
		float		rotation = 0.0f;
		float		scale = 0.0f;
	};

	uint GetSpriteId( TheNomad::SGame::SpriteSheet@ sheet, TheNomad::SGame::EntityState@ state ) {
		const uint offset = state.GetSpriteOffset().y * sheet.GetSpriteCountX() + state.GetSpriteOffset().x;
		return offset + state.GetAnimation().GetFrame();
	}
};