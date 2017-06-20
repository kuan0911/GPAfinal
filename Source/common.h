#include "../Externals/Include/Include.h"
using namespace glm;
using namespace std;

extern int xpre;
extern int ypre;
extern int pokemon;
extern vec3 eye;
extern vec3 center;
extern vec3 up;
extern vec3 direction;
extern bool curve_t_enable;
extern bool restart;

void My_Keyboard(unsigned char key, int x, int y);
void My_Mouse(int button, int state, int x, int y);
void MotionMouse(int x, int y);
void bezier_curv(float t);

