#version 410

layout(location = 0) out vec4 fragColor;

uniform samplerCube tex_cubemap;

in VS_OUT
{                                              
	vec3    tc;                           
} fs_in; 


void main()
{
	fragColor = texture(tex_cubemap, fs_in.tc);
}