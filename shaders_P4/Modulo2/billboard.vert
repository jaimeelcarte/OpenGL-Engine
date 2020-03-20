#version 430 core

layout (binding=4, std430) buffer Pos
{
    vec4 vertices[];
};


uniform mat4 modelViewProj;
uniform mat4 modelView;
uniform mat4 normal;

uniform sampler2D colorTex;

void main()
{
	gl_Position =  vertices[gl_VertexID];

}
