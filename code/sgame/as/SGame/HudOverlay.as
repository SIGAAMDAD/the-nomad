namespace TheNomad::SGame {
	//
	// HudOverlay: a hud element that doesn't use a spritesheet, most usually a fullscreen opaque image
	//
	class HudOverlay {
		HudOverlay() {
		}
		
		void Draw() const {
			
		}
		
		vec2 origin;
		vec2 size;
		int hShader;
	};
};