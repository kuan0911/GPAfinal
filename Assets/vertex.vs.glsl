#version 410

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec2 iv2tex_coord;
layout(location = 2) in vec3 iv3normal;

uniform mat4 um4mv;
uniform mat4 um4p;
uniform int type;
uniform float time;

out VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 H; // eye space halfway vector
    vec2 texcoord;
} vertexData;

void main()
{
	if(type==0){
		gl_Position = um4p * um4mv * vec4(iv3vertex, 1.0);
		vertexData.texcoord = iv2tex_coord;
	}
	else if(type==1){
		vec4 newVertex = vec4(iv3vertex, 1.0);
		newVertex.z += time;                                      
		newVertex.z = fract(newVertex.z);                       
		float size = (20.0 * newVertex.z * newVertex.z);
		newVertex.z = (49.9 * newVertex.z) - 50.0; 

		gl_Position = um4p * um4mv * newVertex;                
		gl_PointSize = size; 
	}

}