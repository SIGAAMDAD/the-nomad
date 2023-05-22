#type vertex
#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_texture_multisample : enable
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out vec2 v_TexCoords;

void main() 
{
    gl_Position = vec4(a_Position, 1.0);
    v_TexCoords = a_TexCoords;
}

#type fragment
#version 450 core
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_texture_multisample : enable

layout(location = 0) out vec4 a_Color;

in vec2 v_TexCoords;

uniform sampler2D u_ColorTexture;
uniform sampler2D u_ScreenTexture;
uniform float u_Gamma = float(0.50);

void main()
{
    a_Color = texture2D(u_ColorTexture, v_TexCoords) + texture2D(u_ScreenTexture, v_TexCoords);
    a_Color *= u_Gamma;
}