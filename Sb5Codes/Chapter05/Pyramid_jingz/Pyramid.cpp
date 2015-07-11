#include <GLTools.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <GLFrustum.h>
#include <GLFrame.h>
#include <GLShaderManager.h>
#include <iostream>

#include <math.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

GLMatrixStack mvpMatrix;
GLMatrixStack projectionMatrix;
GLMatrixStack modelViewMatrix;

GLFrustum viewFrustum;
GLGeometryTransform transformPipeLine;

GLFrame cameraFrame;
GLFrame objectFrame;

GLBatch pyramidBatch;
GLuint textureID;
GLShaderManager shaderManager;

void ChangeSize(int w, int h)
{
	if (h <= 0)
	{
		h = 1;
	}

	glViewport(0, 0, w, h);
	viewFrustum.SetPerspective(35, float(w) / float(h), 1.0f, 1000.0f);
	projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
	transformPipeLine.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

// Load a TGA as a 2D Texture. Completely initialize the state
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
	glTexImage2D(GL_TEXTURE_2D, 0, nComponents, nWidth, nHeight, 0,
		eFormat, GL_UNSIGNED_BYTE, pBits);

	free(pBits);

	if (minFilter == GL_LINEAR_MIPMAP_LINEAR ||
		minFilter == GL_LINEAR_MIPMAP_NEAREST ||
		minFilter == GL_NEAREST_MIPMAP_LINEAR ||
		minFilter == GL_NEAREST_MIPMAP_NEAREST)
		glGenerateMipmap(GL_TEXTURE_2D);

	return true;
}

void MakePyramid(GLBatch & pyramidBatch)
{
	//ÿ�����㻭һ��������
	pyramidBatch.Begin(GL_TRIANGLES, 18, 1);

	//ԭ���ڽ������ļ������ģ���֮���Ƿ��ص㣬�����ĸߵĵ�λ�ã��߶ȵ�1/2��

	//�ײ�,��ʼ����������,����ӳ��Ķ�����Ҫ�鿴ԭʼ�������һ�£��� //todo
	//���ŷ��߿��Ļ�������������Ļ�������ʱ���
	pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
	pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	pyramidBatch.Vertex3f(-1.0f, -1.0f, -1.0f);//���ֵ���������ƽ����������������أ�������������Ӧ��ȴ������������1.0f,-1.0f,1.0f

	pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
	pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	pyramidBatch.Vertex3f(1.0f, -1.0f, -1.0f);

	pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
	pyramidBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
	pyramidBatch.Vertex3f(1.0f, -1.0f, 1.0f);

	//���ŷ��߿��Ļ�������������Ļ�������ʱ���
	pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
	pyramidBatch.MultiTexCoord2f(0, 0.0f, 1.0f);
	pyramidBatch.Vertex3f(-1.0f, -1.0f, 1.0f);

	pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
	pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	pyramidBatch.Vertex3f(-1.0f, -1.0f, -1.0f);

	pyramidBatch.Normal3f(0.0f, -1.0f, 0.0f);
	pyramidBatch.MultiTexCoord2f(0, 1.0f, 1.0f);
	pyramidBatch.Vertex3f(1.0f, -1.0f, 1.0f);

	
	//ԭ�����ߴ����е�front��back�ķ���������ڹ۲������õ�����Ұ
	//��ǰ������澹Ȼ�ǽ������ı��������ǵ��棬����һ�ӽǿ����������
	//�о������꾹Ȼ��

	M3DVector3f vApex = { 0.0f, 1.0f, 0.0f };
	M3DVector3f vFrontLeft = { -1.0f, -1.0f, 1.0f };
	M3DVector3f vFrontRight = { 1.0f, -1.0f, 1.0f };
	M3DVector3f vBackLeft = { -1.0f, -1.0f, -1.0f };
	M3DVector3f vBackRight = { 1.0f, -1.0f, -1.0f };


	M3DVector3f n;

	//����,ǰ
	m3dFindNormal(n, vApex, vFrontLeft, vFrontRight);
	pyramidBatch.Normal3fv(n);
	pyramidBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
	pyramidBatch.Vertex3fv(vApex);

	pyramidBatch.Normal3fv(n);
	pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	pyramidBatch.Vertex3fv(vFrontLeft);

	pyramidBatch.Normal3fv(n);
	pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	pyramidBatch.Vertex3fv(vFrontRight);


	////���ߣ���
	//m3dFindNormal(n, vApex, vBackLeft, vFrontLeft);
	//pyramidBatch.Normal3fv(n);
	//pyramidBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
	//pyramidBatch.Vertex3fv(vApex);

	//pyramidBatch.Normal3fv(n);
	//pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	//pyramidBatch.Vertex3fv(vBackLeft);

	//pyramidBatch.Normal3fv(n);
	//pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	//pyramidBatch.Vertex3fv(vFrontLeft);


	////���ߣ���
	//m3dFindNormal(n, vApex, vFrontRight, vBackRight);
	//pyramidBatch.Normal3fv(n);
	//pyramidBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
	//pyramidBatch.Vertex3fv(vApex);

	//pyramidBatch.Normal3fv(n);
	//pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	//pyramidBatch.Vertex3fv(vFrontRight);

	//pyramidBatch.Normal3fv(n);
	//pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	//pyramidBatch.Vertex3fv(vBackRight);


	////���ߣ���
	//m3dFindNormal(n, vApex, vBackRight, vBackLeft);
	//pyramidBatch.Normal3fv(n);
	//pyramidBatch.MultiTexCoord2f(0, 0.5f, 1.0f);
	//pyramidBatch.Vertex3fv(vApex);

	//pyramidBatch.Normal3fv(n);
	//pyramidBatch.MultiTexCoord2f(0, 0.0f, 0.0f);
	//pyramidBatch.Vertex3fv(vBackRight);

	//pyramidBatch.Normal3fv(n);
	//pyramidBatch.MultiTexCoord2f(0, 1.0f, 0.0f);
	//pyramidBatch.Vertex3fv(vBackLeft);

	pyramidBatch.End();
}

void SetupRC(void)
{
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	
	shaderManager.InitializeStockShaders();

	glEnable(GL_DEPTH_TEST);

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	LoadTGATexture("stone.tga", GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);
	MakePyramid(pyramidBatch);

	cameraFrame.MoveForward(-7.0f);//����ʲô���⣿��
}

void RenderScene(void)
{
	static GLfloat vLightPos[] = { 1.0f, 1.0f, 0.f };
	static GLfloat vWhite[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	modelViewMatrix.PushMatrix();
	//{
		//������λ����أ���ģ��������������λ��

		M3DMatrix44f mCamera;
		cameraFrame.GetCameraMatrix(mCamera);
		modelViewMatrix.MultMatrix(mCamera);
		//modelViewMatrix.Translate(0.0f, -1.0f, -7.0f);


		//����ģ���������ת������
		M3DMatrix44f mObjectFrame;
		objectFrame.GetCameraMatrix(mObjectFrame); //ԭ���еĴ���ΪGetMatrix����ȡ��objectFrame�ĳ��򣬵��¶��������Ķ����ϵ�������෴����
		modelViewMatrix.MultMatrix(mObjectFrame);

		glBindTexture(GL_TEXTURE_2D, textureID);
		shaderManager.UseStockShader(GLT_SHADER_TEXTURE_POINT_LIGHT_DIFF,
			transformPipeLine.GetModelViewMatrix(),
			transformPipeLine.GetProjectionMatrix(),
			vLightPos, vWhite, 0);
		pyramidBatch.Draw();
	//}
	modelViewMatrix.PopMatrix();

	glutSwapBuffers();
}

void ShutdownRC(void)
{
	glDeleteTextures(1, &textureID);
}

void SpecialKeys(int key, int x, int y)
{
	if (key == GLUT_KEY_UP)
	{
		objectFrame.RotateWorld(m3dDegToRad(5.0f),1.0f,0.0f,0.0f);//ģ�ͺ�������Ĳ������෴�ģ�ģ��������ת5.0f
	}

	if (key == GLUT_KEY_DOWN)
	{
		objectFrame.RotateWorld(m3dDegToRad(-5.0f), 1.0f, 0.0f, 0.0f);
	}

	if (key == GLUT_KEY_LEFT)
	{
		objectFrame.RotateWorld(m3dDegToRad(5.0f), 0.0f, 1.0f, 0.0f);
	}

	if (key == GLUT_KEY_RIGHT)
	{
		objectFrame.RotateWorld(m3dDegToRad(-5.0f), 0.0f, 1.0f, 0.0f);
	}

	glutPostRedisplay();
}

int main(int argc, char * argv[])
{
	gltSetWorkingDirectory(argv[0]);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Pyramid");
	glutReshapeFunc(ChangeSize);
	glutSpecialFunc(SpecialKeys);
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