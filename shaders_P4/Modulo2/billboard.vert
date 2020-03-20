#version 430 core

layout (binding=4, std430) buffer Pos
{
    vec4 vertices[];
};

layout(std140, binding = 6) buffer Col
{
	vec4 Colors[];
};

uniform mat4 modelViewProj;
uniform mat4 modelView;
uniform mat4 normal;

out vec4 color;

void main()
{
	color = Colors[gl_VertexID];
	gl_Position =  modelView * vertices[gl_VertexID];
}
