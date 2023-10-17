
varying vec2 v_TexCoords;
varying vec3 v_Position;
varying vec3 v_Color;
varying float v_Alpha;

uniform sampler2D u_DiffuseMap;

void main()
{
    gl_FragColor = texture2D(u_DiffuseMap, v_TexCoords);
}