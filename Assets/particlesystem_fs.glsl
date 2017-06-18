#version 410

layout(location = 0) out vec4 fragColor;

uniform mat4 um4mv;
uniform mat4 um4p;

in VertexData
{
	vec3 N; // eye space normal
	vec3 L; // eye space light vector
	vec3 V; // eye space halfway vector
	vec2 texcoord;
} vertexData;

uniform sampler2D tex;

void main()
{
	vec3 texColor = texture(tex, gl_PointCoord).rgb;
	if (texColor == vec3(0))
		discard;
	else if (texColor == vec3(1))
		discard;
	else
		fragColor = vec4(texColor, 1.0);
}