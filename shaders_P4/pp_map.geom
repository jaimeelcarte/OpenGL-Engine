#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vData
{
	vec3 pos;
    vec3 color;
	vec3 norm;
	vec2 texCoord;
}vertex[];

uniform mat4 modelViewProj;
uniform sampler2D colorTex;

out fData
{
    vec3 pos;
	vec3 color;
	vec3 norm;
	vec2 texCoord;
}frag; 

void main() {  
	
	vec4 textureColor;
	float displacementFactor = 1.0;

	for (int i=0; i<3; i++){
		textureColor = texture(colorTex, vertex[i].texCoord);
		frag.pos = vertex[i].pos;
		frag.color = textureColor.rgb;
		frag.norm = vertex[i].norm;
		frag.texCoord = vertex[i].texCoord;
		gl_Position = gl_in[i].gl_Position + textureColor * displacementFactor;
		//gl_Position = gl_in[i].gl_Position;
		EmitVertex();
	}
    EndPrimitive();
} 