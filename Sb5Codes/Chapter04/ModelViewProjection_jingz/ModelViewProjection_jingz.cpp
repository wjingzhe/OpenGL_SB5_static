

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
	float yRot = rotTimer.GetElapsedSeconds() * 60.0f;//每秒旋转60度？

	//Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	M3DMatrix44f mTranslate, mRotate, mModelView, mModelViewProjection;

	//生成一个平移矩阵，用于将花环移回视野中，有个问题，1.0f会是什么效果？1.0会看不见花托。有可能截头体是世界坐标1.0-1000.0f是指距离camera的。所以此矩阵是相对于摄像机的操作，移到摄像机面前2.5f深度距离处
	//即摄像机所在的地方为原点（0，0，0）
	m3dTranslationMatrix44(mTranslate, 0.0f, 0.0f, -2.5f);

	//生成旋转矩阵 以y为轴，旋转时间t产生的旋转角 yRot
	m3dRotationMatrix44(mRotate, m3dDegToRad(yRot), 0.0f, 1.0f, 0.0f);

	//组合为模型变换矩阵，而原模型矩阵在最右边，即先旋转再平移 ？
	m3dMatrixMultiply44(mModelView, mTranslate, mRotate);

	//组合成模型投影矩阵，而原模型矩阵在最右边，先做模型变换再投影 ？
	m3dMatrixMultiply44(mModelViewProjection, viewFrustum.GetProjectionMatrix(), mModelView);

	GLfloat vBlack[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	//选择要配置的渲染操作类型,非smooth，每种模式对应的参数要熟悉才能正确使用
	shaderManger.UseStockShader(GLT_SHADER_FLAT, mModelViewProjection, vBlack);//黑色背景？不是，是设置当前画笔颜色

	torusBatch.Draw();

	glutSwapBuffers();

	glutPostRedisplay();

	//整个流程是：先旋转在平移，然后进行投影剪裁，最后光栅化。UseStockShader 和draw函数之前的变换管线还是不清晰
}

void SetupRC()
{
	glClearColor(0.8f, 0.8f, 0.8f, 1.0f);

	glEnable(GL_DEPTH_TEST);

	shaderManger.InitializeStockShaders();

	gltMakeTorus(torusBatch, 0.4f, 0.15f, 30, 30);

	//线填充，绘制前后两面
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

}

int main(int argc, char * argv[])
{
	//在某些平台强制将资源放在路径resource目录中
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