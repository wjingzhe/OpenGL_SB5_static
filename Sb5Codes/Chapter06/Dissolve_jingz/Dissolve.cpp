#include <GLTools.h>
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>
#include <GLShaderManager.h>
#include <StopWatch.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

GLFrame viewFrame;
GLFrustum viewFrustum;
GLMatrixStack projctionMatrix;
GLMatrixStack modelViewMatrix;
GLMatrixStack mvpMatrix;
GLGeometryTransform transfromPipeLine;

GLShaderManager shaderManager;
GLTriangleBatch torusBatch;

GLuint ADSDissolveShader;
GLuint uiTexture;
GLint locMV;
GLint locMVP;
GLint locNM;
GLint locLightPosition;

GLint locAmbient;
GLint locDiffuse;
GLint locSpec;
GLint locCloudTex;
GLint locDissolveFactor;

void ChangeSize(int w, int h)
{
	if (h <= 0)
	{
		h = 1;
	}

	glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 100.0f);
	projctionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	transfromPipeLine.SetMatrixStacks(modelViewMatrix, projctionMatrix);

}

bool LoadTGATexture(const char * szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
	GLbyte * pBytes;
	GLint nWidth, nHeight, nComponets;
	GLenum eFormat;

	pBytes = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponets, &eFormat);
	if (pBytes == NULL)
	{
		return false;
	}
	
	/* 来自百度百科，看来我的层次只需要使用百度就能满足需求

	图象从纹理图象空间映射到帧缓冲图象空间(映射需要重新构造纹理图像,这样就会造成应用到多边形上的图像失真),这时就可用glTexParmeteri()函数来确定如何把纹理象素映射成像素.
	部分参数功能说明如下:
　　glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
　　GL_TEXTURE_2D: 操作2D纹理.
　　GL_TEXTURE_WRAP_S: S方向上的贴图模式. 
　　GL_CLAMP: 将纹理坐标限制在0.0,1.0的范围之内.如果超出了会如何呢.不会错误,只是会边缘拉伸填充.
　　glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
　　这里同上,只是它是T方向
　　glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
　　这是纹理过滤
　　GL_TEXTURE_MAG_FILTER: 放大过滤   在纹理被放大时使用的过滤函数
　　GL_LINEAR: 线性过滤, 使用距离当前渲染像素中心最近的4个纹素加权平均值.
　　glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
　　GL_TEXTURE_MIN_FILTER: 缩小过滤 在纹理被缩小时使用的过滤函数
　　GL_LINEAR_MIPMAP_NEAREST: 使用GL_NEAREST对最接近当前多边形的解析度的两个层级贴图进行采样,然后用这两个值进行线性插值.
	*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);


	/*
	//http://www.cnblogs.com/sunnyjones/articles/798237.html

	這個函数是對應著 glDrawPixels 而來的, 因為效率考慮, 所以,

	OpenGL 預設, 你給 glDrawPixels 的圖檔資料, 它的每一個 row 的大小 ( 以 byte 來算 ), 也是可以給 4 整除的.

	假設你的圖檔是 150x150, 每一個 row 的大小就會是 150 * 3 = 450 , 450 不能被 4 整除的. 如果要強行把它換成可以被 4 整除, 一般的做法, 就是在每一個 row 多加 2 bytes 沒用途的資料 (這個步驟我們叫 padding ), 如此 450 就會變成 452, 452 就可以被 4 整除了.

	但是, 每 row 大小, 需要是多少的倍數, 雖然預設了是 4, 但是, 你是可以把它改成 1, 2, 4, 8, 其中任意一個的, 如果你設成 1, 這麼你就可以不用管 padding 的問題了 ( 因為什麼整數也可以被 1 整除呀 ), 但是, 懶散的結果, 就是程式 run-time 時慢一點點.

	最好的做法, 應該直接使用 寬 可被 4 整除的圖.

	*/
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, nComponets, nWidth, nHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);

	free(pBytes);

	if (minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST || minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_NEAREST_MIPMAP_NEAREST)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	return true;
}

void SetupRC(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);

	shaderManager.InitializeStockShaders();

	viewFrame.MoveForward(4.0f);

	gltMakeTorus(torusBatch, 0.8f, 0.25f, 52, 26);

	ADSDissolveShader = shaderManager.LoadShaderPairWithAttributes("Dissolve.vp", "Dissolve.fp", 3, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal", GLT_ATTRIBUTE_TEXTURE0, "vTexCoords0");

	//vp
	locMV = glGetUniformLocation(ADSDissolveShader, "mvMatrix");
	locMVP = glGetUniformLocation(ADSDissolveShader, "mvpMatrix");
	locNM = glGetUniformLocation(ADSDissolveShader, "normalMatrix");
	locLightPosition = glGetUniformLocation(ADSDissolveShader, "vLightPosition");


	//fp
	locAmbient = glGetUniformLocation(ADSDissolveShader, "ambientColor");
	locDiffuse = glGetUniformLocation(ADSDissolveShader, "diffuseColor");
	locSpec = glGetUniformLocation(ADSDissolveShader, "specularColor");
	locCloudTex = glGetUniformLocation(ADSDissolveShader, "cloudTexture");
	locDissolveFactor = glGetUniformLocation(ADSDissolveShader, "dissolveFactor");


	glGenTextures(1, &uiTexture);
	glBindTexture(GL_TEXTURE_2D, uiTexture); // 
	LoadTGATexture("Clouds.tga", GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);

}

void RenderScene(void)
{
	static CStopWatch rotTimer;
	static GLfloat vEyeLight[] = { -100.0f, 100.0f, 100.0f };
	static GLfloat vAmbientColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	static GLfloat vDiffuseColor[] = { 0.1f, 1.0f, 0.1f, 1.0f };
	static GLfloat vSpecularColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	modelViewMatrix.PushMatrix(viewFrame);
	{
		modelViewMatrix.Rotate(rotTimer.GetElapsedSeconds() * 10.0f,0.0f,1.0f,0.0f);
		glUseProgram(ADSDissolveShader);
		//vp
		glUniformMatrix4fv(locMV, 1, GL_FALSE, transfromPipeLine.GetModelViewMatrix());
		glUniformMatrix4fv(locMVP, 1, GL_FALSE, transfromPipeLine.GetModelViewProjectionMatrix());
		glUniformMatrix3fv(locNM, 1, GL_FALSE, transfromPipeLine.GetNormalMatrix());
		glUniform3fv(locLightPosition, 1, vEyeLight);

		//fp
		glUniform4fv(locAmbient, 1, vAmbientColor);
		glUniform4fv(locDiffuse, 1, vDiffuseColor);
		glUniform4fv(locSpec, 1, vSpecularColor);
		glUniform1i(locCloudTex, 0);// ？？

		float fFactor = fmod(rotTimer.GetElapsedSeconds(), 10.0f);
		fFactor /= 10.0f;
		
		glUniform1f(locDissolveFactor, fFactor);

		torusBatch.Draw();
	}
	modelViewMatrix.PopMatrix();

	glutSwapBuffers();

	glutPostRedisplay();

}

void ShutdownRC(void)
{
	glDeleteTextures(1, &uiTexture);
	glDeleteProgram(ADSDissolveShader);
}

int main(int argc, char * argv[])
{
	gltSetWorkingDirectory(argv[0]);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Dissolve Jingz");
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
		return 1;
	}

	SetupRC();
	glutMainLoop();
	ShutdownRC();
	return 0;
}