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

#define NUM_SPHERES 60

GLFrame cameraFrame;
GLFrustum viewFrustum;
GLMatrixStack projctionMatrix;
GLMatrixStack mvpMatrix;
GLMatrixStack modelViewMatrix;
GLShaderManager shaderManager;
GLGeometryTransform transformPipeLine;

GLTriangleBatch sphereBatch;
GLTriangleBatch torusBatch;
GLBatch floorBatch;

GLFrame sphereFrames[NUM_SPHERES];

GLuint uiTextures[3];

void DrawSongAndDance(GLfloat yRot)		// Called to draw dancing objects
{
	static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	static GLfloat vLightPos[] = { 0.0f, 3.0f, 0.0f, 1.0f };

	// Get the light position in eye space
	M3DVector4f	vLightTransformed;
	M3DMatrix44f mCamera;
	modelViewMatrix.GetMatrix(mCamera);
	m3dTransformVector4(vLightTransformed, vLightPos, mCamera);

	// Draw the light source
	modelViewMatrix.PushMatrix();
	modelViewMatrix.Translatev(vLightPos);
	shaderManager.UseStockShader(GLT_SHADER_FLAT,
		transformPipeLine.GetModelViewProjectionMatrix(),
		vWhite);
	sphereBatch.Draw();
	modelViewMatrix.PopMatrix();

	glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
	for (int i = 0; i < NUM_SPHERES; i++) {
		modelViewMatrix.PushMatrix();
		modelViewMatrix.MultMatrix(sphereFrames[i]);
		shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
			modelViewMatrix.GetMatrix(),
			transformPipeLine.GetProjectionMatrix(),
			vLightTransformed,
			vWhite,
			0);
		sphereBatch.Draw();
		modelViewMatrix.PopMatrix();
	}

	// Song and dance
	modelViewMatrix.Translate(0.0f, 0.2f, -2.5f);
	modelViewMatrix.PushMatrix();	// Saves the translated origin
	modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);

	// Draw stuff relative to the camera
	glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
	shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
		modelViewMatrix.GetMatrix(),
		transformPipeLine.GetProjectionMatrix(),
		vLightTransformed,
		vWhite,
		0);
	torusBatch.Draw();
	modelViewMatrix.PopMatrix(); // Erased the rotate

	modelViewMatrix.Rotate(yRot * -2.0f, 0.0f, 1.0f, 0.0f);
	modelViewMatrix.Translate(0.8f, 0.0f, 0.0f);

	glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
	shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
		modelViewMatrix.GetMatrix(),
		transformPipeLine.GetProjectionMatrix(),
		vLightTransformed,
		vWhite,
		0);
	sphereBatch.Draw();
}

bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
{
	GLbyte *pBits;
	int nWidth, nHeight, nComponents;
	GLenum eFormat;

	// Read the texture bits
	pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents, &eFormat);
	if (pBits == NULL)
		return false;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, nWidth, nHeight, 0,
		eFormat, GL_UNSIGNED_BYTE, pBits);

	free(pBits);

	if (minFilter == GL_LINEAR_MIPMAP_LINEAR ||
		minFilter == GL_LINEAR_MIPMAP_NEAREST ||
		minFilter == GL_NEAREST_MIPMAP_LINEAR ||
		minFilter == GL_NEAREST_MIPMAP_NEAREST)
		glGenerateMipmap(GL_TEXTURE_2D);

	return true;
}

void ChangeSize(int w, int h)
{
	if (h <= 0)
	{
		h = 1;
	}

	glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 100.0f);
	projctionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	transformPipeLine.SetMatrixStacks(modelViewMatrix, projctionMatrix);
}

void SetupRC(void)
{
	shaderManager.InitializeStockShaders();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	gltMakeSphere(sphereBatch, 0.2f, 26, 13);

	gltMakeTorus(torusBatch, 0.4f, 0.15f, 30, 30);

	for (size_t i = 0; i < NUM_SPHERES; ++i)
	{
		float x = (GLfloat(rand() % 400) - 200.0f) * 0.1f;
		float z = (GLfloat(rand() % 400) - 200.0f) * 0.1f;
		sphereFrames[i].SetOrigin(x, 0.0f, z);
	}
	

	//Make the solid ground
	GLfloat texSize = 10.0f;

	floorBatch.Begin(GL_TRIANGLE_FAN, 4,1);
	{
		//没看懂纹理和顶点的映射关系
		floorBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
		floorBatch.Vertex3f(-20.0f, -0.41f, 20.0f);

		floorBatch.MultiTexCoord2f(0, texSize, 0.0f);
		floorBatch.Vertex3f(20.0f, -0.41f, 20.0f);

		floorBatch.MultiTexCoord2f(0, texSize, texSize);
		floorBatch.Vertex3f(20.0f, -0.41f, -20.0f);

		floorBatch.MultiTexCoord2f(0, 0.0f, texSize);
		floorBatch.Vertex3f(-20.0f, -0.41f, -20.0f);
	}
	floorBatch.End();

	// Make 3 texture objects
	glGenTextures(3, uiTextures);

	// Load the Marble
	glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
	LoadTGATexture("marble.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT);

	// Load Mars
	glBindTexture(GL_TEXTURE_2D, uiTextures[1]);
	LoadTGATexture("marslike.tga", GL_LINEAR_MIPMAP_LINEAR,
		GL_LINEAR, GL_CLAMP_TO_EDGE);

	// Load Moon
	glBindTexture(GL_TEXTURE_2D, uiTextures[2]);
	LoadTGATexture("moonlike.tga", GL_LINEAR_MIPMAP_LINEAR,
		GL_LINEAR, GL_CLAMP_TO_EDGE);
}

void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	static GLfloat vFloorColor[] = { 1.0f, 1.0f, 1.0f, 0.75f };
	static GLfloat vTorusColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	static GLfloat vSphereColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	
	static CStopWatch rotTimer;
	float yRot = rotTimer.GetElapsedSeconds() * 60.0f;

	modelViewMatrix.PushMatrix();
	{

		M3DMatrix44f mCamera;
		cameraFrame.GetCameraMatrix(mCamera);
		modelViewMatrix.MultMatrix(mCamera);
		// Draw the world upside down
		modelViewMatrix.PushMatrix();
		{
			
			modelViewMatrix.Scale(1.0f, -1.0f, 1.0f); // Flips the Y Axis
			modelViewMatrix.Translate(0.0f, 0.8f, 0.0f); // Scootch the world down a bit...
			glFrontFace(GL_CW);//倒影世界，环绕方向为顺时针
			DrawSongAndDance(yRot);
			glFrontFace(GL_CCW);
		}
		modelViewMatrix.PopMatrix();

		glEnable(GL_BLEND);
		glBindTexture(GL_TEXTURE_2D, uiTextures[0]);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		shaderManager.UseStockShader(GLT_SHADER_TEXTURE_MODULATE, transformPipeLine.GetModelViewProjectionMatrix(), vFloorColor, 0);
		floorBatch.Draw();
		glDisable(GL_BLEND);

		DrawSongAndDance(yRot);
	}
	
	modelViewMatrix.PopMatrix();

	glutSwapBuffers();

	glutPostRedisplay();
}

void SpecialKeys(int key, int x, int y)
{
	float linear = 0.1f;
	float angular = float(m3dDegToRad(5.0f));

	if (key == GLUT_KEY_UP)
	{
		cameraFrame.MoveForward(linear);
	}

	if (key == GLUT_KEY_DOWN)
	{
		cameraFrame.MoveForward(-linear);
	}

	if (key == GLUT_KEY_LEFT)
	{
		cameraFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);
	}

	if (key == GLUT_KEY_RIGHT)
	{
		cameraFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);
	}
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
	glutCreateWindow("SphereWprld with Texture @Jingz");

	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);
	glutSpecialFunc(SpecialKeys);

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