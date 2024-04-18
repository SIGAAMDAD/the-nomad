namespace TheNomad::SGame {
	//
	// HudOverlay: a hud element that doesn't use a spritesheet, most usually a fullscreen opaque image
	//
	class HudOverlay {
		HudOverlay() {
		}
		
		void Draw() const {
//			Engine::Renderer::SetColor( color );
			Engine::Renderer::DrawImage( origin.x, origin.y, size.x, size.y, 0, 0, 1, 1, hShader );
		}
		
		vec4 color = vec4( 1.0f ); // default to white
		vec2 origin = vec2( 0.0f, 0.0f );
		vec2 size = vec2( 0.0f, 0.0f );
		int hShader = FS_INVALID_HANDLE;
	};
};