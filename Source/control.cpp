#include "common.h"
#include "../Externals/Include/irrKlang/irrKlang.h"
using namespace glm;
using namespace std;
using namespace irrklang;
#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll
void My_Keyboard(unsigned char key, int x, int y)
{
	// start the sound engine with default parameters
	irrklang::ISoundEngine* engine = irrklang::createIrrKlangDevice();

	printf("Key %c is pressed at (%d, %d)\n", key, x, y);
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