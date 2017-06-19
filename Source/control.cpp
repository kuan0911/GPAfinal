#include "common.h"
#include "../Externals/Include/irrKlang/irrKlang.h"
using namespace glm;
using namespace std;
using namespace irrklang;
#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll

typedef struct group
{
	vec3 p0;
	vec3 p1;
	vec3 p2;
	vec3 p3;
}Group;

void My_Keyboard(unsigned char key, int x, int y)
{
	// start the sound engine with default parameters
	irrklang::ISoundEngine* engine = irrklang::createIrrKlangDevice();

	//printf("Key %c is pressed at (%d, %d)\n", key, x, y);
	if (key == 'd')
	{
		eye = eye + normalize(cross(direction, up));
	}
	else if (key == 'a')
	{
		eye = eye - normalize(cross(direction, up));
	}
	else if (key == 'w')
	{
		eye = eye + normalize(direction);
	}
	else if (key == 's')
	{
		eye = eye - normalize(direction);
	}
	else if (key == 'z')
	{
		eye = eye + vec3(0, 1.0, 0);
	}
	else if (key == 'x')
	{
		eye = eye - vec3(0, 1.0, 0);
	}
	else if (key == 'r')
	{
		eye = vec3(0.0f, 200.0f, 0.0f);
		direction = vec3(-5.0f, 0.0f, 0.0f);
		up = vec3(0.0f, 1.0f, 0.0f);
	}
	else if (key == 'p')
	{
		printf("position: (%f, %f, %f)\n", eye.x, eye.y, eye.z);
	}
	else if (key == '1')
	{
		pokemon = 1;
		engine->play3D("../Assets/media/Fushigidane_voice.wav", vec3df(0, 0, 0));
	}
	else if (key == '2')
	{
		pokemon = 2;
		engine->play3D("../Assets/media/Digda_voice.wav", vec3df(0, 0, 0));
	}
	else if (key == '3')
	{
		pokemon = 3;
		engine->play3D("../Assets/media/Golonya_voice.wav", vec3df(0, 0, 0));
	}
	else if (key == '4')
	{
		pokemon = 4;
		engine->play3D("../Assets/media/Hitokage_voice.wav", vec3df(0, 0, 0));
	}
	else if (key == '5')
	{
		pokemon = 5;
		engine->play3D("../Assets/media/Mew_voice.wav", vec3df(0, 0, 0));
	}
	else if (key == '6')
	{
		pokemon = 6;
		engine->play3D("../Assets/media/Nyarth_voice.wav", vec3df(0, 0, 0));
	}
	else if (key == '7')
	{
		pokemon = 7;
		engine->play3D("../Assets/media/Zenigame_voice.wav", vec3df(0, 0, 0));
	}
}

void My_Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
			xpre = x;
			ypre = y;
		}
		else if (state == GLUT_UP)
		{
			printf("Mouse %d is released at (%d, %d)\n", button, x, y);

			xpre = x;
			ypre = y;
		}
	}
}

void MotionMouse(int x, int y)
{
	vec3 mousedelta = vec3(float((y - ypre) / 4), 0, float((x - xpre) / 4));
	mat4 Identy_Init(1.0);
	mat4 dir_matrix = rotate(Identy_Init, radians(mousedelta[2]), vec3(0.0, 1.0, 0.0));
	dir_matrix = rotate(dir_matrix, radians(mousedelta[0]), cross(direction, vec3(0.0, 1.0, 0.0)));
	direction = (dir_matrix * vec4(direction, 0.0)).xyz;
	up = (dir_matrix * vec4(up, 0.0)).xyz;
	xpre = x;
	ypre = y;
}

float bezier_coordinate(float t, float n0, float n1, float n2, float n3) {
	float ans = n0*pow((1 - t), 3) + 3 * n1*t*pow((1 - t), 2) + 3 * n2*pow(t, 2) * (1 - t) + n3 * pow(t, 3);
	return ans;
}

vec3 bezier_point(float t, vec3 p0, vec3 p1, vec3 p2, vec3 p3) {
	float x = bezier_coordinate(t, p0.x, p1.x, p2.x, p3.x);
	float y = bezier_coordinate(t, p0.y, p1.y, p2.y, p3.y);
	float z = bezier_coordinate(t, p0.z, p1.z, p2.z, p3.z);
	vec3 point = vec3(x, y, z);
	return point;
}

void bezier_curv(float t) {
	Group p[9] = { { vec3(96.0, 40.0, 109.0),vec3(96.0, 40.0, 100.0),vec3(94.0, 40.0, -100.0),vec3(94.0, 40.0, -107.0) },
	               { vec3(94.0, 40.0, -107.0),vec3(94.0, 40.0, -150.0),vec3(250.0, 100.0, -215.0),vec3(274.0, 100.0, -215.0) },
				   { vec3(274.0, 100.0, -215.0),vec3(290.0, 100.0, -215.0),vec3(516.0, 100.0, -180.0),vec3(516.0, 200.0, -180.0) },
				   { vec3(516.0, 200.0, -180.0),vec3(516.0, 300.0, -190.0),vec3(670.0, 374.0, -160.0),vec3(670.0, 374.0, -141.0) },
				   { vec3(670.0, 374.0, -141.0),vec3(670.0, 374.0, -120.0),vec3(850.0, 254.0, 120.0),vec3(887.0, 254.0, 101.0) },
				   { vec3(887.0, 254.0, 101.0),vec3(930.0, 254.0, 101.0),vec3(1135.0, 20.0, -780.0),vec3(1132.0, 20.0, -792.0) },
				   { vec3(1132.0, 20.0, -792.0),vec3(1112.0, 120.0, -780.0),vec3(300.0, 240.0, -580.0),vec3(289.0, 240.0, -568.0) },
				   { vec3(289.0, 240.0, -568.0),vec3(280.0, 240.0, -576.0),vec3(-397.0, 32.0, 60.0), vec3(-397.0, 32.0, 50.0) },
				   { vec3(-397.0, 32.0, 50.0),vec3(-1397.0, 32.0, -150.0),vec3(-1417.0, 300.0, -380.0), vec3(-917.0, 300.0, -480.0) } };
	
	//if (t < 1) eye = bezier_point(t, p[0].p0, p[0].p1, p[0].p2, p[0].p3);
	for (int i = 0; i < 9; i++) {
		if (t < i + 1) {
			eye = bezier_point(t - i, p[i].p0, p[i].p1, p[i].p2, p[i].p3);
			break;
		}
		
	}
	
	
}


