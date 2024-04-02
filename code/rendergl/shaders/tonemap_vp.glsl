in vec3 a_Position;
in vec4 a_TexCoord0;

uniform mat4 u_ModelViewProjection;
uniform vec3 u_ToneMinAvgMaxLinear;

out vec2 v_TexCoords;
out float v_InvWhite;

float FilmicTonemap(float x)
{
	const float SS  = 0.22; // Shoulder Strength
	const float LS  = 0.30; // Linear Strength
	const float LA  = 0.10; // Linear Angle
	const float TS  = 0.20; // Toe Strength
	const float TAN = 0.01; // Toe Angle Numerator
	const float TAD = 0.30; // Toe Angle Denominator

	return ((x*(SS*x+LA*LS)+TS*TAN)/(x*(SS*x+LS)+TS*TAD)) - TAN/TAD;
}

void main() {
	gl_Position = u_ModelViewProjection * vec4(a_Position, 1.0);
	v_TexCoords = a_TexCoord0.st;
	v_InvWhite = 1.0 / FilmicTonemap(u_ToneMinAvgMaxLinear.z - u_ToneMinAvgMaxLinear.x);
}
