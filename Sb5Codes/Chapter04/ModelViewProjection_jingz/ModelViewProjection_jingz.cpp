

#include <GLTools.h>
#include <GLMatrixStack.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <StopWatch.h>

#include <math.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

GLFrustum             viewFrustum;
GLShaderManager       shaderManger;
GLTriangleBatch       torusBatch;

void ChangeSize(int w, int h)
{
	//prevent divide by 0
	if (h == 0)
	{
		h = 1;
	}

	//
	glViewport(0, 0, w, h);

	viewFrustum.SetPerspective(35.0f, float(w) / float(h), 1.0f, 1000.0f);

}

void RenderScence(void)
{

	static CStopWatch rotTimer;
	float yRot = rotTimer.GetElapsedSeconds() * 60.0f;//ÿ����ת60�ȣ�

	//Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	M3DMatrix44f mTranslate, mRotate, mModelView, mModelViewProjection;

	//����һ��ƽ�ƾ������ڽ������ƻ���Ұ�У��и����⣬1.0f����ʲôЧ����1.0�ῴ�������С��п��ܽ�ͷ������������1.0-1000.0f��ָ����camera�ġ����Դ˾����������������Ĳ������Ƶ��������ǰ2.5f��Ⱦ��봦
	//����������ڵĵط�Ϊԭ�㣨0��0��0��
	m3dTranslationMatrix44(mTranslate, 0.0f, 0.0f, -2.5f);

	//������ת���� ��yΪ�ᣬ��תʱ��t��������ת�� yRot
	m3dRotationMatrix44(mRotate, m3dDegToRad(yRot), 0.0f, 1.0f, 0.0f);

	//���Ϊģ�ͱ任���󣬶�ԭģ�;��������ұߣ�������ת��ƽ�� ��
	m3dMatrixMultiply44(mModelView, mTranslate, mRotate);

	//��ϳ�ģ��ͶӰ���󣬶�ԭģ�;��������ұߣ�����ģ�ͱ任��ͶӰ ��
	m3dMatrixMultiply44(mModelViewProjection, viewFrustum.GetProjectionMatrix(), mModelView);

	GLfloat vBlack[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	//ѡ��Ҫ���õ���Ⱦ��������,��smooth��ÿ��ģʽ��Ӧ�Ĳ���Ҫ��Ϥ������ȷʹ��
	shaderManger.UseStockShader(GLT_SHADER_FLAT, mModelViewProjection, vBlack);//��ɫ���������ǣ������õ�ǰ������ɫ

	torusBatch.Draw();

	glutSwapBuffers();

	glutPostRedisplay();

	//���������ǣ�����ת��ƽ�ƣ�Ȼ�����ͶӰ���ã�����դ����UseStockShader ��draw����֮ǰ�ı任���߻��ǲ�����
}

void SetupRC()
{
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

	glEnable(GL_DEPTH_TEST);

	shaderManger.InitializeStockShaders();

	gltMakeTorus(torusBatch, 0.4f, 0.15f, 30, 30);

	//����䣬����ǰ������
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

}

int main(int argc, char * argv[])
{
	//��ĳЩƽ̨ǿ�ƽ���Դ����·��resourceĿ¼��
	gltSetWorkingDirectory(argv[0]);

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);

	glutInitWindowSize(800, 600);

	glutCreateWindow("ModeleViewPRojection Example");

	glutReshapeFunc(ChangeSize);

	glutDisplayFunc(RenderScence);


	GLenum err = glewInit();

	if (GLEW_OK != err)
	{
		fprintf(stderr, "GLEW Error:%s/n", glewGetErrorString(err));
		return 1;
	}

	SetupRC();

	glutMainLoop();

	return 0;
}