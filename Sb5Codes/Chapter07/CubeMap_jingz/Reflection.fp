#version 330

smooth in vec3 vVaryingTexCoord;
smooth in vec2 vTarnishCoords;

uniform samplerCube cubeMap;
uniform sampler2D tarnishMap;

out vec4 vFragColor;

void main(void)
{
	vFragColor = texture(cubeMap, vVaryingTexCoord.stp);
	//两段纹理，只需要颜色相乘就可以了。后面的纹理为最外层色彩
	vFragColor *= texture(tarnishMap, vTarnishCoords.st);
}