#include "common.h"
#include "../Externals/Include/irrKlang/irrKlang.h"
using namespace glm;
using namespace std;
using namespace irrklang;
#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll

bool iftour = true;

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
	else if (key == 't')
	{
		if (curve_t_enable == true) curve_t_enable = false;
		else if (curve_t_enable == false) curve_t_enable = true;
	}
	else if (key == 'f')
	{
		restart = true;
	}
	else if (key == 'g')
	{
		if (iftour == true) iftour = false;
		else if (iftour == false) iftour = true;
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
	Group p[13] = { { vec3(-500.0, 300.0, -480.0),vec3(-300.0, 300.0, -60.0),vec3(-150.0, 148.0, 0.0),vec3(-2.0, 148.0, -60.0) },
	               { vec3(-2.0, 148.0, -60.0),vec3(53.0, 158.0, -47.0),vec3(240.0, 133.0, -89.0),vec3(375.0, 110.0, -220.0) },
				   { vec3(375.0, 110.0, -220.0),vec3(448.0, 126.0, -283.0),vec3(500.0, 122.0, -283.0),vec3(520.0, 205.0, -170.0) },
				   { vec3(520.0, 205.0, -170.0),vec3(536.0, 280.0, -169.0),vec3(624.0, 361.0, -156.0),vec3(674.0, 377.0, -140.0) },
				   { vec3(674.0, 377.0, -140.0),vec3(684.0, 278.0, -47.0),vec3(762.0, 133.0, 53.0),vec3(859.0, 45.0, 75.0) },
				   { vec3(859.0, 45.0, 75.0),vec3(1078.0, 204.0, -215.0),vec3(1177.0, 104.0, -539.0),vec3(1108.0, 35.0, -969.0) },

				   { vec3(1108.0, 35.0, -969.0),vec3(954.0, 158.0, -1159.0),vec3(441.0, 138.0, -1179.0),vec3(198.0, 101.0, -1044.0) },
				   { vec3(198.0, 101.0, -1044.0),vec3(47.0, 123.0, -866.0),vec3(-111.0, 17.0, -541.0),vec3(-181.0, 17.0, -418.0) },
				   { vec3(-181.0, 17.0, -418.0),vec3(-240.0, 17.0, -300.0),vec3(-358.0, 18.0, -101.0),vec3(-407.0, 19.0, 123.0) },
				   { vec3(-407.0, 19.0, 123.0),vec3(-407.0, 19.0, 140.0),vec3(-338.0, 31.0, 300.0),vec3(-162.0, 38.0, 267.0) },

				   { vec3(-162.0, 38.0, 267.0),vec3(-1.0, 30.0, 356.0),vec3(25.0, 27.0, 555.0),vec3(52.0, 24.0, 733.0) },
				   { vec3(52.0, 24.0, 733.0),vec3(177.0, 74.0, 837.0),vec3(429.0, 184.0, 835.0),vec3(603.0, 239.0, 757.0) },
				   { vec3(603.0, 239.0, 757.0),vec3(753.0, 338.0, 521.0),vec3(863.0, 461.0, 297.0),vec3(820.0, 499.0, 183.0)}


	};

	Group c[13] = { { vec3(-2.0, 148.0, -55.0),vec3(0.0, 148.0, -55.0),vec3(0.0, 148.0, -55.0),vec3(10.0, 148.0, -60.0) },
					{ vec3(10.0, 148.0, -60.0),vec3(53.0, 158.0, -27.0),vec3(420.0, 133.0, -87.0),vec3(430.0, 110.0, -220.0) },
					{ vec3(430.0, 110.0, -220.0),vec3(450.0, 120.0, -220.0),vec3(500.0, 200.0, -170.0),vec3(536.0, 205.0, -120.0) },
					{ vec3(536.0, 205.0, -120.0),vec3(500.0, 320.0, -120.0),vec3(670.0, 360.0, -120.0),vec3(674.0, 377.0, -120.0) },
					{ vec3(674.0, 377.0, -120.0),vec3(674.0, 270.0, 0.0),vec3(859.0, 45.0, 50.0),vec3(859.0, 45.0, 60.0) },
					{ vec3(859.0, 45.0, 60.0),vec3(1078.0, 204.0, -230.0),vec3(1177.0, 104.0, -539.0),vec3(1100.0, 35.0, -980.0) },

					{ vec3(1100.0, 35.0, -980.0),vec3(454.0, 158.0, -1160.0),vec3(441.0, 138.0, -1100.0),vec3(198.0, 101.0, -1020.0) },
					{ vec3(198.0, 101.0, -1020.0),vec3(47.0, 123.0, -866.0),vec3(-111.0, 17.0, -541.0),vec3(-200.0, 17.0, -418.0) },
					{ vec3(-200.0, 17.0, -418.0),vec3(-440.0, 17.0, -300.0),vec3(-658.0, 18.0, 101.0),vec3(-500.0, 19.0, 110.0) },
					{ vec3(-500.0, 19.0, 110.0),vec3(-500.0, 19.0, 150.0),vec3(-500.0, 18.0, 970.0),vec3(-500.0, 18.0, 980.0) },

					{ vec3(-500.0, 18.0, 980.0),vec3(0.0, 18.0, 1000.0),vec3(-100.0, 24.0, 730.0),vec3(200.0, 24.0, 740.0) },
					{ vec3(200.0, 24.0, 740.0),vec3(187.0, 74.0, 837.0),vec3(440.0, 184.0, 835.0),vec3(610.0, 239.0, 750.0) },
					{ vec3(610.0, 239.0, 750.0),vec3(758.0, 330.0, 510.0),vec3(510.0, 490.0, 190.0),vec3(500.0, 300.0, 0.0) }


					 };
	
	if (iftour) {
		for (int i = 0; i < 13; i++) {
			if (t < i + 1) {
				eye = bezier_point(t - i, p[i].p0, p[i].p1, p[i].p2, p[i].p3);
				vec3 camerapoint = bezier_point(t - i, c[i].p0, c[i].p1, c[i].p2, c[i].p3);
				direction = camerapoint - eye;
				//vec3 eye_next = bezier_point(t+0.0001 - i, p[i].p0, p[i].p1, p[i].p2, p[i].p3);
				//direction = eye_next - eye;
				break;
			}
		}
	}
}


