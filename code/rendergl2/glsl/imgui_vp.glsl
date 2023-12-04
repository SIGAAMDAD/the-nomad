in vec3 a_Position;
in vec2 a_TexCoord;
in vec4 a_Color;

uniform mat4 u_ModelViewProjection;

out vec2 v_TexCoord;
out vec4 v_Color;

void main() {
    v_TexCoord = a_TexCoord;
    v_Color = a_Color;
    gl_Position = u_ModelViewProjection * vec4(a_Position.xy, 0, 1);
}