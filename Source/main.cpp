#include "common.h"
#include "openGLrelated.h"
#include <stdio.h>
#include "../Externals/Include/irrKlang/irrKlang.h"

// include console I/O methods (conio.h for windows, our wrapper in linux)
#if defined(WIN32)
#include <conio.h>
#else
#include "../common/conio.h"
#endif

using namespace glm;
using namespace std;
using namespace irrklang;

#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll

#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2
#define MENU_EXIT 3
#define M_PI 3.14
#define NUM_FLAME 2000
GLubyte timer_cnt = 0;
bool timer_enabled = true;
static unsigned int seed = 0x13371337;
unsigned int timer_speed = 16;
float angleX = 0;
float startX = 0;
float angleY = 0;
float startY = 0;
GLint program;
GLint mv_location;
GLint proj_location;
mat4 view;
mat4 proj_matrix;
mat4 ch_matrix;
mat4 mv_matrix[8];

//for point sprite
GLuint vao;
GLuint buffer;
GLuint m_texture[7];

int xpre;
int ypre;
int pokemon;
vec3 eye;
vec3 center;
vec3 up;
vec3 direction;
// start the sound engine with default parameters
irrklang::ISoundEngine* engine = irrklang::createIrrKlangDevice();

const aiScene *scene0 = aiImportFile("city.obj", aiProcessPreset_TargetRealtime_MaxQuality);//background
const aiScene *scene1 = aiImportFile("Fushigidane.obj", aiProcessPreset_TargetRealtime_MaxQuality);//����ؤl
const aiScene *scene2 = aiImportFile("Digda.obj", aiProcessPreset_TargetRealtime_MaxQuality);//�a��
const aiScene *scene3 = aiImportFile("Golonya.obj", aiProcessPreset_TargetRealtime_MaxQuality);//���Y��
const aiScene *scene4 = aiImportFile("Hitokage.obj", aiProcessPreset_TargetRealtime_MaxQuality);//�p���s
const aiScene *scene5 = aiImportFile("Mew.obj", aiProcessPreset_TargetRealtime_MaxQuality);
const aiScene *scene6 = aiImportFile("Nyarth.obj", aiProcessPreset_TargetRealtime_MaxQuality);//�p�p
const aiScene *scene7 = aiImportFile("Zenigame.obj", aiProcessPreset_TargetRealtime_MaxQuality);//�ǥ��t


static inline float random_float()
{
	float res;
	unsigned int tmp;

	seed *= 16807;

	tmp = seed ^ (seed >> 4) ^ (seed << 15);

	*((unsigned int *)&res) = (tmp >> 9) | 0x3F800000;

	return (res - 1.0f);
}

struct star_t
{
	glm::vec3     position;
	glm::vec3     color;
};

struct Shape {
	GLuint vao;
	GLuint vbo_position;
	GLuint vbo_normal;
	GLuint vbo_texcoord;
	GLuint ibo;
	int drawCount;
	int materialID;
};

Shape **shapes = NULL;

struct Material {
	GLuint diffuse_tex;
};
Material **materials = NULL;;

char** loadShaderSource(const char* file)
{
	FILE* fp = fopen(file, "rb");
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *src = new char[sz + 1];
	fread(src, sizeof(char), sz, fp);
	src[sz] = '\0';
	char **srcp = new char*[1];
	srcp[0] = src;
	return srcp;
}

void freeShaderSource(char** srcp)
{
	delete[] srcp[0];
	delete[] srcp;
}

// define a simple data structure for storing texture image raw data
typedef struct _TextureData
{
	_TextureData(void) :
		width(0),
		height(0),
		data(0)
	{
	}

	int width;
	int height;
	unsigned char* data;
} TextureData;

// load a png image and return a TextureData structure with raw data
// not limited to png format. works with any image format that is RGBA-32bit
TextureData loadPNG(const char* const pngFilepath)
{
	TextureData texture;
	int components;

	// load the texture with stb image, force RGBA (4 components required)
	stbi_uc *data = stbi_load(pngFilepath, &texture.width, &texture.height, &components, 4);

	// is the image successfully loaded?
	if (data != NULL)
	{
		// copy the raw data
		size_t dataSize = texture.width * texture.height * 4 * sizeof(unsigned char);
		texture.data = new unsigned char[dataSize];
		memcpy(texture.data, data, dataSize);

		// mirror the image vertically to comply with OpenGL convention
		for (size_t i = 0; i < texture.width; ++i)
		{
			for (size_t j = 0; j < texture.height / 2; ++j)
			{
				for (size_t k = 0; k < 4; ++k)
				{
					size_t coord1 = (j * texture.width + i) * 4 + k;
					size_t coord2 = ((texture.height - j - 1) * texture.width + i) * 4 + k;
					std::swap(texture.data[coord1], texture.data[coord2]);
				}
			}
		}

		// release the loaded image
		stbi_image_free(data);
	}
	else
		cout << pngFilepath << endl;
	return texture;
}

void My_Init()
{
	program = glCreateProgram();
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	char** vertexShaderSource = loadShaderSource("vertex.vs.glsl");
	char** fragmentShaderSource = loadShaderSource("fragment.vs.glsl");
	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
	freeShaderSource(vertexShaderSource);
	freeShaderSource(fragmentShaderSource);
	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);
	shaderLog(vertexShader);
	shaderLog(fragmentShader);
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	materials = new Material*[7];
	shapes = new Shape*[7];
	for (int s = 0; s < 8; s++) {
		const aiScene *scene;
		if (s == 0)
			scene = scene0;
		else if (s == 1)
			scene = scene1;
		else if (s == 2)
			scene = scene2;
		else if (s == 3)
			scene = scene3;
		else if (s == 4)
			scene = scene4;
		else if (s == 5)
			scene = scene5;
		else if (s == 6)
			scene = scene6;
		else if (s == 7)
			scene = scene7;
		if (glGenerateMipmap == NULL)
			cout << "error no minmap" << endl;
		materials[s] = new Material[scene->mNumMaterials];
		shapes[s] = new Shape[scene->mNumMeshes];
		for (int k = 0; k < scene->mNumMaterials; ++k) {
			aiMaterial *material = scene->mMaterials[k];
			Material m = *(new Material());
			aiString texturePath;
			if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == aiReturn_SUCCESS) {
				glGenTextures(1, &m.diffuse_tex);
				glBindTexture(GL_TEXTURE_2D, m.diffuse_tex);
				char* path = new char[80];
				/*strcat(path, "../Source/");
				strcat(path, texturePath.C_Str());
				strcat(path, "\0");*/
				//cout << texturePath.C_Str() << endl;
				TextureData text = loadPNG(texturePath.C_Str());
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, text.width, text.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, text.data);
				glGenerateMipmap(GL_TEXTURE_2D);
			}
			else {
				cout << "no diffuse tex" << endl;
				m.diffuse_tex = 0;
			}
			materials[s][k] = m;
		}
		for (int l = 0; l < int(scene->mNumMeshes); ++l) {
			aiMesh *mesh = (scene->mMeshes)[l];
			Shape shape = *(new Shape());
			glGenVertexArrays(1, &shape.vao);
			glBindVertexArray(shape.vao);
			glGenBuffers(1, &shape.vbo_normal);
			glGenBuffers(1, &shape.vbo_position);
			glGenBuffers(1, &shape.vbo_texcoord);
			glGenBuffers(1, &shape.ibo);
			shape.materialID = mesh->mMaterialIndex;

			shape.drawCount = mesh->mNumFaces * 3;
			GLfloat *data1 = new GLfloat[mesh->mNumVertices * 3];
			GLfloat *data2 = new GLfloat[mesh->mNumVertices * 3];
			GLfloat *data3 = new GLfloat[mesh->mNumVertices * 2];
			int x = 0;
			for (int v = 0; v < mesh->mNumVertices; ++v) {
				if (mesh->mTextureCoords[0] != NULL) {
					data1[3 * x] = mesh->mNormals[v].x;
					data1[3 * x + 1] = mesh->mNormals[v].y;
					data1[3 * x + 2] = mesh->mNormals[v].z;
					data2[3 * x] = mesh->mVertices[v].x;
					data2[3 * x + 1] = mesh->mVertices[v].y;
					data2[3 * x + 2] = mesh->mVertices[v].z;
					data3[2 * x] = mesh->mTextureCoords[0][v].x;
					data3[2 * x + 1] = mesh->mTextureCoords[0][v].y;
					x++;
				}
			}
			glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_normal);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, data1, GL_STATIC_DRAW);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(2);
			glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_position);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, data2, GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_texcoord);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * mesh->mNumVertices, data3, GL_STATIC_DRAW);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(1);
			//delete data1;
			//delete data2;
			//delete data3;
			//cout << mesh->mNumFaces << endl;
			unsigned int *data = new unsigned int[mesh->mNumFaces * 3];

			for (unsigned int f = 0; f < mesh->mNumFaces; f++) {
				for (unsigned int j = 0; j < 3; j++) {

					data[3 * f + j] = mesh->mFaces[f].mIndices[j];
				}
			}
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape.ibo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 3 * mesh->mNumFaces, data, GL_STATIC_DRAW);
			//delete data;
			delete data1;
			delete data2;
			delete data3;
			delete data;
			shapes[s][l] = shape;
		}
		glClearColor(0.0f, 0.6f, 0.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		//aiReleaseImport(scene);
		mv_location = glGetUniformLocation(program, "um4mv");

		proj_location = glGetUniformLocation(program, "um4p");
		glm::mat4 Identy_Init(1.0);
		ch_matrix = Identy_Init;
		if (s>0)
			mv_matrix[s] = glm::translate(Identy_Init, glm::vec3(-20.0f + 10 * s, -26.0f, -75.0f));
		else
			mv_matrix[s] = glm::translate(Identy_Init, glm::vec3(5.0f + 10 * s, -30.0f, -75.0f));
	}

	//for point sprite
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, NUM_FLAME * sizeof(star_t), NULL, GL_STATIC_DRAW);
	star_t * star = (star_t *)glMapBufferRange(GL_ARRAY_BUFFER, 0, NUM_FLAME * sizeof(star_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int i = 0; i < 1000; i++)
	{
		star[i].position[0] = (random_float() * 2.0f - 1.0f) * 100.0f;
		star[i].position[1] = (random_float() * 2.0f - 1.0f) * 100.0f;
		star[i].position[2] = random_float();
		star[i].color[0] = 0.8f + random_float() * 0.2f;
		star[i].color[1] = 0.8f + random_float() * 0.2f;
		star[i].color[2] = 0.8f + random_float() * 0.2f;
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(star_t), NULL);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(star_t), (void *)sizeof(glm::vec3));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_POINT_SPRITE);
	for (int s = 1; s <= 7; s++) {
		TextureData tdata;
		if (s == 1)
			tdata = loadPNG("grass_1.png");
		else if (s == 2)
			tdata = loadPNG("dust_1.png");
		else if (s == 3)
			tdata = loadPNG("rock_1.png");
		else if (s == 4)
			tdata = loadPNG("flame_1.png");
		else if (s == 5)
			tdata = loadPNG("bubble_1.png");
		else if (s == 6)
			tdata = loadPNG("smoke_1.png");
		else if (s == 7)
			tdata = loadPNG("water_1.png");
		glEnable(GL_BLEND);
		glGenTextures(1, &m_texture[s - 1]);
		glBindTexture(GL_TEXTURE_2D, m_texture[s - 1]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}

void My_Display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);
	glm::mat4 Identy_Init(1.0);
	mat4 m = Identy_Init;
	float f_timer_cnt = glutGet(GLUT_ELAPSED_TIME);
	float currentTime = f_timer_cnt* 0.001f;
	currentTime *= 0.1f;
	currentTime -= floor(currentTime);
	GLuint tex_location = glGetUniformLocation(program, "tex");
	GLint time_Loc = glGetUniformLocation(program, "time");
	GLint type_Loc = glGetUniformLocation(program, "type");
	glUniform1f(time_Loc, currentTime);
	glUniform1i(tex_location, 0);

	//background
	glUniform1i(type_Loc, 0);
	view = lookAt(eye, eye + direction, up);
	glUniformMatrix4fv(mv_location, 1, GL_FALSE, value_ptr(view*mv_matrix[0]));
	glUniformMatrix4fv(proj_location, 1, GL_FALSE, value_ptr(proj_matrix));
	glActiveTexture(GL_TEXTURE0);
	const aiScene *scene;
	scene = scene0;
	for (int l = 0; l< int(scene->mNumMeshes); ++l)
	{
		glBindVertexArray(shapes[0][l].vao);
		int materialID = shapes[0][l].materialID;
		glBindTexture(GL_TEXTURE_2D, materials[0][materialID].diffuse_tex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shapes[0][l].ibo);
		glDrawElements(GL_TRIANGLES, shapes[0][l].drawCount, GL_UNSIGNED_INT, 0);
	}

	//pokemon
	int s = pokemon;
	glUniform1i(type_Loc, 0);
	glUniformMatrix4fv(mv_location, 1, GL_FALSE, value_ptr(view*mv_matrix[s]));
	glUniformMatrix4fv(proj_location, 1, GL_FALSE, value_ptr(proj_matrix));
	glActiveTexture(GL_TEXTURE0);
	//glBlendFunc(GL_ONE, GL_ZERO);////////////////////////////
	if (s == 1)
		scene = scene1;
	else if (s == 2)
		scene = scene2;
	else if (s == 3)
		scene = scene3;
	else if (s == 4)
		scene = scene4;
	else if (s == 5)
		scene = scene5;
	else if (s == 6)
		scene = scene6;
	else if (s == 7)
		scene = scene7;
	for (int l = 0; l< int(scene->mNumMeshes); ++l)
	{
		glBindVertexArray(shapes[s][l].vao);
		int materialID = shapes[s][l].materialID;
		glBindTexture(GL_TEXTURE_2D, materials[s][materialID].diffuse_tex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shapes[s][l].ibo);
		glDrawElements(GL_TRIANGLES, shapes[s][l].drawCount, GL_UNSIGNED_INT, 0);
	}



	//for point sprite
	glUniform1i(type_Loc, 1);
	glm::mat4 mv_sprite = glm::translate(mv_matrix[s], glm::vec3(0, 0, 50.0f));
	glUniformMatrix4fv(mv_location, 1, GL_FALSE, value_ptr(view*mv_sprite));
	glUniformMatrix4fv(proj_location, 1, GL_FALSE, value_ptr(proj_matrix));
	glEnable(GL_BLEND);
	//glBlendFunc(GL_ONE, GL_ONE);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture[s - 1]);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glDrawArrays(GL_POINTS, 0, NUM_FLAME);
	glutPostRedisplay();

	glutSwapBuffers();
	ch_matrix = Identy_Init;
}

void My_Reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	float viewportAspect = (float)width / (float)height;
	proj_matrix = perspective(radians(60.0f), viewportAspect, 0.1f, 10000.0f);
	//initial position
	eye = vec3(20.0f, 2.0f, 0.0f);
	direction = vec3(0.0f, 0.0f, -5.0f);
	center = eye + direction;
	up = vec3(0.0f, 1.0f, 0.0f);
}

void My_Timer(int val)
{
	glutPostRedisplay();
	glutTimerFunc(timer_speed, My_Timer, val);
}

void My_SpecialKeys(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F1:
		printf("F1 is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_PAGE_UP:
		printf("Page up is pressed at (%d, %d)\n", x, y);
		break;
	case GLUT_KEY_LEFT:
		printf("Left arrow is pressed at (%d, %d)\n", x, y);
		break;
	default:
		printf("Other special key is pressed at (%d, %d)\n", x, y);
		break;
	}
}

void My_Menu(int id)
{
	switch (id)
	{
	case MENU_TIMER_START:
		if (!timer_enabled)
		{
			timer_enabled = true;
			glutTimerFunc(timer_speed, My_Timer, 0);
		}
		break;
	case MENU_TIMER_STOP:
		timer_enabled = false;
		break;
	case MENU_EXIT:
		exit(0);
		break;
	default:
		break;
	}
}

int main(int argc, char *argv[])
{
#ifdef __APPLE__
	// Change working directory to source code path
	chdir(__FILEPATH__("/../Assets/"));
#endif
	// Initialize GLUT and GLEW, then create a window.
	////////////////////
	glutInit(&argc, argv);
#ifdef _MSC_VER
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(600, 600);
	glutCreateWindow("AS2_Framework"); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
	glewInit();
#endif

	glPrintContextInfo();
	My_Init();

	if (!engine)
	{
		printf("Could not startup engine\n");
		return 0; // error starting up the engine
	}
	// play some sound stream, looped
	engine->play3D("../Assets/media/BGM_Pokemon.ogg", vec3df(0, 0, 0), true, false, true);

	// Create a menu and bind it to mouse right button.
	int menu_main = glutCreateMenu(My_Menu);
	int menu_timer = glutCreateMenu(My_Menu);

	glutSetMenu(menu_main);
	glutAddSubMenu("Timer", menu_timer);
	glutAddMenuEntry("Exit", MENU_EXIT);

	glutSetMenu(menu_timer);
	glutAddMenuEntry("Start", MENU_TIMER_START);
	glutAddMenuEntry("Stop", MENU_TIMER_STOP);

	glutSetMenu(menu_main);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// Register GLUT callback functions.
	glutDisplayFunc(My_Display);
	glutReshapeFunc(My_Reshape);
	glutMouseFunc(My_Mouse);
	glutMotionFunc(MotionMouse);
	glutKeyboardFunc(My_Keyboard);
	glutSpecialFunc(My_SpecialKeys);
	glutTimerFunc(timer_speed, My_Timer, 0);

	// Enter main event loop.
	glutMainLoop();

	engine->drop(); // delete engine
	return 0;
}
