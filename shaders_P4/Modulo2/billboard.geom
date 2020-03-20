#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

uniform mat4 proj;
uniform mat4 modelView;
uniform mat4 normal;
uniform vec4 cameraPos;

in vec4 color[];

out vec4 fragColor;
out vec3 pos;
out vec3 norm;
out vec2 texCoord;

void main() {    

	//Pass-through del color
	fragColor = color[0];
	
	//Rellenar tanto el gl_position como texcoord
	float escala = 0.1;

	texCoord = vec2(0.0, 0.0);
	gl_Position = proj * (gl_in[0].gl_Position + vec4(texCoord, 0.0, 0.0));
	EmitVertex();
	texCoord = vec2(1.0, 0.0);
	gl_Position = proj * (gl_in[0].gl_Position + vec4(texCoord, 0.0, 0.0));
	EmitVertex();
	texCoord = vec2(0.0, 1.0);
	gl_Position = proj * (gl_in[0].gl_Position + vec4(texCoord, 0.0, 0.0));
	EmitVertex();
	texCoord = vec2(1.0, 1.0);
	gl_Position = proj * (gl_in[0].gl_Position + vec4(texCoord, 0.0, 0.0));
	EmitVertex();

	EndPrimitive();
} 