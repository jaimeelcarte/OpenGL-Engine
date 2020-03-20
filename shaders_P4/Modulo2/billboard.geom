#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

uniform mat4 modelViewProj;
uniform mat4 modelView;
uniform mat4 normal;
uniform vec4 cameraPos;

out fData
{
	vec3 color;
}frag; 

out vec2 texCoord;

void main() {    
	//Rellenar tanto el gl_position como texcoord
	gl_Position = gl_in[0].gl_Position;
	mat4 view = rotate(0.0,0.0,0.0,1.0);
} 