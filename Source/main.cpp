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
#define SHADOW_MAP_SIZE 4096

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
GLint blinnPhongProg;
GLint depthProg;
GLint particleProg;
GLint mv_location;
GLint proj_location;
mat4 view;
mat4 proj_matrix;
mat4 ch_matrix;
mat4 mv_matrix[8];
mat4 sun_mv_matrix;
vec3 light_pos;
GLint light_pos_location;

struct
{
	struct
	{
		GLint   mvp;
	} light;
	struct
	{
		GLuint  shadow_tex_location;
		GLint   mv_matrix_location;
		GLint   proj_matrix_location;
		GLint   shadow_matrix_location;
		GLint   full_shading_location;
		GLint   light_matrix_location;
	} view;
} uniforms;
GLint   mv_matrix_normal_location;
GLint   proj_matrix_normal_location;
GLint   mv_matrix_particle_location;
GLint   proj_matrix_particle_location;

struct
{
	GLuint fbo;
	GLuint depthMap;
} shadowBuffer;

struct
{
	int width;
	int height;
} viewportSize;


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
const aiScene *scene1 = aiImportFile("Fushigidane.obj", aiProcessPreset_TargetRealtime_MaxQuality);//§®µìºØ¤l
const aiScene *scene2 = aiImportFile("Digda.obj", aiProcessPreset_TargetRealtime_MaxQuality);//¦a¹«
const aiScene *scene3 = aiImportFile("Golonya.obj", aiProcessPreset_TargetRealtime_MaxQuality);//¥ÛÀY©Ç
const aiScene *scene4 = aiImportFile("Hitokage.obj", aiProcessPreset_TargetRealtime_MaxQuality);//¤p¤õÀs
const aiScene *scene5 = aiImportFile("Mew.obj", aiProcessPreset_TargetRealtime_MaxQuality);
const aiScene *scene6 = aiImportFile("Nyarth.obj", aiProcessPreset_TargetRealtime_MaxQuality);//ØpØp
const aiScene *scene7 = aiImportFile("Zenigame.obj", aiProcessPreset_TargetRealtime_MaxQuality);//³Ç¥§Àt
const aiScene *scene8 = aiImportFile("globe-sphere.obj", aiProcessPreset_TargetRealtime_MaxQuality);//¤Ó¶§


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
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.5, 0.5, 0.5, 1.0);

	// ----- Begin Initialize Depth Shader Program -----
	depthProg = glCreateProgram();
	GLuint shadow_vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint shadow_fs = glCreateShader(GL_FRAGMENT_SHADER);
	char** vertexShaderSourceDepth = loadShaderSource("depth_vs.glsl");
	char** fragmentShaderSourceDepth = loadShaderSource("depth_fs.glsl");
	glShaderSource(shadow_vs, 1, vertexShaderSourceDepth, 0);
	glShaderSource(shadow_fs, 1, fragmentShaderSourceDepth, 0);
	freeShaderSource(vertexShaderSourceDepth);
	freeShaderSource(fragmentShaderSourceDepth);
	glCompileShader(shadow_vs);
	glCompileShader(shadow_fs);
	shaderLog(shadow_vs);
	shaderLog(shadow_fs);
	depthProg = glCreateProgram();
	glAttachShader(depthProg, shadow_vs);
	glAttachShader(depthProg, shadow_fs);
	glLinkProgram(depthProg);
	uniforms.light.mvp = glGetUniformLocation(depthProg, "mvp");
	// ----- End Initialize Depth Shader Program -----

	// ----- Begin Initialize Blinn-Phong Shader Program -----
	blinnPhongProg = glCreateProgram();
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
	glAttachShader(blinnPhongProg, vertexShader);
	glAttachShader(blinnPhongProg, fragmentShader);
	glLinkProgram(blinnPhongProg);
	glUseProgram(blinnPhongProg);
	uniforms.view.proj_matrix_location = glGetUniformLocation(blinnPhongProg, "proj_matrix");
	uniforms.view.mv_matrix_location = glGetUniformLocation(blinnPhongProg, "mv_matrix");
	uniforms.view.shadow_matrix_location = glGetUniformLocation(blinnPhongProg, "shadow_matrix");
	uniforms.view.shadow_tex_location = glGetUniformLocation(blinnPhongProg, "shadow_tex");
	light_pos_location = glGetUniformLocation(blinnPhongProg, "light_pos");
	
	// ----- End Initialize Blinn-Phong Shader Program -----

	// ----- Begin Initialize normal Shader Program -----
	program = glCreateProgram();
	GLuint normal_vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint normal_fs = glCreateShader(GL_FRAGMENT_SHADER);
	char** vertexShaderSourceNormal = loadShaderSource("vertex.vs.origin.glsl");
	char** fragmentShaderSourceNormal = loadShaderSource("fragment.fs.origin.glsl");
	glShaderSource(normal_vs, 1, vertexShaderSourceNormal, NULL);
	glShaderSource(normal_fs, 1, fragmentShaderSourceNormal, NULL);
	freeShaderSource(vertexShaderSourceNormal);
	freeShaderSource(fragmentShaderSourceNormal);
	glCompileShader(normal_vs);
	glCompileShader(normal_fs);
	shaderLog(normal_vs);
	shaderLog(normal_fs);
	glAttachShader(program, normal_vs);
	glAttachShader(program, normal_fs);
	glLinkProgram(program);
	glUseProgram(program);
	mv_matrix_normal_location = glGetUniformLocation(program, "um4mv");
	proj_matrix_normal_location = glGetUniformLocation(program, "um4p");
	// ----- End Initialize normal Shader Program 

	// ----- Begin Initialize Partical System Shader Program -----
	particleProg = glCreateProgram();
	GLuint particle_vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint particle_fs = glCreateShader(GL_FRAGMENT_SHADER);
	char** vertexShaderSourceParticle = loadShaderSource("particlesystem_vs.glsl");
	char** fragmentShaderSourceParticle = loadShaderSource("particlesystem_fs.glsl");
	glShaderSource(particle_vs, 1, vertexShaderSourceParticle, NULL);
	glShaderSource(particle_fs, 1, fragmentShaderSourceParticle, NULL);
	freeShaderSource(vertexShaderSourceParticle);
	freeShaderSource(fragmentShaderSourceParticle);
	glCompileShader(particle_vs);
	glCompileShader(particle_fs);
	shaderLog(particle_vs);
	shaderLog(particle_fs);
	glAttachShader(particleProg, particle_vs);
	glAttachShader(particleProg, particle_fs);
	glLinkProgram(particleProg);
	glUseProgram(particleProg);
	mv_matrix_particle_location = glGetUniformLocation(particleProg, "um4mv");
	proj_matrix_particle_location = glGetUniformLocation(particleProg, "um4p");
	// ----- End Initialize Partical System Shader Program -----

	materials = new Material*[8];
	shapes = new Shape*[8];
	for (int s = 0; s < 9; s++) {
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
		else if (s == 8)
			scene = scene8;
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
		glm::mat4 Identy_Init(1.0);
		ch_matrix = Identy_Init;
		if (s > 0 & s < 8)
			mv_matrix[s] = glm::translate(Identy_Init, glm::vec3(80.0f + 10 * s, 4.0f, 0.0f));
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
	// ----- Begin Initialize Shadow Framebuffer Object -----
	glGenFramebuffers(1, &shadowBuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer.fbo);

	glGenTextures(1, &shadowBuffer.depthMap);
	glBindTexture(GL_TEXTURE_2D, shadowBuffer.depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowBuffer.depthMap, 0);
	glDrawBuffer(GL_NONE);
	// ----- End Initialize Shadow Framebuffer Object -----
}

void My_Display()
{
	float f_timer_cnt = glutGet(GLUT_ELAPSED_TIME);
	light_pos = vec3(3000 * cos(f_timer_cnt* 0.0001f), 3000 * sin(f_timer_cnt* 0.0001f), 0.0);
	view = lookAt(eye, eye + direction, up);
	mat4 scale_bias_matrix = mat4(
		vec4(0.5f, 0.0f, 0.0f, 0.0f),
		vec4(0.0f, 0.5f, 0.0f, 0.0f),
		vec4(0.0f, 0.0f, 0.5f, 0.0f),
		vec4(0.5f, 0.5f, 0.5f, 1.0f)
	);

	// ----- Begin Shadow Map Pass -----
	mat4 light_proj_matrix = frustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 10000.0f);
	mat4 light_view_matrix = lookAt(light_pos, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 light_vp_matrix = light_proj_matrix * light_view_matrix;

	mat4 shadow_sbpv_matrix = scale_bias_matrix * light_vp_matrix;

	glUseProgram(depthProg);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowBuffer.fbo);
	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(4.0f, 4.0f);

	// ----  Background Shadow Map ----
	glUniformMatrix4fv(uniforms.light.mvp, 1, GL_FALSE, value_ptr(light_vp_matrix));
	const aiScene *scene;
	scene = scene0;
	for (int l = 0; l< int(scene->mNumMeshes); ++l)
	{
		glBindVertexArray(shapes[0][l].vao);
		glDrawElements(GL_TRIANGLES, shapes[0][l].drawCount, GL_UNSIGNED_INT, 0);
	}
	// ---- Pokemon Shadow Map ----
	int s = pokemon;
	glUniformMatrix4fv(uniforms.light.mvp, 1, GL_FALSE, value_ptr(light_vp_matrix*mv_matrix[s]));
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
		glDrawElements(GL_TRIANGLES, shapes[s][l].drawCount, GL_UNSIGNED_INT, 0);
	}
	glDisable(GL_POLYGON_OFFSET_FILL);
	// ----- End Shadow Map Pass -----

	// ----- Begin Blinn-Phong Shading Pass -----	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, viewportSize.width, viewportSize.height);
	glUseProgram(blinnPhongProg);

	//GLint light_pos_location = glGetUniformLocation(blinnPhongProg, "light_pos");
	glUniform3fv(light_pos_location, 1, value_ptr(light_pos));

	glUniformMatrix4fv(uniforms.view.proj_matrix_location, 1, GL_FALSE, value_ptr(proj_matrix));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowBuffer.depthMap);
	glUniform1i(uniforms.view.shadow_tex_location, 0);
	
	mat4 shadow_matrix = shadow_sbpv_matrix;	
	
	// ----  Background Blinn-Phong ----
	glUniformMatrix4fv(uniforms.view.mv_matrix_location, 1, GL_FALSE, value_ptr(view));
	glUniformMatrix4fv(uniforms.view.shadow_matrix_location, 1, GL_FALSE, value_ptr(shadow_matrix));

	scene = scene0;
	for (int l = 0; l< int(scene->mNumMeshes); ++l)
	{
		glBindVertexArray(shapes[0][l].vao);
		int materialID = shapes[0][l].materialID;
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, materials[0][materialID].diffuse_tex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shapes[0][l].ibo);
		glDrawElements(GL_TRIANGLES, shapes[0][l].drawCount, GL_UNSIGNED_INT, 0);
	}
	// ----  Pokemon Blinn-Phong ----
	s = pokemon;
	glUniformMatrix4fv(uniforms.view.mv_matrix_location, 1, GL_FALSE, value_ptr(view*mv_matrix[s]));
	glUniformMatrix4fv(uniforms.view.shadow_matrix_location, 1, GL_FALSE, value_ptr(shadow_matrix));
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
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, materials[s][materialID].diffuse_tex);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shapes[s][l].ibo);
		glDrawElements(GL_TRIANGLES, shapes[s][l].drawCount, GL_UNSIGNED_INT, 0);
	}


	// ----- End Blinn-Phong Shading Pass -----
	
	// ---- Begin Draw Sun ----
	glUseProgram(program);
	glUniform3fv(light_pos_location, 1, value_ptr(light_pos));
	sun_mv_matrix = glm::translate(mat4(1.0f), light_pos);
	sun_mv_matrix = glm::scale(sun_mv_matrix, glm::vec3(100.0f));
	glUniformMatrix4fv(mv_matrix_normal_location, 1, GL_FALSE, value_ptr(view*sun_mv_matrix));
	glUniformMatrix4fv(proj_matrix_normal_location, 1, GL_FALSE, value_ptr(proj_matrix));
	//glActiveTexture(GL_TEXTURE0);	
	scene = scene8;
	for (int l = 0; l< int(scene->mNumMeshes); ++l)
	{
		glBindVertexArray(shapes[8][l].vao);
		//int materialID = shapes[8][l].materialID;
		//glBindTexture(GL_TEXTURE_2D, materials[8][materialID].diffuse_tex);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shapes[8][l].ibo);
		glDrawElements(GL_TRIANGLES, shapes[8][l].drawCount, GL_UNSIGNED_INT, 0);
	}
	// ---- End Draw Sun ----

	// ---- Begin Graw Particle ----
	glUseProgram(particleProg);
	
	float currentTime = f_timer_cnt* 0.001f;
	currentTime *= 0.1f;
	currentTime -= floor(currentTime);
	GLuint tex_location = glGetUniformLocation(particleProg, "tex");
	GLint time_Loc = glGetUniformLocation(particleProg, "time");
	glUniform1f(time_Loc, currentTime);
	glUniform1i(tex_location, 0);

	//for point sprite
	glm::mat4 mv_sprite = glm::translate(mv_matrix[s], glm::vec3(0, 0, 50.0f));
	glUniformMatrix4fv(mv_matrix_particle_location, 1, GL_FALSE, value_ptr(view*mv_sprite));
	glUniformMatrix4fv(proj_matrix_particle_location, 1, GL_FALSE, value_ptr(proj_matrix));
	glEnable(GL_BLEND);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_texture[s - 1]);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glDrawArrays(GL_POINTS, 0, NUM_FLAME);
	glutPostRedisplay();
	// ---- End Graw Particle ----
	
	glutSwapBuffers();
}

void My_Reshape(int width, int height)
{
	viewportSize.width = width;
	viewportSize.height = height;
	glViewport(0, 0, width, height);
	float viewportAspect = (float)width / (float)height;
	proj_matrix = perspective(radians(60.0f), viewportAspect, 0.1f, 10000.0f);
	//initial position
	eye = vec3(95.0f, 40.0f, 75.0f);
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
