#version 410 core
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 shadow_matrix;
// Position of light
uniform vec3 light_pos;
layout (location = 0) in vec4 position;
layout (location = 1) in vec2 iv2tex_coord;
layout (location = 2) in vec3 normal;

uniform int type;

out VS_OUT
{
    vec4 shadow_coord;
    vec3 N;
    vec3 L;
    vec3 V;
	vec2 texcoord;
} vs_out;

void main(void)
{
    // Calculate view-space coordinate
    vec4 P = mv_matrix * position;
    // Calculate normal in view-space
    vs_out.N = mat3(mv_matrix) * normal;
    // Calculate light vector
    vs_out.L = light_pos - P.xyz;
    // Calculate view vector
    vs_out.V = -P.xyz;
    // Light-space coordinates
    vs_out.shadow_coord = shadow_matrix * position;
   // Calculate the clip-space position of each vertex
    gl_Position = proj_matrix * P;
	vs_out.texcoord = iv2tex_coord;
}