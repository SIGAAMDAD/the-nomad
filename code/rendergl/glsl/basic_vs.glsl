in vec3 a_Position;
in vec2 a_TexCoords;
in vec4 a_Normal;
in vec4 a_Color;

out vec3 v_Position;
out vec2 v_TexCoords;
out vec4 v_Color;
out vec4 v_Normal;
out vec3 v_FragPos;

uniform mat4 u_ModelViewProjection;

void main()
{
    mat4 model = mat4(1.0);

    v_Position = a_Position;
    v_TexCoords = a_TexCoords;
    v_Color = a_Color;

    v_FragPos = vec3(model * vec4(a_Position, 1.0));

    gl_Position = u_ModelViewProjection * vec4(a_Position, 1.0);
}