#include <GLTools.h>
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>
#include <GLShaderManager.h>
#include <StopWatch.h>

#ifdef __APPLE__
#include <lgut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

GLFrame viewFrame;
GLFrustum viewFrustum;
GLMatrixStack projectionMatrix;
GLMatrixStack mvpMatrix;
GLMatrixStack mvMatrix;
GLGeometryTransform transformPipeLine;
GLShaderManager shaderManager;

GLTriangleBatch sphereBatch;

GLuint ADSPhongShader;

GLint locAC;
GLint locDC;
GLint locSC;
GLint locLP;
GLint locMVP;
GLint locMV;
GLint locNM;

void SetupRC(void)
{
	//先绘制背景
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);

	shaderManager.InitializeStockShaders();

	viewFrame.MoveForward(4.0f);

	gltMakeSphere(sphereBatch, 1.0f, 26, 13);
	//将顶点和法线向量输入至顶点着色器中，变量名分别为：vVertex，vNormal
	ADSPhongShader = shaderManager.LoadShaderPairWithAttributes("ADSPhong.vp", "ADSPhong.fp", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_NORMAL, "vNormal");

	locAC = glGetUniformLocation(ADSPhongShader, "ambientColor");
	locDC = glGetUniformLocation(ADSPhongShader, "diffuseColor");
	locSC = glGetUniformLocation(ADSPhongShader, "specularColor");
	locLP = glGetUniformLocation(ADSPhongShader, "vLightPosition");
	locMVP = glGetUniformLocation(ADSPhongShader, "mvpMatrix");
	locMV = glGetUniformLocation(ADSPhongShader, "mvMatrix");
	locNM = glGetUniformLocation(ADSPhongShader, "normalMatrix");
}

void ChangeSize(int w, int h)
{
	if (h <= 0)
	{
		h = 1;
	}

	glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(35, float(w) / float(h), 1.0f, 100.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	transformPipeLine.SetMatrixStacks(mvMatrix, projectionMatrix);

}

void RenderScene(void)
{
	static CStopWatch rotTimer;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mvMatrix.PushMatrix(viewFrame);
	mvMatrix.Rotate(rotTimer.GetElapsedSeconds() * 10.0f, 0.0f, 1.0f, 0.0f);

	GLfloat vAmbientColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };

	GLfloat vDiffuseColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };

	GLfloat vSpecularColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	GLfloat vEyeLight[] = { -100.0f, 100.0f, 100.0f };

	glUseProgram(ADSPhongShader);

	glUniform4fv(locAC, 1, vAmbientColor);
	glUniform4fv(locDC, 1, vDiffuseColor);
	glUniform4fv(locSC, 1, vSpecularColor);
	glUniform3fv(locLP, 1, vEyeLight);

	glUniformMatrix4fv(locMVP, 1, GL_FALSE, transformPipeLine.GetModelViewProjectionMatrix());
	glUniformMatrix4fv(locMV, 1, GL_FALSE, transformPipeLine.GetModelViewMatrix());
	glUniformMatrix3fv(locNM, 1, GL_FALSE, transformPipeLine.GetNormalMatrix());


	sphereBatch.Draw();
	mvMatrix.PopMatrix();

	glutSwapBuffers();

	glutPostRedisplay();
}

void ShutdownRC(void)
{
	glDeleteProgram(ADSPhongShader);
}

int main(int argc, char * argv[])
{
	gltSetWorkingDirectory(argv[0]);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("ADSPhong Jingz");
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);


	GLenum err = glewInit();

	if (err != GLEW_OK)
	{
		fprintf(stderr, "GLEW ERROR:%s\n", glewGetErrorString(err));
		return 1;
	}


	SetupRC();

	glutMainLoop();

	ShutdownRC();

	return 0;
}