namespace TheNomad::Engine::Renderer {
    class RenderEntity {
        RenderEntity() {
        }

		void Draw() const {
			AddEntityToScene( sheetNum, spriteId, renderfx, lightingOrigin, origin, frame, flags, color, shaderTexCoord, shaderTime,
				rotation, scale );
		}

        // texturing
	    int sheetNum;  // sprite sheet index
    	int spriteId;  // sprite id

    	int renderfx;

	    vec3 lightingOrigin; // RF_LIGHTING_ORIGIN
	    vec3 origin;
	    uint64 frame;

	    uint32 flags;

	    // misc
	    uint32		color;
	    vec2		shaderTexCoord;	// texture coordinates used by tcMod entity modifiers

    	// subtracted from refdef time to control effect start times
    	float		shaderTime;

    	// extra sprite information
//    	float		radius;
    	float		rotation;
        float       scale;
    };
};