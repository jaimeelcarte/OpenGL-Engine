#version 330 core

//Color de salida
layout(location = 0) out vec4 outColor;

//Variables Variantes
in fData
{
	vec4 pos;
    vec3 color;
	vec4 norm;
	vec2 texCoord;
}frag;


//Textura
uniform sampler2D colorTex;
uniform sampler2D vertexTex; //Vamos a probar cambiar DOF con depth buffer

void main()
{
	outColor = vec4(frag.color, 1.0);   
}