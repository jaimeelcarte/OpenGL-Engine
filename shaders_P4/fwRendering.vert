#version 330 core

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
	color = vec3(0.2,0.2,0.2);
	texCoord = inTexCoord;
	norm = (normal * vec4(inNormal, 0.0)).xyz;
	pos = (modelView * vec4(inPos, 1.0)).xyz;

	vec3 textureColor = texture(colorTex, texCoord).rgb;
	float escalar = textureColor.x * 0.3 + textureColor.y * 0.4 + textureColor.z * 0.3;

	vec4 posNueva = vec4(norm * escalar, 1.0) + vec4(inPos,1.0);
	//gl_Position =  modelViewProj * vec4 (inPos,1.0);
	gl_Position = modelViewProj * posNueva;

}
