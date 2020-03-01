#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in vData
{
	vec3 pos;
    vec3 color;
	vec3 norm;
	vec2 texCoord;
}vertex[];


out fData
{
    vec3 pos;
	vec3 color;
	vec3 norm;
	vec2 texCoord;
}frag; 


void GenerateLine(int a, int b)
{
	gl_Position = gl_in[a].gl_Position;
    EmitVertex();
    gl_Position = gl_in[b].gl_Position;
    EmitVertex();
    EndPrimitive();
}

void main() {    
	gl_PointSize = 5.0;
	frag.color = vec3(1.0, 1.0, 0.0);
    GenerateLine(0,1);
	GenerateLine(1,2);
	GenerateLine(2,0);
} 