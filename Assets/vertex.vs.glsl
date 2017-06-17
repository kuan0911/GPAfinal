#version 410

layout(location = 0) in vec3 iv3vertex;
layout(location = 1) in vec2 iv2tex_coord;
layout(location = 2) in vec3 iv3normal;


uniform mat4 um4mv;
uniform mat4 um4p;
uniform int type;
uniform float time;
uniform vec3 light_pos;

out VertexData
{
    vec3 N; 
    vec3 L; 
    vec3 V; 
    vec2 texcoord;
} vertexData;


void main()
{
	if(type==0){                                                         
                                          
        // Calculate view-space coordinate                 
	    vec4 P = um4mv * vec4(iv3vertex, 1.0);          
                                                           
        // Calculate normal in view-space                  
        vertexData.N = mat3(um4mv) * iv3normal;               
                                                           
        // Calculate light vector                          
        vertexData.L = light_pos - P.xyz;                      
                                                           
        // Calculate view vector                           
        vertexData.V = -P.xyz;                                 
                                                           
        // Calculate the clip-space position of each vertex
        gl_Position = um4p * P;        
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