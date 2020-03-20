#version 330 core

out vec4 outColor;

in vec4 fragColor;
in vec3 pos;
in vec3 norm;
in vec2 texCoord;

uniform sampler2D colorTex;

//Propiedades del objeto
vec3 Ka;
vec3 Kd;
vec3 Ks;
vec3 N;
float alpha = 500.0;
vec3 Ke;

//Propiedades de la luz
vec3 Ia = vec3 (0.3);
vec3 Id = vec3 (1.0);
vec3 Is = vec3 (0.7);
vec3 lpos = vec3 (0.0); 

vec3 shade();


void main()
{

	//Ka = color;
	//Kd = color;
	vec4 textureColor = texture(colorTex, texCoord);
	//Ka = textureColor;
	//Kd = textureColor;
	Ke = vec3(0.0);
	Ks = vec3 (1.0);


	N = normalize (norm);
	
	//outColor = vec4(shade(), 1.0); 
	if (textureColor.w == 0)
		outColor = vec4(textureColor.xyz,0);
	else
		outColor = vec4(textureColor.xyz,fragColor.w);
	
}

vec3 shade()
{
	vec3 c = vec3(0.0);
	c = Ia * Ka;

	vec3 L = normalize (lpos - pos);
	vec3 diffuse = Id * Kd * dot (L,N);
	c += clamp(diffuse, 0.0, 1.0);
	
	vec3 V = normalize (-pos);
	vec3 R = normalize (reflect (-L,N));
	float factor = max (dot (R,V), 0.01);
	vec3 specular = Is*Ks*pow(factor,alpha);
	c += clamp(specular, 0.0, 1.0);

	c+=Ke;
	
	return c;
}
