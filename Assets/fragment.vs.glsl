#version 420 core

layout(binding = 0) uniform sampler2DShadow shadow_tex;
layout(binding = 1) uniform sampler2D tex;
out vec4 color;

uniform int type;

in VS_OUT
{
    vec4 shadow_coord;
    vec3 N;
    vec3 L;
    vec3 V;
	vec2 texcoord;
} fs_in;

// Material properties
uniform vec3 diffuse_albedo = vec3(0.8, 0.8, 0.8);
uniform vec3 specular_albedo = vec3(0.7);
uniform float specular_power = 300.0;
uniform bool full_shading = true;
uniform float Ia = 0.3;
uniform float Id = 0.8;
uniform float Is = 0.4;
uniform float shadowrate = 0.6;
void main(void)
{
    // Normalize the incoming N, L and V vectors
    vec3 N = normalize(fs_in.N);
    vec3 L = normalize(fs_in.L);
    vec3 V = normalize(fs_in.V);
	vec3 H = normalize(L + V);

	vec3 texcolor = texture(tex, fs_in.texcoord).rgb;
	vec3 ambient = texcolor * Ia;
    // Compute the diffuse and specular components for each fragment
    vec3 diffuse = max(dot(N, L), 0.0) * texcolor * Id;
    vec3 specular = pow(max(dot(N, H), 0.0), specular_power) * specular_albedo * Is;
    color = textureProj(shadow_tex, fs_in.shadow_coord) * vec4(ambient + diffuse + specular, 1.0)*shadowrate + vec4(ambient + diffuse + specular, 1.0)*(1-shadowrate);
}