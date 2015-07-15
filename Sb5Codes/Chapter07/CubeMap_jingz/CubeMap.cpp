#include <GLTools.h>
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

GLFrame viewFrame;
GLFrustum viewFrustum;
GLMatrixStack projctionMatrix;
GLMatrixStack mvpMatrix;
GLMatrixStack modelViewMatrix;
GLGeometryTransform transformPipeLine;

GLTriangleBatch sphereBatch;
GLBatch cubeBatch;

GLuint tarnishTexture;
GLuint cubeTexture;
GLuint reflectionShader;
GLuint skyBoxShader;

GLint locMVPReflect;
GLint locMVReflect;
GLint locNormalReflect;
GLint locInvertedCamera;

GLint locCubeMap, locTarnishMap;

GLint locMVPSkyBox;

// Six sides of a cube map
const char *szCubeFaces[6] = { "pos_x.tga", "neg_x.tga", "pos_y.tga", "neg_y.tga", "pos_z.tga", "neg_z.tga" };

GLenum cube[6] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
};

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


void SetupRC(void)
{
	GLbyte * pBytes;
	int nWidth, nHeight, nComponent;
	GLenum eFormat;

	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Load the tarnish texture
	glGenTextures(1, &tarnishTexture);
	glBindTexture(GL_TEXTURE_2D, tarnishTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	pBytes = gltReadTGABits("tarnish.tga", &nWidth, &nHeight, &nComponent, &eFormat);
	glTexImage2D(GL_TEXTURE_2D, 0, nComponent, nWidth, nHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
	free(pBytes);
	glGenerateMipmap(GL_TEXTURE_2D);

	glGenTextures(1, &cubeTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	

	for (size_t i = 0; i < 6; ++i)
	{
		pBytes = gltReadTGABits(szCubeFaces[i], &nWidth, &nHeight, &nComponent, &eFormat);
		// border 为0，是指边框的宽度
		//eFormat 不需要 和internalFormat取值必须相同，都是指定纹理中的颜色组件
		glTexImage2D(cube[i], 0, nComponent, nWidth, nHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
		free(pBytes);
	}
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	viewFrame.MoveForward(-4.0f);
	gltMakeSphere(sphereBatch, 1.0f, 52, 26);
	gltMakeCube(cubeBatch, 20.0f);

	reflectionShader = gltLoadShaderPairWithAttributes("Reflection.vp", "Reflection.fp", 3, 
		GLT_ATTRIBUTE_VERTEX, "vVertex", 
		GLT_ATTRIBUTE_NORMAL, "vNormal", 
		GLT_ATTRIBUTE_TEXTURE0, "vTexCoords");
	//uniform值绑定
	locMVPReflect = glGetUniformLocation(reflectionShader, "mvpMatrix");
	locMVReflect = glGetUniformLocation(reflectionShader, "mvMatrix");
	locNormalReflect = glGetUniformLocation(reflectionShader, "normalMatrix");
	locInvertedCamera = glGetUniformLocation(reflectionShader, "mInverseCamera");

	locCubeMap = glGetUniformLocation(reflectionShader, "cubeMap");
	locTarnishMap = glGetUniformLocation(reflectionShader, "tarnishMap");

	skyBoxShader = gltLoadShaderPairWithAttributes("skyBox.vp", "skyBox.fp", 1, GLT_ATTRIBUTE_VERTEX, "vVertex");
	//uniform值绑定

	locMVPSkyBox = glGetUniformLocation(skyBoxShader, "mvpMatrix");


	//启用两段纹理 与顶点着色器输入输出的纹理数组对应
	// glUniform1i(locCubeMap, 0);
	// glUniform1i(locTarnishMap, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tarnishTexture);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);
}

void RenderScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	M3DMatrix44f mCamera;
	M3DMatrix44f mCameraRotOnly;
	M3DMatrix44f mInverseCamera;

	viewFrame.GetCameraMatrix(mCamera, false);
	viewFrame.GetCameraMatrix(mCameraRotOnly, true);
	m3dInvertMatrix44(mInverseCamera, mCameraRotOnly);

	modelViewMatrix.PushMatrix();
	{
		
		modelViewMatrix.MultMatrix(mCamera);//向后移动 4.0f，惯性坐标系的向后则是z变小，模型变小
		glUseProgram(reflectionShader);

		glUniformMatrix4fv(locMVReflect, 1, GL_FALSE, transformPipeLine.GetModelViewMatrix());
		glUniformMatrix4fv(locMVPReflect, 1, GL_FALSE, transformPipeLine.GetModelViewProjectionMatrix());
		glUniformMatrix3fv(locNormalReflect, 1, GL_FALSE, transformPipeLine.GetNormalMatrix());
		glUniformMatrix4fv(locInvertedCamera, 1, GL_FALSE, mInverseCamera);

		glUniform1i(locCubeMap, 0);
		glUniform1i(locTarnishMap, 1);

		glEnable(GL_CULL_FACE);
		sphereBatch.Draw();
		glDisable(GL_CULL_FACE);
	}
	modelViewMatrix.PopMatrix();

	modelViewMatrix.PushMatrix();
	{
		modelViewMatrix.MultMatrix(mCameraRotOnly);
		glUseProgram(skyBoxShader);
		glUniformMatrix4fv(locMVPSkyBox, 1, GL_FALSE, transformPipeLine.GetModelViewProjectionMatrix());
		cubeBatch.Draw();
	}
	modelViewMatrix.PopMatrix();

	glutSwapBuffers();
}

void SpecialKeys(int key, int x, int y)
{
	float linear = 1.0f;
	float angular = float(m3dDegToRad(5.0f));

	if (key == GLUT_KEY_UP)
		viewFrame.MoveForward(linear);

	if (key == GLUT_KEY_DOWN)
		viewFrame.MoveForward(-linear);

	if (key == GLUT_KEY_LEFT)
		viewFrame.RotateWorld(angular, 0.0f, 1.0f, 0.0f);

	if (key == GLUT_KEY_RIGHT)
		viewFrame.RotateWorld(-angular, 0.0f, 1.0f, 0.0f);

	glutPostRedisplay();
}

void ShutdownRC(void)
{
	glDeleteTextures(1, &tarnishTexture);
	glDeleteTextures(1, &cubeTexture);
	glDeleteProgram(reflectionShader);
	glDeleteProgram(skyBoxShader);
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("OpenGL Cube Maps");
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);
	glutSpecialFunc(SpecialKeys);

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