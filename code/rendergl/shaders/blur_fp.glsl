#if !defined(GLSL_LEGACY)
layout( location = 0 ) out vec4 a_Color;
#if defined(USE_BLOOM) && defined(USE_HDR)
layout( location = 1 ) out vec4 a_BrightColor;
#endif
#endif

in vec2 v_TexCoords;

uniform sampler2D u_TextureMap;
uniform bool u_Horizontal;
uniform float u_BlurWeights[5] = float[]( 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 );

void main() {
    const vec2 tex_offset = 1.0 / textureSize( u_TextureMap, 0 ); // gets size of single texel
    const vec3 result = texture2D( u_TextureMap, v_TexCoords ).rgb * u_BlurWeights[0];
    
    if ( horizontal ) {
        for ( int i = 1; i < 5; ++i ) {
           result += texture2D( u_TextureMap, v_TexCoords + vec2( tex_offset.x * i, 0.0 ) ).rgb * u_BlurWeights[i];
           result += texture2D( u_TextureMap, v_TexCoords - vec2( tex_offset.x * i, 0.0 ) ).rgb * u_BlurWeights[i];
        }
    }
    else {
        for ( int i = 1; i < 5; ++i ) {
            result += texture2D( u_TextureMap, v_TexCoords + vec2( 0.0, tex_offset.y * i ) ).rgb * u_BlurWeights[i];
            result += texture2D( u_TextureMap, v_TexCoords - vec2( 0.0, tex_offset.y * i ) ).rgb * u_BlurWeights[i];
        }
    }
    FragColor = vec4(result, 1.0);
}
