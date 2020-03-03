#version 330 core

in vec3 inPos;	
in vec3 inColor;
in vec2 inTexCoord;
in vec3 inNormal;

uniform mat4 modelViewProj;
uniform mat4 modelView;
uniform mat4 normal;

out vData
{
    vec3 pos;
	vec3 color;
	vec3 norm;
	vec2 texCoord;
}vertex;


void main()
{

	vertex.color = vec3(1.0, 1.0, 0.0);
	vertex.texCoord = inTexCoord;
	vertex.norm = (normal * vec4(inNormal, 0.0)).xyz;
	vertex.pos = (modelView * vec4(inPos, 1.0)).xyz;

	gl_Position =  modelViewProj * vec4 (inPos,1.0);
	
}
