#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 8) out;

uniform mat4 modelViewProj;

in vData
{
	vec3 pos;
    vec3 color;
	vec3 norm;
	vec2 texCoord;
}vertex[];


out fData
{
    vec4 pos;
	vec3 color;
	vec4 norm;
	vec2 texCoord;
}frag; 

const float MAGNITUDE = 0.1;

void main() 
{   
	// Vertex normals
	for (int i = 0; i<3; ++i){
		frag.color = vec3(1.0, 1.0, 0.0);
		vec4 pos = gl_in[i].gl_Position;
		vec4 normal = vec4(vertex[i].norm, 0.0);
		gl_Position =  modelViewProj * (normal * 0.000001 + pos);
		EmitVertex();
		gl_Position =  modelViewProj * (normal * MAGNITUDE + pos);
		EmitVertex();
		EndPrimitive();
	}

	// Face normals
	vec3 p0 = gl_in[0].gl_Position.xyz;
	vec3 p1 = gl_in[1].gl_Position.xyz;
	vec3 p2 = gl_in[2].gl_Position.xyz;

	vec3 v0 = p0 - p1;
	vec3 v1 = p2 - p1;

	vec3 N = normalize(cross(v1, v0));
	vec3 P = (p0 + p1 + p2) / 3.0;

	// face normals will be coloured in red
	frag.color = vec3(1.0, 0.0, 0.0);

	gl_Position = modelViewProj * vec4(P, 1.0);
	EmitVertex();
	gl_Position = modelViewProj * vec4(P + N * MAGNITUDE, 1.0);
	EmitVertex();
	EndPrimitive();
	
    
}  