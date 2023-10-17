
attribute vec3 a_Position;
attribute vec2 a_TexCoords;
attribute vec3 a_Color;
attribute float a_Alpha;

varying vec3 v_Position;
varying vec2 v_TexCoords;
varying vec3 v_Color;
varying vec3 v_Normal;
varying float v_Alpha;
varying vec3 v_FragPos;

uniform mat4 u_ModelViewProjection;

void main()
{
    v_Position = a_Position;
    v_TexCoords = a_TexCoords;
    v_Color = a_Color;
    v_Alpha = v_Alpha;

    gl_Position = u_ModelViewProjection * vec4(a_Position, 1.0);
}