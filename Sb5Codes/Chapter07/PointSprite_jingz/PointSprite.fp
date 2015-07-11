#version 330

in vec4 vVaryingColor;
out vec4 vFragColor;//只有唯一的输出，所以不管命名时什么，输入的数据格式必须对就行了

uniform sampler2D starImage;

void main(void)
{
	vFragColor = texture(starImage,gl_PointCoord) * vVaryingColor;
}