#version 430 core

layout (binding=4, std430) buffer Pos
{
    vec4 vertices[];
};

in vec3 inPos;	
in vec3 inColor;
in vec2 inTexCoord;
in vec3 inNormal;

uniform mat4 modelViewProj;
uniform mat4 modelView;
uniform mat4 normal;

uniform sampler2D colorTex;

out vec3 color;
out vec3 pos;
out vec3 norm;
out vec2 texCoord;


void main()
{
	color = vec3(1.0,1.0,0.0);
	texCoord = inTexCoord;
	norm = (normal * vec4(inNormal, 0.0)).xyz;
	pos = (modelView * vertices[gl_VertexID]).xyz;
	gl_Position =  modelViewProj * vertices[gl_VertexID];

}
