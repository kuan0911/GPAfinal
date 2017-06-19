#version 410

layout(location = 0) out vec4 fragColor;

uniform mat4 um4mv;
uniform mat4 um4p;
uniform samplerCube tex_cubemap;

in VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 H; // eye space halfway vector
    vec2 texcoord;
	vec3 normal;                           
    vec3 view; 
} vertexData;

uniform sampler2D tex;

void main()
{
	vec3 r = reflect(vertexData.view, normalize(vertexData.normal)); 
	fragColor = texture(tex_cubemap, r) * vec4(0.95, 0.80, 0.45, 1.0);
    //vec3 texColor = texture(tex,vertexData.texcoord).rgb;
    //fragColor = vec4(texColor, 1.0);	
	//fragColor = vec4(vertexData.N, 1.0);	
}