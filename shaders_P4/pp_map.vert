#version 330 core

in vec3 inPos;	
in vec3 inColor;
in vec2 inTexCoord;
in vec3 inNormal;

uniform mat4 model;

uniform mat4 modelViewProj;
uniform mat4 modelView;
uniform mat4 normal;
uniform sampler2D colorTex;

out vData
{
    vec3 pos;
	vec3 color;
	vec3 norm;
	vec2 texCoord;
}vertex;


void main()
{
//	vertex.color = vec3(1.0, 1.0, 0.0);
////	vertex.color = texture(colorTex, vertex.texCoord).xyz;
//	vertex.texCoord = inTexCoord;
//	vertex.norm = (normal * vec4(inNormal, 0.0)).xyz;
//	vertex.pos = (modelView * vec4(inPos, 1.0)).xyz;
//
//	gl_Position =  modelViewProj * vec4 (inPos,1.0);

	vertex.pos = (model * vec4(inPos, 1.0)).xyz;
	vertex.color = vec3(1.0, 1.0, 0.0);
	vertex.norm = (model * vec4(inNormal, 0.0)).xyz;
	vertex.texCoord = inTexCoord;

}
