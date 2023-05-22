#type vertex
#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_texture_multisample : enable
#extension GL_NV_uniform_buffer_std430_layout : enable

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoords;

out vec4 v_Color;
out vec2 v_TexCoords;
out vec3 v_Position;

uniform mat4 u_MVP = mat4(1.0);
uniform mat4 u_ViewProjection;

void main()
{
    v_TexCoords = a_TexCoords;
    v_Position = a_Position;
    v_Color = a_Color;
    gl_Position = u_ViewProjection * u_MVP * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_texture_multisample : enable
#extension GL_NV_uniform_buffer_std430_layout : enable

layout(location = 0) out vec4 a_Color;

in vec4 v_Color;
in vec2 v_TexCoords;
in vec3 v_Position;

uniform vec3 u_LightPos = vec3(0.0);
uniform float u_SurroundingLight = float(1.0);
uniform sampler2D u_Texture;

float lighting()
{
    return u_SurroundingLight / distance(u_LightPos, vec3(0.0)); // use for the future
}

void main()
{
    a_Color = texture2D(u_Texture, v_TexCoords);
}