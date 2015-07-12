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


GLFrame cameraFrame;
GLFrustum viewFrustum;
GLMatrixStack projctionMatrix;
GLMatrixStack mvpMatrix;
GLMatrixStack modelViewMatrix;
GLShaderManager shaderManager;
GLGeometryTransform transformPipeLine;


GLBatch floorBatch;
GLBatch ceilingBatch;
GLBatch leftWallBatch;
GLBatch rightWallBatch;

GLfloat viewZ = -65.0f;

#define TEXTURE_BRICK 0
#define TEXTURE_FLOOR 1
#define TEXTURE_CEILING 2
#define TEXTURE_COUNT 3

GLuint uiTextures[TEXTURE_COUNT];
const char * uiTextureName[TEXTURE_COUNT] = {"brick.tga","ceiling.tga","floor.tga"};

void ChangeSize(int w, int h)
{
	if (h <= 0)
	{
		h = 1;
	}

	glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(80.0f, float(w) / float(h), 1.0f, 120.0f);
	projctionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	transformPipeLine.SetMatrixStacks(modelViewMatrix, projctionMatrix);
}

void ProcessMenu(int value)
{
	GLfloat fLargest;
	for (size_t i = 0; i < TEXTURE_COUNT; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, uiTextures[i]);

		switch (value)
		{
		case 0:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			break;

		case 1:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			break;

		case 2:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			break;

		case 3:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			break;

		case 4:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			break;

		case 5:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			break;

		case 6://在三线性过滤的模式下开启各项异性过滤就会发现计算很远距离的仍然能看到砖缝
			if (gltIsExtSupported("GL_EXT_texture_filter_anisotropic"))
			{
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
			}
			break;

		case 7:
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
			break;
		default:
			break;
		}
	}

	glutPostRedisplay();

}


void SetupRC(void)
{
	shaderManager.InitializeStockShaders();

	glGenTextures(TEXTURE_COUNT, uiTextures);

	GLbyte *pBits;
	int nWidth, nHeight, nComponents;
	GLenum eFormat;
	
	for (size_t i = 0; i < TEXTURE_COUNT; ++i)
	{
		glBindTexture(GL_TEXTURE_2D, uiTextures[i]);
		// Read the texture bits
		pBits = gltReadTGABits(uiTextureName[i], &nWidth, &nHeight, &nComponents, &eFormat);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBits);
		glGenerateMipmap(GL_TEXTURE_2D);
		free(pBits);
	}

	glClearColor(0.0f,0.0f,0.0f,1.0f);

	GLfloat z;
	floorBatch.Begin(GL_TRIANGLE_STRIP, 28,1);
	for (z = 60.f; z >= 0.0f; z -= 10.0f)
	{
		floorBatch.Normal3f(0.0f, 1.0f, 0.0f);
		floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
		floorBatch.Vertex3f(-10.0f, -10.0f, z);

		floorBatch.Normal3f(0.0f, 1.0f, 0.0f);
		floorBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
		floorBatch.Vertex3f(10.0f, -10.0f, z);

		floorBatch.Normal3f(0.0f, 1.0f, 0.0f);
		floorBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
		floorBatch.Vertex3f(-10.0f, -10.0f, z - 10.0f);

		floorBatch.Normal3f(0.0f, 1.0f, 0.0f);
		floorBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
		floorBatch.Vertex3f(10.0f,-10.0f , z - 10.0f);

	}
	floorBatch.End();


	ceilingBatch.Begin(GL_TRIANGLE_STRIP, 28, 1);// 哪里来，哪里回；根据法线来确认逆时针方向。strip的画法要熟悉，从其实的两个顶点开始已折线的形式前进
	for (z = 60.f; z >= 0.0f; z -= 10.0f)
	{
		ceilingBatch.Normal3f(0.0f, -1.0f, 0.0f);
		ceilingBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
		ceilingBatch.Vertex3f(10.0f, 10.0f, z);

		ceilingBatch.Normal3f(0.0f, -1.0f, 0.0f);
		ceilingBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
		ceilingBatch.Vertex3f(-10.0f, 10.0f, z);

		ceilingBatch.Normal3f(0.0f, -1.0f, 0.0f);
		ceilingBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
		ceilingBatch.Vertex3f(10.0f, 10.0f, z - 10.0f);

		ceilingBatch.Normal3f(0.0f, -1.0f, 0.0f);
		ceilingBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
		ceilingBatch.Vertex3f(-10.0f, 10.0f, z - 10.0f);
	}
	ceilingBatch.End();


	leftWallBatch.Begin(GL_TRIANGLE_STRIP, 28, 1);
	for (z = 60.f; z >= 0.0f; z -= 10.0f)
	{
		leftWallBatch.Normal3f(1.0f, 0.0f, 0.0f);
		leftWallBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
		leftWallBatch.Vertex3f(-10.0f, 10.0f, z);

		leftWallBatch.Normal3f(1.0f, 0.0f, 0.0f);
		leftWallBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
		leftWallBatch.Vertex3f(-10.0f, -10.0f, z);

		leftWallBatch.Normal3f(1.0f, 0.0f, 0.0f);
		leftWallBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
		leftWallBatch.Vertex3f(-10.0f, 10.0f, z - 10.0f);

		leftWallBatch.Normal3f(1.0f, 0.0f, 0.0f);
		leftWallBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
		leftWallBatch.Vertex3f(-10.0f, -10.0f, z - 10.0f);
	}
	leftWallBatch.End();


	rightWallBatch.Begin(GL_TRIANGLE_STRIP, 28, 1);
	for (z = 60.f; z >= 0.0f; z -= 10.0f)
	{
		rightWallBatch.Normal3f(-1.0f, 0.0f, 0.0f);
		rightWallBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
		rightWallBatch.Vertex3f(10.0f, -10.0f, z);

		rightWallBatch.Normal3f(-1.0f, 0.0f, 0.0f);
		rightWallBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
		rightWallBatch.Vertex3f(10.0f, 10.0f, z);

		rightWallBatch.Normal3f(-1.0f, 0.0f, 0.0f);
		rightWallBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
		rightWallBatch.Vertex3f(10.0f, -10.0f, z - 10.0f);

		rightWallBatch.Normal3f(-1.0f, 1.0f, 1.0f);
		rightWallBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
		rightWallBatch.Vertex3f(10.0f, 10.0f, z - 10.0f);
	}
	rightWallBatch.End();
}

void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	modelViewMatrix.PushMatrix();
	{

		modelViewMatrix.Translate(0.0f, 0.0f, viewZ);

		shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeLine.GetModelViewProjectionMatrix(), 0);

		glBindTexture(GL_TEXTURE_2D, uiTextures[TEXTURE_FLOOR]);
		floorBatch.Draw();

		glBindTexture(GL_TEXTURE_2D, uiTextures[TEXTURE_CEILING]);
		ceilingBatch.Draw();

		glBindTexture(GL_TEXTURE_2D, uiTextures[TEXTURE_BRICK]);
		leftWallBatch.Draw();
		rightWallBatch.Draw();
	}

	modelViewMatrix.PopMatrix();

	glutSwapBuffers();
}

void SpecialKeys(int key, int x, int y)
{
	if (key == GLUT_KEY_UP)
	{
		viewZ += 0.5f;
	}

	if (key == GLUT_KEY_DOWN)
	{
		viewZ -= 0.5f;
	}

	glutPostRedisplay();
}

void ShutdownRC(void)
{
	glDeleteTextures(3, uiTextures);
}

int main(int argc, char * argv[])
{
	gltSetWorkingDirectory(argv[0]);

	glutInit(&argc, argv);
	glutInitDisplayMode(GL_DOUBLE | GL_DEPTH | GL_STENCIL);
	glutInitWindowSize(1440, 900);
	glutCreateWindow("Tunnel @Jingz");

	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);
	glutSpecialFunc(SpecialKeys);

	glutCreateMenu(ProcessMenu);
	glutAddMenuEntry("GL_NEAREST", 0);
	glutAddMenuEntry("GL_LINEAR", 1);
	glutAddMenuEntry("GL_NEAREST_MIPMAP_NEAREST", 2);
	glutAddMenuEntry("GL_NEAREST_MIPMAP_LINEAR", 3);
	glutAddMenuEntry("GL_LINEAR_MIPMAP_NEAREST", 4);
	glutAddMenuEntry("GL_LINEAR_MIPMAP_LINEAR", 5);
	glutAddMenuEntry("Anisotropic Filter", 6);
	glutAddMenuEntry("Anisotropic Off", 7);

	glutAttachMenu(GLUT_RIGHT_BUTTON);

	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		fprintf(stderr, "GLEW ERROR: %s\n", glewGetErrorString(err));
	}

	SetupRC();

	glutMainLoop();

	ShutdownRC();

	return 0;
}