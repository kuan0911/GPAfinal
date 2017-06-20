#version 410

layout(location = 0) out vec4 fragColor;

uniform samplerCube tex_cubemap;
uniform vec3 light_pos;

in VS_OUT
{                                              
	vec3    tc;                           
} fs_in; 


void main()
{
    if(light_pos.y<0.0) {
	    fragColor = texture(tex_cubemap, fs_in.tc)*0.3;
	}else {
	    fragColor = texture(tex_cubemap, fs_in.tc);
	}
	
}