// PointSprites.cpp
// OpenGL SuperBible
// Demonstrates Point Sprites via a flythrough star field
// Program by Richard S. Wright Jr.

#include <GLTools.h>	// OpenGL toolkit
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>
#include <GLBatch.h>
#include <GLShaderManager.h>
#include <StopWatch.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

#define NUM_STARS 10000

GLFrame viewFrame;
GLFrustum viewFrustum;
GLMatrixStack projectionMatrix;
GLMatrixStack modelViewMatrix;
GLMatrixStack mvpMatrix;
GLGeometryTransform transformPipeLine;
GLBatch starsBatch;

GLShaderManager shaderManager;
GLuint starFieldShader;
GLuint textureID;

GLint locMVP;
GLint locTimeStamp;
GLint locTexture;

bool LoadTGATexture(const char *szFileName, GLint minFilter, GLint magFilter, GLenum wrapMode)
{
	GLbyte * pBytes;
	int nWidth, nHeight, nComponent;
	GLenum eFormat;
	//载入gpu中
	pBytes = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponent, &eFormat);

	if (pBytes == NULL)
		return false;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//？？

	//绑定数据至GL_TEXTURE_2D 和对应的索引？？
	glTexImage2D(GL_TEXTURE_2D, 0, nComponent, nWidth, nHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);

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

	shaderManager.InitializeStockShaders();

	glEnable(GL_POINT_SPRITE);//启用点精灵

	GLfloat fColors[4][4] = {
		{ 1.0f, 1.0f, 1.0f, 1.0f },//white
		{ 0.67f, 0.68f, 0.82f, 1.0f },//blue Stars
		{ 1.0f, 0.5f, 0.5f, 1.0f },//Reddish
		{ 1.0f, 0.82f, 0.65f, 1.0f }
	};

	starsBatch.Begin(GL_POINTS, NUM_STARS);

	for (size_t i = 0; i < NUM_STARS; ++i)
	{
		int iColor = 0; // 默认为白
		//1/5 变为 blue
		if (rand() % 5 == 1)
			iColor = 1;

		//1/50 变为 red
		if (rand() % 50 == 1)
			iColor = 2;

		//1/100 变为 amber
		if (rand() % 100 == 1)
			iColor = 3;


		starsBatch.Color4fv(fColors[iColor]);

		M3DVector3f vPosition;
		vPosition[0] = float(3000 - (rand() % 6000)) * 0.1f;
		vPosition[1] = float(3000 - (rand() % 6000)) * 0.1f;
		vPosition[2] = -float(rand() % 1000) - 1.0f; // -1.0f 到 -1000.0f 之间

		starsBatch.Vertex3fv(vPosition);
	}

	starsBatch.End();

	starFieldShader = shaderManager.LoadShaderPairWithAttributes("PointSprite.vp", "PointSprite.fp", 2,GLT_ATTRIBUTE_VERTEX,"vVertex",GLT_ATTRIBUTE_COLOR,"vColor");

	locMVP = glGetUniformLocation(starFieldShader, "mvpMatrix");
	locTimeStamp = glGetUniformLocation(starFieldShader, "timeStamp");
	locTexture = glGetUniformLocation(starFieldShader, "starImage");
	
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	LoadTGATexture("Star.tga",GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR,GL_CLAMP_TO_EDGE);
}

void ChangeSize(int w, int h)
{
	if (h <= 0)
	{
		h = 1;
	}

	glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(80.0f, float(w) / float(h), 1.0f, 1000.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	transformPipeLine.SetMatrixStacks(modelViewMatrix, projectionMatrix);

}

void RenderScene(void)
{
	static CStopWatch timer;

	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	glEnable(GL_PROGRAM_POINT_SIZE);//允许点的大小可以修改

	glUseProgram(starFieldShader);
	//transformPipeLine.GetModelViewProjectionMatrix()
	
	glUniformMatrix4fv(locMVP, 1, GL_FALSE, viewFrustum.GetProjectionMatrix());
	glUniform1i(locTexture, 0);

	float fTime = timer.GetElapsedSeconds() * 10.0f;
	fTime = fmod(fTime, 999.0f);
	glUniform1f(locTimeStamp, fTime);

	starsBatch.Draw();

	glutSwapBuffers();
	glutPostRedisplay();
}

void ShutdownRC(void)
{
	glDeleteProgram(starFieldShader);
	glDeleteTextures(1, &textureID);
}

int main(int argc, char * argv[])
{
	gltSetWorkingDirectory(argv[0]);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(1000, 800);
	glutCreateWindow("Point Sprite Out");

	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);

	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		fprintf(stderr, "GLEW ERROR : %s\n", glewGetErrorString(err));
		return 1;
	}

	SetupRC();

	glutMainLoop();

	ShutdownRC();

	return 0;
}