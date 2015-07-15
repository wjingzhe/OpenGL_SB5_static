#version 330

uniform samplerCube cubeMap;

in vec3 vVaryingTexCoord;

out vec4 vFragColor;

void main(void)
{
	vFragColor = texture(cubeMap,vVaryingTexCoord);
}