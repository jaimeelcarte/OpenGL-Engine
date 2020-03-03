#include "BOX.h"
#include "auxiliar.h"
#include "PLANE.h"
#include "TRIANGLE.h"
#include "OBJ_Loader.h"

#include <gl/glew.h>
#define SOLVE_FGLUT_WARNING
#include <gl/freeglut.h> 

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp> //Para rotar vectores

#include <iostream>
#include <cstdlib>

#define RAND_SEED 31415926
#define SCREEN_SIZE 500,500

//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

//Matrices
glm::mat4	proj = glm::mat4(1.0f);
glm::mat4	view = glm::mat4(1.0f);
glm::mat4	model = glm::mat4(1.0f);


//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////
float angle = 0.0f;

//Lectura del fichero .obj
objl::Loader loader;
std::vector< objl::Vector3 >  position;
std::vector< objl::Vector3 >  normals;
std::vector< objl::Vector2 >  texCoords;
std::vector<unsigned int> indices;

//Ajustes de la camara
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 10.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

//Opciones de post-procesado
bool postProcessing = true;
bool isQuad = false;
bool usingModel = true;
const char* ppShaderOption = "map"; //


//Parametros Motion Blur
bool constantColor = false;
float alpha = 0.6f;
float red = 0.5f;
float green = 0.5f;
float blue = 0.5f;

//Parametros tessellation
int nSub = 2;

unsigned int fbo;
unsigned int colorBuffTexId;
unsigned int depthBuffTexId;

//VAO
unsigned int vao;

//VBOs que forman parte del objeto
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;

unsigned int colorTexId;
unsigned int emiTexId;

unsigned int planeVAO;
unsigned int planeVertexVBO;
unsigned int triangleVAO;
unsigned int triangleVertexVBO;

//Por definir
unsigned int vshader;
unsigned int fshader;
unsigned int program;

//Variables Uniform 
int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;

//Tessellation 
int uNSub;

//Texturas Uniform
int uColorTex;
int uEmiTex;
int uAlpha; //Indica la transparencia de una textura

//Atributos
int inPos;
int inColor;
int inNormal;
int inTexCoord;

//Post-proceso
unsigned int postProccesVShader;
unsigned int postProccesTCS_Shader;
unsigned int postProccesTES_Shader;
unsigned int postProccesGShader;
unsigned int postProccesFShader;
unsigned int postProccesProgram;

//Uniform
unsigned int uColorTexPP;

//Atributos
int inPosPP;

//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////

//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void resizeFBO(unsigned int w, unsigned int h);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);

void renderCube();
void renderTeapot();

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShaderFw(const char *vname, const char *fname);
void initShaderPP(const char *vname, const char *tcs_name, const char *tes_name, const char *gname, const char *fname);
void initObj();

void initPlane();
void initTriangle();

void initFBO();
void destroy();


//Carga el shader indicado, devuele el ID del shader
//!Por implementar
GLuint loadShader(const char *fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL, 
//y devuelve el identificador de la textura 
//!!Por implementar
unsigned int loadTex(const char *fileName);

//////////////////////////////////////////////////////////////
// Nuevas variables auxiliares
//////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////
// Nuevas funciones auxiliares
//////////////////////////////////////////////////////////////
//!!Por implementar


int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)

	initContext(argc, argv);
	initOGL();
	initShaderFw("../shaders_P4/fwRendering.vert", "../shaders_P4/fwRendering.frag");
	if (postProcessing) {
		if (ppShaderOption == "points")
			initShaderPP("../shaders_P4/pp_points.vert", "../shaders_P4/pp_triangle.tcs", "../shaders_P4/pp_triangle.tes", "../shaders_P4/pp_points.geom", "../shaders_P4/pp_points.frag");
		else if (ppShaderOption == "normals")
			initShaderPP("../shaders_P4/pp_normals.vert", "../shaders_P4/pp_triangle.tcs", "../shaders_P4/pp_triangle.tes", "../shaders_P4/pp_normals.geom", "../shaders_P4/pp_normals.frag");
		else if (ppShaderOption == "wired")
		{
			if (isQuad)
				initShaderPP("../shaders_P4/pp_wired.vert", "../shaders_P4/pp_quad.tcs", "../shaders_P4/pp_quad.tes", "../shaders_P4/pp_wired.geom", "../shaders_P4/pp_wired.frag");
			else
				initShaderPP("../shaders_P4/pp_wired.vert", "../shaders_P4/pp_triangle.tcs", "../shaders_P4/pp_triangle.tes", "../shaders_P4/pp_wired.geom", "../shaders_P4/pp_wired.frag");
		}
		else if(ppShaderOption == "map")
			initShaderPP("../shaders_P4/pp_map.vert", "../shaders_P4/pp_map.tcs", "../shaders_P4/pp_map.tes", "../shaders_P4/pp_map.geom", "../shaders_P4/pp_map.frag");
			
	}
	

	loader = objl::Loader();
	loader.LoadFile("../obj/teapot2.obj");

	for (int i = 0; i < loader.LoadedVertices.size(); ++i)
	{
		position.push_back(loader.LoadedVertices[i].Position);
		//std::cout << position[i].X << ", " << position[i].Y << ", " << position[i].Z << std::endl;
		normals.push_back(loader.LoadedVertices[i].Normal);
		texCoords.push_back(loader.LoadedVertices[i].TextureCoordinate);
	}

	initObj();
	initPlane();
	initTriangle();
	initFBO();
	
	glutMainLoop();

	destroy();

	return 0;
}

//////////////////////////////////////////
// Funciones auxiliares 
void initContext(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	//glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(SCREEN_SIZE);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Prácticas GLSL");

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}

	const GLubyte *oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

	glutReshapeFunc(resizeFunc);
	if(ppShaderOption == "map")
		glutDisplayFunc(renderTeapot);
	else
		glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);
}

void initOGL()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	proj = glm::perspective(glm::radians(60.0f), 1.0f, 1.0f, 50.0f);
	view = glm::mat4(1.0f);
	view[3].z = -25.0f;
}


void destroy()
{
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(program);

	glDetachShader(postProccesProgram, postProccesVShader);
	glDetachShader(postProccesProgram, postProccesFShader);
	if (postProcessing) {
		glDetachShader(program, postProccesGShader);
		glDeleteShader(postProccesGShader);
	}
	glDeleteShader(postProccesVShader);
	glDeleteShader(postProccesFShader);
	glDeleteProgram(postProccesProgram);

	if (inPos != -1) glDeleteBuffers(1, &posVBO);
	if (inColor != -1) glDeleteBuffers(1, &colorVBO);
	if (inNormal != -1) glDeleteBuffers(1, &normalVBO);
	if (inTexCoord != -1) glDeleteBuffers(1, &texCoordVBO);
	glDeleteBuffers(1, &triangleIndexVBO);

	glDeleteVertexArrays(1, &vao);

	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &emiTexId);

	glDeleteBuffers(1, &planeVertexVBO);
	glDeleteVertexArrays(1, &planeVAO);

	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &colorBuffTexId);
	glDeleteTextures(1, &depthBuffTexId);
}

void initShaderFw(const char *vname, const char *fname)
{
	vshader = loadShader(vname, GL_VERTEX_SHADER);
	fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);

	glBindAttribLocation(program, 0, "inPos");
	glBindAttribLocation(program, 1, "inColor");
	glBindAttribLocation(program, 2, "inNormal");
	glBindAttribLocation(program, 3, "inTexCoord");


	glLinkProgram(program);

	int linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);

		char *logString = new char[logLen];
		glGetProgramInfoLog(program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;

		glDeleteProgram(program);
		program = 0;
		exit(-1);
	}

	uNormalMat = glGetUniformLocation(program, "normal");
	uModelViewMat = glGetUniformLocation(program, "modelView");
	uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");

	uColorTex = glGetUniformLocation(program, "colorTex");
	uEmiTex = glGetUniformLocation(program, "emiTex");

	inPos = glGetAttribLocation(program, "inPos");
	inColor = glGetAttribLocation(program, "inColor");
	inNormal = glGetAttribLocation(program, "inNormal");
	inTexCoord = glGetAttribLocation(program, "inTexCoord");
}

void initShaderPP(const char *vname, const char *tcs_name, const char *tes_name, const char *gname, const char *fname)
{
	postProccesVShader = loadShader(vname, GL_VERTEX_SHADER);
	postProccesTCS_Shader = loadShader(tcs_name, GL_TESS_CONTROL_SHADER);
	postProccesTES_Shader = loadShader(tes_name, GL_TESS_EVALUATION_SHADER);
	postProccesGShader = loadShader(gname, GL_GEOMETRY_SHADER);
	postProccesFShader = loadShader(fname, GL_FRAGMENT_SHADER);

	postProccesProgram = glCreateProgram();
	glAttachShader(postProccesProgram, postProccesVShader);
	glAttachShader(postProccesProgram, postProccesTCS_Shader);
	glAttachShader(postProccesProgram, postProccesTES_Shader);
	glAttachShader(postProccesProgram, postProccesGShader);
	glAttachShader(postProccesProgram, postProccesFShader);

	glBindAttribLocation(postProccesProgram, 0, "inPos"); 
	glBindAttribLocation(postProccesProgram, 1, "inColor");
	glBindAttribLocation(postProccesProgram, 2, "inNormal");
	glBindAttribLocation(postProccesProgram, 3, "inTexCoord");

	glLinkProgram(postProccesProgram);
	int linked;
	glGetProgramiv(postProccesProgram, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(postProccesProgram, GL_INFO_LOG_LENGTH, &logLen);
		char *logString = new char[logLen];
		glGetProgramInfoLog(postProccesProgram, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete logString;
		glDeleteProgram(postProccesProgram);
		postProccesProgram = 0;
		exit(-1);
	}
	uColorTexPP = glGetUniformLocation(postProccesProgram, "colorTex");
	uEmiTex = glGetUniformLocation(postProccesProgram, "emiTex");
	uNSub = glGetUniformLocation(postProccesProgram, "nSub");
	uAlpha = glGetUniformLocation(postProccesProgram, "alpha");

	inPosPP = glGetAttribLocation(postProccesProgram, "inPos");
	inColor = glGetAttribLocation(postProccesProgram, "inColor");
	inNormal = glGetAttribLocation(postProccesProgram, "inNormal");
	inTexCoord = glGetAttribLocation(postProccesProgram, "inTexCoord");

	glUseProgram(postProccesProgram);
	if (uColorTexPP != -1)
		glUniform1i(uColorTexPP, 0);
}

void initObj()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	if (inPos != -1)
	{
		glGenBuffers(1, &posVBO);
		glBindBuffer(GL_ARRAY_BUFFER, posVBO);
		glBufferData(GL_ARRAY_BUFFER, position.size() *sizeof(float) * 3,
			&position[0], GL_STATIC_DRAW);
		glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos);
	}

	if (inColor != -1)
	{
		glGenBuffers(1, &colorVBO);
		glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex*sizeof(float) * 3,
			cubeVertexColor, GL_STATIC_DRAW);
		glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inColor);
	}

	if (inNormal != -1)
	{
		glGenBuffers(1, &normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
		glBufferData(GL_ARRAY_BUFFER, normals.size() *sizeof(float) * 3,
			&normals[0], GL_STATIC_DRAW);
		glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inNormal);
	}


	if (inTexCoord != -1)
	{
		glGenBuffers(1, &texCoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
		glBufferData(GL_ARRAY_BUFFER, texCoords.size() *sizeof(float) * 2,
			&texCoords[0], GL_STATIC_DRAW);
		glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inTexCoord);
	}

	glGenBuffers(1, &triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		(loader.LoadedIndices.size() / 3) *sizeof(unsigned int) * 3, &loader.LoadedIndices[0],
		GL_STATIC_DRAW);

	model = glm::mat4(1.0f);

	colorTexId = loadTex("../img/mapamundi.jpg");
	emiTexId = loadTex("../img/emissive.png");
}

void initPlane()
{
	glGenVertexArrays(1, &planeVAO);
	glBindVertexArray(planeVAO);

	glGenBuffers(1, &planeVertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVertexVBO);

	glBufferData(GL_ARRAY_BUFFER, planeNVertex * sizeof(float) * 3,
		planeVertexPos, GL_STATIC_DRAW); //Reserva memoria de la tarjeta gráfica y subo datos relativos al plano
	glVertexAttribPointer(inPosPP, 3, GL_FLOAT, GL_FALSE, 0, 0); //Le dije a que atributo asignar las posiciones del vertice
	glEnableVertexAttribArray(inPosPP); 

}

void initTriangle()
{

	glGenVertexArrays(1, &triangleVAO);
	glBindVertexArray(triangleVAO);

	glGenBuffers(1, &triangleVertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, triangleVertexVBO);

	glBufferData(GL_ARRAY_BUFFER, triangleNVertex * sizeof(float) * 3,
		triangleVertexPos, GL_STATIC_DRAW); //Reserva memoria de la tarjeta gráfica y subo datos relativos al plano
	glVertexAttribPointer(inPosPP, 3, GL_FLOAT, GL_FALSE, 0, 0); //Le dije a que atributo asignar las posiciones del vertice
	glEnableVertexAttribArray(inPosPP);

}

GLuint loadShader(const char *fileName, GLenum type)
{
	unsigned int fileLen;
	char *source = loadStringFromFile(fileName, fileLen);

	//////////////////////////////////////////////
	//Creación y compilación del Shader
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1,
		(const GLchar **)&source, (const GLint *)&fileLen);
	glCompileShader(shader);
	delete[] source;

	//Comprobamos que se compilo bien
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);

		char *logString = new char[logLen];
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;

		glDeleteShader(shader);
		exit(-1);
	}

	return shader;
}

unsigned int loadTex(const char *fileName)
{
	unsigned char *map;
	unsigned int w, h;
	map = loadTexture(fileName, w, h);

	if (!map)
	{
		std::cout << "Error cargando el fichero: "
			<< fileName << std::endl;
		exit(-1);
	}

	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, (GLvoid*)map);
	delete[] map;
	glGenerateMipmap(GL_TEXTURE_2D);

	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

	return texId;
}
void renderFunc()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0); //Primero lo activo y luego lo limpio
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	/**/
	glUseProgram(program);

	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * view * model;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));
	if (uModelViewMat != -1) //Si está utilizando dicha matriz, la subo a los shaders activados en el programa
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE,
			&(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE,
			&(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE,
			&(normal[0][0]));

	//Texturas
	if (uColorTex != -1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTexId);
		glUniform1i(uColorTex, 0); //No tiene que ver con la textura, sino con el shader
	}

	if (uEmiTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, emiTexId);
		glUniform1i(uEmiTex, 1);
	}

	if (usingModel)
	{
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, loader.LoadedIndices.size() * 3,
			GL_UNSIGNED_INT, (void*)0);
	}
	
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0); //Activo el fbo por defecto (para que se pueda ver por pantalla)

	if (postProcessing) {
		glUseProgram(postProccesProgram);

		if (uModelViewMat != -1) //Si está utilizando dicha matriz, la subo a los shaders activados en el programa
			glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE,
				&(modelView[0][0]));
		if (uModelViewProjMat != -1)
			glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE,
				&(modelViewProj[0][0]));
		if (uNormalMat != -1)
			glUniformMatrix4fv(uNormalMat, 1, GL_FALSE,
				&(normal[0][0]));
		if (uNSub != -1)
			glUniform1i(uNSub, nSub);
		/*
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		//Variable uniform alpha que se envía al shader
		if (uAlpha != -1)
			glUniform1f(uAlpha, alpha);

		glEnable(GL_BLEND);
		//Control de parámetros
		if (constantColor == false)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Factor fuente (alfa) y factor destino (1-alfa)
		glBlendEquation(GL_FUNC_ADD);
		if (constantColor == true)
			glBlendFunc(GL_CONSTANT_COLOR, GL_CONSTANT_ALPHA);
		glBlendColor(red, green, blue, 0.6); //Da mas control que la linea comentada arriba
		glBlendEquation(GL_FUNC_ADD);


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffTexId); //No está cambiando para nada el estado del shader
		*/
		
		

		

		//Prueba con el quad
		
		if (isQuad)
		{

			glPatchParameteri(GL_PATCH_VERTICES, 4); //Definimos el numero de patch

			if (usingModel)
			{
				glPointSize(5.0f);
				glBindVertexArray(vao);
				glDrawElements(GL_PATCHES, loader.LoadedIndices.size() * 3,
					GL_UNSIGNED_INT, (void*)0); //Desde el vertice 0, pintamos 4 vertices
			}
			else 
			{
				glBindVertexArray(planeVAO);
				glDrawArrays(GL_PATCHES, 0, 4); //Es necesario utilizar la topología patch
			}
		}
		else
		{
			glPatchParameteri(GL_PATCH_VERTICES, 3); //Definimos el numero de patch
			if (usingModel)
			{
				glPointSize(5.0f);
				glBindVertexArray(vao);
				glDrawElements(GL_PATCHES, loader.LoadedIndices.size() * 3,
					GL_UNSIGNED_INT, (void*)0); //Desde el vertice 0, pintamos 4 vertices
			}
			else
			{
				glBindVertexArray(triangleVAO);
				glDrawArrays(GL_PATCHES, 0, 3); //Es necesario utilizar la topología patch
			}
		}
		

		/*
		glDisable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		*/
		
	}
	glutSwapBuffers();
	
}

void renderTeapot()
{
	/**/
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(postProccesProgram);
	
	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * view * model;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));
	if (uModelViewMat != -1) //Si está utilizando dicha matriz, la subo a los shaders activados en el programa
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE,
			&(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE,
			&(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE,
			&(normal[0][0]));
	if (uNSub != -1)
		glUniform1i(uNSub, nSub);

	//Texturas
	if (uColorTex != -1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTexId);
		glUniform1i(uColorTex, 0); //No tiene que ver con la textura, sino con el shader
	}

	if (uEmiTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, emiTexId);
		glUniform1i(uEmiTex, 1);
	}

	glPatchParameteri(GL_PATCH_VERTICES, 3); //Definimos el numero de patch
	glPointSize(5.0f);
	glBindVertexArray(vao);
	glDrawElements(GL_PATCHES, loader.LoadedIndices.size() * 3,
		GL_UNSIGNED_INT, (void*)0); //Desde el vertice 0, pintamos 4 vertices
	
	glutSwapBuffers();

}

void renderCube()
{
	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * view * model;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE,
		&(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE,
		&(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE,
		&(normal[0][0]));
	
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3,
		GL_UNSIGNED_INT, (void*)0);
}



void resizeFunc(int width, int height)
{
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(60.0f), float(width) /float(height), 1.0f, 50.0f);

	resizeFBO(width, height);

	glutPostRedisplay();
}

void idleFunc()
{
	angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.002f;
	
	glutPostRedisplay();
}

void initFBO()
{
	glGenFramebuffers(1, &fbo); //El numero indica cuantos fbo's quiero crear
	glGenTextures(1, &colorBuffTexId);
	glGenTextures(1, &depthBuffTexId);

	resizeFBO(SCREEN_SIZE);

}

void resizeFBO(unsigned int w, unsigned int h)
{
	glBindTexture(GL_TEXTURE_2D, colorBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
		GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, depthBuffTexId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0,
		GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, colorBuffTexId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
		depthBuffTexId, 0);
	const GLenum buffs[1] = {GL_COLOR_ATTACHMENT0 }; //El attachment se coloca en la posicion 1 que luego se indica en el shader de fragmentos
	glDrawBuffers(1, buffs);
	if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER))
	{
		std::cerr << "Error configurando el FBO" << std::endl;
		exit(-1);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0); //El 0 es el default frame buffer. NO HACER! 
}

void keyboardFunc(unsigned char key, int x, int y)
{
	/*
	float inc = 0.1;
	if (key == 'a') constantColor = false; //Si pulso la a es que voy a modificar el parametro alfa del blend
	if (key == 'c') constantColor = true;//Si pulso la a es que voy a modificar los colores constantes del blend
	else if (key == '+') alpha = alpha + inc;
	else if (key == '-') alpha = alpha - inc;
	else if (key == 'r') red = red + inc;
	else if (key == 'g') green = green + inc;
	else if (key == 'b') blue = blue + inc;
	*/
	float angulo = 0.0f;
	float cameraSpeed = 1.0;
	float cameraRotate = 0.1;

	if (key == 'j')
	{
		angulo = (angulo < 6.2830f) ? angulo + cameraRotate : 0.0f; //? true:false
		cameraFront = glm::rotate(cameraFront, angulo, glm::vec3(0.0f, 1.0f, 0.0f));
	}
	else if (key == 'l')
	{
		angulo = (angulo < 6.2830f) ? angulo - cameraRotate : 0.0f;
		cameraFront = glm::rotate(cameraFront, angulo, glm::vec3(0.0f, 1.0f, 0.0f));
	}
	
	else if (key == 'm') cameraPos += cameraSpeed * cameraUp;
	else if (key == 'n') cameraPos -= cameraSpeed * cameraUp;
	else if (key == 'w') cameraPos += cameraSpeed * cameraFront;
	else if (key == 's') cameraPos -= cameraSpeed * cameraFront;
	else if (key == 'a') cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	else if (key == 'd') cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	else if (key == 'p' && postProcessing) postProcessing = false;
	else if (key == 'p' && !postProcessing) postProcessing = true;
	else if (key == '1') ppShaderOption = "points";
	else if (key == '2') ppShaderOption = "normals";
	else if (key == '3') ppShaderOption = "wired";
	else if (key == '+') nSub++;
	else if (key == '-') nSub--;
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}
void mouseFunc(int button, int state, int x, int y){}

