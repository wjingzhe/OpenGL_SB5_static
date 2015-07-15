#version 140
smooth in vec2 vVaryingTexCoord;

uniform sampler2DRect rectangleImage;

out vec4 vFragColor;

void main(void)
{
	vFragColor = texture(rectangleImage,vVaryingTexCoord);
}