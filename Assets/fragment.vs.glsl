#version 410

layout(location = 0) out vec4 fragColor;

uniform mat4 um4mv;
uniform mat4 um4p;
uniform int type;

in VertexData
{
    vec3 N; // eye space normal
    vec3 L; // eye space light vector
    vec3 V; // eye space halfway vector
    vec2 texcoord;
} vertexData;

uniform sampler2D tex;
//Material properties
uniform vec3 diffuse_albedo = vec3(0.8, 0.8, 0.8);
uniform vec3 specular_albedo = vec3(0.7);
uniform float specular_power = 200.0;
uniform float Ia = 0.3;
uniform float Id = 0.8;
uniform float Is = 0.3;
void main()
{
	if(type==0){
	    // Normalize the incoming N, L and V vectors
        vec3 N = normalize(vertexData.N);
        vec3 L = normalize(vertexData.L);
        vec3 V = normalize(vertexData.V);
        vec3 H = normalize(L + V);

		vec3 texcolor = texture(tex,vertexData.texcoord).rgb;
		vec3 ambient = texcolor*Ia;
        // Compute the diffuse and specular components for each fragment
        vec3 diffuse = max(dot(N, L), 0.0) * texcolor*Id;
        vec3 specular = pow(max(dot(N, H), 0.0), specular_power) * specular_albedo*Is;

        // Write final color to the framebuffer
        fragColor = vec4(ambient+diffuse+specular, 1.0);
		
	}
	else if(type==1){
		vec3 texColor = texture(tex,gl_PointCoord).rgb;
		if(texColor == vec3(0))
			 discard;
		else if(texColor == vec3(1))
			 discard;
		else
			fragColor = vec4(texColor, 1.0);
	}
}