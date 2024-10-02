out vec4 a_Color;

in vec4 v_Position;
in vec4 v_TexCoord0;
in vec4 v_TexCoord1;
in vec4 v_TexCoord2;
in vec4 v_TexCoord3;
in vec4 v_TexCoord4;

TEXTURE2D u_DiffuseMap;

const vec3 dt = vec3( 1.0 );

void main() {
	vec3 c00 = texture2D( u_DiffuseMap, v_TexCoord1.zw ).xyz; 
	vec3 c10 = texture2D( u_DiffuseMap, v_TexCoord3.xy ).xyz;
	vec3 c20 = texture2D( u_DiffuseMap, v_TexCoord3.zw ).xyz;
	vec3 c01 = texture2D( u_DiffuseMap, v_TexCoord1.xy ).xyz;
	vec3 c11 = texture2D( u_DiffuseMap, v_TexCoord0.xy ).xyz;
	vec3 c21 = texture2D( u_DiffuseMap, v_TexCoord2.xy ).xyz;
	vec3 c02 = texture2D( u_DiffuseMap, v_TexCoord2.zw ).xyz;
	vec3 c12 = texture2D( u_DiffuseMap, v_TexCoord4.xy ).xyz;
	vec3 c22 = texture2D( u_DiffuseMap, v_TexCoord4.zw ).xyz;

	float d1 = dot( abs( c00 - c22 ), dt ) + 0.0001;
	float d2 = dot( abs( c20 - c02 ), dt ) + 0.0001;
	float hl = dot( abs( c01 - c21 ), dt ) + 0.0001;
	float vl = dot( abs( c10 - c12 ), dt ) + 0.0001;
   
	float k1 = 0.5 * ( hl + vl );
	float k2 = 0.5 * ( d1 + d2 );

	vec3 t1 = ( hl * ( c10 + c12 ) + vl * ( c01 + c21 ) + k1 * c11 ) / ( 2.5 * ( hl + vl ) );
	vec3 t2 = ( d1 * ( c20 + c02 ) + d2 * ( c00 + c22 ) + k2 * c11 ) / ( 2.5 * ( d1 + d2 ) );

	k1 = dot( abs( t1 - c11 ), dt ) + 0.0001;
	k2 = dot( abs( t2 - c11 ), dt ) + 0.0001;
   
	a_Color = vec4( ( k1 * t2 + k2 * t1 ) / ( k1 + k2 ), 1 );
}