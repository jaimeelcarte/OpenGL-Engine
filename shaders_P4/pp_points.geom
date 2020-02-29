#version 330 core
layout (triangles) in;
layout (points, max_vertices = 3) out;

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


void main() 
{   
	for (int i = 0; i < 3; ++i)
	{
		frag.color = vertex[i].color;
		frag.pos = vertex[i].pos;
		frag.norm = vertex[i].norm;
		frag.texCoord = vertex[i].texCoord;

		gl_Position = gl_in[i].gl_Position;
		
		EmitVertex();
	}
	
    EndPrimitive();
}  