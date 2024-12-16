in vec2 a_Position;
in vec2 a_TexCoords;
in vec2 a_WorldPos;
in vec4 a_Color;

out vec2 v_TexCoords;
out vec4 v_Color;
out vec2 v_WorldPos;
out vec3 v_LightingColor;
out vec2 v_Position;

uniform mat4 u_ModelViewProjection;
uniform vec4 u_BaseColor;
uniform vec4 u_VertColor;
uniform int u_ColorGen;

uniform int u_LightingQuality;
uniform vec3 u_AmbientColor;
uniform int u_NumLights;

TEXTURE2D u_DiffuseMap;

struct Light {
	vec4 color;
	uvec2 origin;
	float brightness;
	float range;
	float linear;
	float quadratic;
	float constant;
	int type;
};

#if defined(EXPLICIT_BUFFER_LOCATIONS)
layout( std140, binding = 0 ) readonly buffer u_LightBuffer {
	Light u_LightData[];
};
#else
layout( std140 ) uniform u_LightBuffer {
	Light u_LightData[ MAX_LIGHTS ];
};
#endif

vec3 CalcPointLight( Light light ) {
	vec3 diffuse = v_LightingColor.rgb;
	float dist = distance( v_WorldPos, light.origin );
	float diff = 0.0;
	float range = light.range;
	float attenuation = 1.0;

	if ( dist <= light.range ) {
		diff = 1.0 - abs( dist / light.range );
	}
	diff += light.brightness;
	diffuse = min( diff * ( diffuse + vec3( light.color ) ), diffuse );

	range = light.range * light.brightness;

	attenuation = ( light.constant + light.linear + light.quadratic * ( light.range * light.range ) );
	
	return ( diffuse * attenuation );
}

void main() {
	if ( u_ColorGen == CGEN_VERTEX ) {
		v_Color = vec4( 1.0 );
	} else {
		v_Color = u_VertColor * a_Color + u_BaseColor;
	}
	v_WorldPos = a_WorldPos;
	v_TexCoords = a_TexCoords;

	if ( u_LightingQuality == QUALITY_LOW ) {
		v_LightingColor = vec3( 1.0 );
		for ( int i = 0; i < u_NumLights; i++ ) {
			v_LightingColor += CalcPointLight( u_LightData[ i ] );
		}
		v_LightingColor *= u_AmbientColor;
	}

    gl_Position = u_ModelViewProjection * vec4( a_Position, 0.0, 1.0 );
	v_Position.xy = gl_Position.xy;
}