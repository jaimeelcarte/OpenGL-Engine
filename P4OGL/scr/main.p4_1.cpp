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
#define SCREEN_SIZE 1280,720

#define NUM_PARTICLES 1024
#define WORK_GROUP_SIZE 128

//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

//Matrices
glm::mat4	proj = glm::mat4(1.0f);
glm::mat4	view = glm::mat4(1.0f);
glm::mat4	model = glm::mat4(1.0f);

//Estructuras
struct pos
{
	float x, y, z, w;
};
struct vel
{
	float vx, vy, vz, vw;
};
struct color
{
	float r, g, b, a;
};

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
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 25.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

//Opciones integrador shader de computo
int usingVerlet = 1; //1: true           0: false

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
float dispFactor = 0.0f;
int nSub = 1;

unsigned int fbo;
unsigned int colorBuffTexId;
unsigned int depthBuffTexId;

//SSBO
unsigned int posSSBO;
unsigned int prevPosSSBO;
unsigned int velSSBO;
unsigned int colSSBO;

//VAO
unsigned int vao;
unsigned int vaoC;

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
unsigned int gshader;
unsigned int fshader;
unsigned int program;

unsigned int cshader;
unsigned int programCompute;

//Variables Uniform 
int uModelMat;
int uModelViewMat;
int uViewProjMat;
int uModelViewProjMat;
int uNormalMat;
int uUsingVerlet;

//Tessellation 
int uDispFactor;
int uNSub;
int uCameraPos;

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
void renderParticles();

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShaderFw(const char *vname, const char *fname);
void initShaderParticle(const char *vname, const char *gname ,const char *fname);
void initShaderPP(const char *vname, const char *tcs_name, const char *tes_name, const char *gname, const char *fname);
void initComputeShader(const char *cname);
void initObj();

void initPlane();
void initTriangle();
void initStructure();

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
float Ranf(float Min, float Max);


int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)

	initContext(argc, argv);
	initOGL();
	//initShaderFw("../shaders_P4/fwRendering.vert", "../shaders_P4/fwRendering.frag");
	initShaderParticle("../shaders_P4/Modulo2/billboard.vert", "../shaders_P4/Modulo2/billboard.geom" , "../shaders_P4/Modulo2/billboard.frag");
	initComputeShader("../shaders_P4/Modulo2/particleSystem.comp");

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
		normals.push_back(loader.LoadedVertices[i].Normal);
		texCoords.push_back(loader.LoadedVertices[i].TextureCoordinate);
	}

	initObj();
	initPlane();
	initTriangle();
	initStructure();
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
	glutDisplayFunc(renderParticles);
	/*
	if(ppShaderOption == "map")
		glutDisplayFunc(renderTeapot);
	else
		glutDisplayFunc(renderFunc);
	*/
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

	proj = glm::perspective(glm::radians(60.0f), 16.0f/9.0f, 0.1f, 100.0f);
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
		glDetachShader(program, postProccesTCS_Shader);
		glDetachShader(program, postProccesTES_Shader);
		glDeleteShader(postProccesGShader);
		glDeleteShader(postProccesTCS_Shader);
		glDeleteShader(postProccesTES_Shader);
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

void initShaderParticle(const char *vname, const char *gname, const char *fname)
{
	vshader = loadShader(vname, GL_VERTEX_SHADER);
	gshader = loadShader(gname, GL_GEOMETRY_SHADER);
	fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, gshader);
	glAttachShader(program, fshader);


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
	uModelViewProjMat = glGetUniformLocation(program, "proj");

	uColorTex = glGetUniformLocation(program, "colorTex");
	uEmiTex = glGetUniformLocation(program, "emiTex");


	
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
	
	uModelMat = glGetUniformLocation(postProccesProgram, "model");
	uViewProjMat = glGetUniformLocation(postProccesProgram, "viewProj");

	uColorTexPP = glGetUniformLocation(postProccesProgram, "colorTex");
	uEmiTex = glGetUniformLocation(postProccesProgram, "emiTex");
	uNSub = glGetUniformLocation(postProccesProgram, "nSub");
	uDispFactor = glGetUniformLocation(postProccesProgram, "dispFactor");
	uAlpha = glGetUniformLocation(postProccesProgram, "alpha");
	uCameraPos = glGetUniformLocation(postProccesProgram, "cameraPos");

	inPosPP = glGetAttribLocation(postProccesProgram, "inPos");
	inColor = glGetAttribLocation(postProccesProgram, "inColor");
	inNormal = glGetAttribLocation(postProccesProgram, "inNormal");
	inTexCoord = glGetAttribLocation(postProccesProgram, "inTexCoord");

	glUseProgram(postProccesProgram);
	if (uColorTexPP != -1)
		glUniform1i(uColorTexPP, 0);
}

void initComputeShader(const char *cname)
{
	cshader = loadShader(cname, GL_COMPUTE_SHADER);

	programCompute = glCreateProgram();
	glAttachShader(programCompute, cshader);
	glLinkProgram(programCompute);

	int linked;
	glGetProgramiv(programCompute, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(programCompute, GL_INFO_LOG_LENGTH, &logLen);
		char *logString = new char[logLen];
		glGetProgramInfoLog(programCompute, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete logString;
		glDeleteProgram(programCompute);
		programCompute = 0;
		exit(-1);
	}


	uUsingVerlet = glGetUniformLocation(programCompute, "usingVerlet");

	

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

	colorTexId = loadTex("../img/pokeball.png");
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

void initStructure()
{
	float h = NUM_PARTICLES / 4;


	glGenBuffers(1, &posSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct pos), NULL, GL_STATIC_DRAW);

	unsigned int bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

	struct pos *points = (struct pos *) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct pos), bufMask);
	for (int i = 0; i < NUM_PARTICLES; ++i)
	{

		if (i < h)
		{
			points[i].x = 9.0;
			points[i].y = 10.0;
			points[i].z = 0.0;
			points[i].w = 1;
		}
		else if (i < (h * 2))
		{
			points[i].x = 2.0;
			points[i].y = 5.0;
			points[i].z = 3.0;
			points[i].w = 1;
		}
		else if (i < (h * 3))
		{
			points[i].x = -15.0;
			points[i].y = 7.0;
			points[i].z = -3.0;
			points[i].w = 1;
		}
		else if (i < (h * 4))
		{
			points[i].x = -6.0;
			points[i].y = 6.0;
			points[i].z = -5.0;
			points[i].w = 1;
		}
		
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	if (uUsingVerlet != -1)
	{
		glGenBuffers(1, &prevPosSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, prevPosSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct pos), NULL, GL_STATIC_DRAW);

		unsigned int bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

		struct pos *pointsPrev = (struct pos *) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct pos), bufMask);
		for (int i = 0; i < NUM_PARTICLES; ++i)
		{
			pointsPrev[i].x = 0.0;
			pointsPrev[i].y = 0.0;
			pointsPrev[i].z = 0.0;
			pointsPrev[i].w = 0.0;

		}
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, prevPosSSBO);
	}

	glGenBuffers(1, &velSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct vel), NULL, GL_STATIC_DRAW);

	struct vel *vels = (struct vel *) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct vel), bufMask);
	for (int i = 0; i < NUM_PARTICLES; ++i)
	{
		//De momento hardcodeo el random de los elementos
		float intervalo[] = { -10.0, 10.0 };
		vels[i].vx = Ranf(intervalo[0], intervalo[1]);
		vels[i].vy = Ranf(intervalo[0], intervalo[1]);
		vels[i].vz = Ranf(intervalo[0], intervalo[1]);
		vels[i].vw = 0;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	glGenBuffers(1, &colSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, colSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(struct vel), NULL, GL_STATIC_DRAW);

	struct color *cols = (struct color *) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct color), bufMask);
	for (int i = 0; i < NUM_PARTICLES; ++i)
	{
		//De momento hardcodeo el random de los elementos
		cols[i].r = Ranf(0.0, 1.0);
		cols[i].g = Ranf(0.0, 1.0);
		cols[i].b = Ranf(0.0, 1.0);
		cols[i].a = Ranf(0.0, 1.0);
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, posSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, velSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, colSSBO);

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
		if (uDispFactor != -1)
			glUniform1f(uDispFactor, dispFactor);
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
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(postProccesProgram);
	
	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * model;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));
	glm::mat4 viewProj = proj * view;
	glm::vec4 camPos = model * glm::vec4(cameraPos, 1.0);
	if (uModelMat != -1) //Si está utilizando dicha matriz, la subo a los shaders activados en el programa
		glUniformMatrix4fv(uModelMat, 1, GL_FALSE,
			&(model[0][0]));
	if (uModelViewMat != -1) //Si está utilizando dicha matriz, la subo a los shaders activados en el programa
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE,
			&(modelView[0][0]));
	if (uViewProjMat != -1) //Si está utilizando dicha matriz, la subo a los shaders activados en el programa
		glUniformMatrix4fv(uViewProjMat, 1, GL_FALSE,
			&(viewProj[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE,
			&(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE,
			&(normal[0][0]));
	if (uDispFactor != -1)
		glUniform1f(uDispFactor, dispFactor);
	if (uNSub != -1)
		glUniform1i(uNSub, nSub);
	if (uCameraPos != -1)
		glUniform3fv(uCameraPos, 1, &(camPos[0]));

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

	

	// Dynamic LODs

	glPatchParameteri(GL_PATCH_VERTICES, 3); //Definimos el numero de patch
	glPointSize(5.0f);
	glBindVertexArray(vao);
	glDrawElements(GL_PATCHES, loader.LoadedIndices.size() * 3,
		GL_UNSIGNED_INT, (void*)0); //Desde el vertice 0, pintamos 3 vertices
	
	glutSwapBuffers();

}

void renderParticles()
{
	{
		// Compute shader time test block
		GLuint query;
		GLuint64 elapsed_time;
		int done = 0;
		glGenQueries(1, &query);
		glBeginQuery(GL_TIME_ELAPSED, query);

		glUseProgram(programCompute);

		if (uUsingVerlet != -1)
			glUniform1i(uUsingVerlet, usingVerlet);

		glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		// Testing time elapsed
		glEndQuery(GL_TIME_ELAPSED);
		while (!done) {
			glGetQueryObjectiv(query,
				GL_QUERY_RESULT_AVAILABLE,
				&done);
		}
		glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
		printf("Shader de computo: %f ms\n", elapsed_time / 1000000.0);
	}
	{
		// Render time test block
		GLuint query;
		GLuint64 elapsed_time;
		int done = 0;
		glGenQueries(1, &query);
		glBeginQuery(GL_TIME_ELAPSED, query);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program);

		glm::mat4 modelView = view * model;
		glm::mat4 modelViewProj = proj * view * model;
		glm::mat4 normal = glm::transpose(glm::inverse(modelView));
		glm::vec4 camPos = model * glm::vec4(cameraPos, 1.0);
		if (uModelViewMat != -1) //Si está utilizando dicha matriz, la subo a los shaders activados en el programa
			glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE,
				&(modelView[0][0]));
		if (uModelViewProjMat != -1)
			glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE,
				&(proj[0][0]));
		if (uNormalMat != -1)
			glUniformMatrix4fv(uNormalMat, 1, GL_FALSE,
				&(normal[0][0]));
		if (uColorTex != -1)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, colorTexId);
			glUniform1i(uColorTex, 0);
		}
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindBuffer(GL_ARRAY_BUFFER, posSSBO);
		glVertexPointer(4, GL_FLOAT, 0, (void *)0);
		glEnableClientState(GL_VERTEX_ARRAY);
		//glPointSize(5.0f);
		glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
		glDisableClientState(GL_VERTEX_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		// Testing time elapsed
		glEndQuery(GL_TIME_ELAPSED);
		while (!done) {
			glGetQueryObjectiv(query,
				GL_QUERY_RESULT_AVAILABLE,
				&done);
		}
		glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
		printf("Shaders de vertices, geometria y fragmentos: %f ms\n", elapsed_time / 1000000.0);

	}
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
	else if (ppShaderOption == "map" && key == '+') dispFactor += 0.1f;
	else if (ppShaderOption == "map" && key == '-') dispFactor -= 0.1f;
	else if (ppShaderOption != "map" && key == '+') nSub++;
	else if (ppShaderOption != "map" && key == '-') nSub--;

	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

}
void mouseFunc(int button, int state, int x, int y){}

//Metodo auxiliar para generar un float aleatorio
float Ranf(float Min, float Max)
{
	return ((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
}

