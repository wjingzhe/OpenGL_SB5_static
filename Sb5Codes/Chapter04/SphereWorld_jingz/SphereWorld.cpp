#include <GLTools.h>
#include <GLBatch.h>
#include <GLFrustum.h>
#include <GLMatrixStack.h>
#include <GLShaderManager.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>
#include <GLFrame.h>

#include <math.h>
#include <stdio.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

GLShaderManager shaderManager;
GLFrustum viewFrusm;
GLMatrixStack projectionMatrix;
GLGeometryTransform transformPipeline;
GLMatrixStack modelViewMatrix;
GLBatch floorBatch;
GLTriangleBatch torusBatch;
GLTriangleBatch sphereBatch;
GLFrame cameraFrame;
#define  NUM_SPHERES 50
GLFrame spheres[NUM_SPHERES];

void SetUpRC(void)
{
	shaderManager.InitializeStockShaders();

	glEnable(GL_DEPTH_TEST);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	gltMakeTorus(torusBatch, 0.4f, 0.15f, 30, 30);

	// This make a sphere
	gltMakeSphere(sphereBatch, 0.1f, 26, 13);

	for (int i = 0; i < NUM_SPHERES; ++i)
	{
		GLfloat x = (GLfloat(rand() % 400) - 200) * 0.1f;
		GLfloat z = (GLfloat(rand() % 400) - 200) * 0.1f;
		spheres[i].SetOrigin(x, 0.0f, z);
	}

	//ÿ���������һ����
	floorBatch.Begin(GL_LINES,324);

	for (GLfloat x = - 20.0; x <= 20.0f; x+= 0.5)
	{
		//����
		floorBatch.Vertex3f(x, -0.55f, 20.0f);
		floorBatch.Vertex3f(x, -0.55f, -20.0f);

		//����
		floorBatch.Vertex3f(20.0f, -0.55f, x);
		floorBatch.Vertex3f(-20.0f, -0.55f, x);
	}

	floorBatch.End();
}

void ChangeSize(int w, int h)
{
	if (h <= 0)
	{
		h = 1;
	}

	glViewport(0, 0, w, h);

	//�������ö�Ӧ�Ľ�ͷ����Ϣ
	viewFrusm.SetPerspective(35, float(w) / float(h), 1.0f, 100.0f);
	projectionMatrix.LoadMatrix(viewFrusm.GetProjectionMatrix());//�����µ�ͶӰ����
	//ͨ�������Ͳ��������ü��α任��ģ����ͼ�����ͶӰ����Ϊ��������������ģ����ͼ�����ͶӰ���󣨼���ͷ�嵼����ͶӰӳ�����
	transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
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

void RenderScene(void)
{
	static GLfloat vFloorColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	static GLfloat vTorusColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
	static GLfloat vSphereColor[] = { 0.0f, 1.0f, 0.0f, 1.0f };

	//
	static CStopWatch rotTimer;
	float yRot = rotTimer.GetElapsedSeconds() * 60.0f;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

	modelViewMatrix.PushMatrix();
	
	M3DMatrix44f mCamera;
	cameraFrame.GetCameraMatrix(mCamera);
	modelViewMatrix.PushMatrix(mCamera);

	M3DVector4f vLightPos = { 0.0f, 10.0f, 5.0f, 1.0f };
	M3DVector4f vLightEyePos;
	m3dTransformVector4(vLightEyePos, vLightPos, mCamera);

	//֮��Ļ��ƻ���Ⱦ���ǻ����������ǰ��λ�ò���������������λ�û���ΪͶӰ��������ðѱ仯Ч����ʾ����Ļ���ˡ��򵥵����ԭ�ȵĻ��ƽ�����ٴξ�������������ź���ת����ͶӰ��2D��Ļ��

	shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vFloorColor);

	floorBatch.Draw();


	for (int i = 0; i < NUM_SPHERES; ++i)
	{
		modelViewMatrix.PushMatrix();
		modelViewMatrix.MultMatrix(spheres[i]);
		shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vLightEyePos, vSphereColor);
		sphereBatch.Draw();
		modelViewMatrix.PopMatrix();
	}



	modelViewMatrix.Translate(0.0f, 0.0f, -2.5f);

	modelViewMatrix.PushMatrix();

	//Draw the spinning Torus
	//modelViewMatrix ���������ʲô��;��
	modelViewMatrix.Rotate(yRot, 0.0f, 1.0f, 0.0f);
	shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vLightEyePos, vTorusColor);
	torusBatch.Draw();
	modelViewMatrix.PopMatrix();// "Erase" the Rotation from before

	modelViewMatrix.PushMatrix();
	//��Щ���������������ݣ�����ֱ��������modelViewMatrix�ĵ�ǰ����ע���Ⱥ�˳��
	modelViewMatrix.Rotate(yRot * -2.0f, 0.0f, 1.0f, 0.0f);
	modelViewMatrix.Translate(0.8f, 0.0f, 0.0f);//����ڻ��е�λ���ƶ������в��������������������в�����
	shaderManager.UseStockShader(GLT_SHADER_POINT_LIGHT_DIFF, transformPipeline.GetModelViewMatrix(), transformPipeline.GetProjectionMatrix(), vLightEyePos, vSphereColor);

	sphereBatch.Draw();
	modelViewMatrix.PopMatrix();
	

	
	


	modelViewMatrix.PopMatrix();
	modelViewMatrix.PopMatrix();

	glutSwapBuffers();

	glutPostRedisplay();
}

int main(int argc, char * argv[])
{
	gltSetWorkingDirectory(argv[1]);

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);

	glutCreateWindow("OpenGL SphereWorld");

	glutSpecialFunc(SpecialKeys);

	glutReshapeFunc(ChangeSize);

	glutDisplayFunc(RenderScene);

	GLenum err = glewInit();

	if (GLEW_OK != err)
	{
		fprintf(stderr, "GLEW Error : %s\n", glewGetErrorString(err));
		return 1;
	}


	SetUpRC();

	glutMainLoop();

	return 0;
}