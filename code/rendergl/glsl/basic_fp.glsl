#if !defined(GLSL_LEGACY)
out vec4 a_Color;
#endif

in vec2 v_TexCoord;
in vec3 v_FragPos;
in vec4 v_Color;

uniform sampler2D u_DiffuseMap;
uniform float u_AmbientIntensity = float(0.0);
uniform vec3 u_AmbientColor = vec3(0.0);

//uniform sampler2D u_ColorMap;

#if defined(USE_NORMALMAP)
uniform sampler2D u_NormalMap;
#endif

#if defined(USE_SPECULARMAP)
uniform sampler2D u_SpecularMap;
#endif

#if 0
void applyLighting() {
    vec3 ambient = u_AmbientColor + u_AmbientIntensity;

    for (int i = 0; i < u_numLights; i++) {
        float diffuse = 0.0;
        mat4 model = mat4(1.0);

        //
        // using world position will give the lighting a more
        // retro and pixel-game (8-bit) style look because
        // individual tiles will be lit up
        //

        float dist = distance(lights[i].origin, v_WorldPos);
        vec3 color = lights[i].color + lights[i].intensity.x;
        float range = lights[i].intensity.y;

        if (dist <= range) {
            diffuse = 1.0 - abs(dist / lights[i].intensity.x);
        }

        a_Color *= vec4(min(a_Color.rgb * ((lights[i].color * diffuse) + u_AmbientColor), a_Color.rgb), 1.0);
    }
}
#endif

void main() {
    vec4 color = v_Color;
    color = vec4(1.0);
    a_Color = texture2D( u_DiffuseMap, v_TexCoord.st ) * color;
}
