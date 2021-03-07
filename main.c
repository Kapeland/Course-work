#include <GL/glut.h>
#include <stdlib.h>
#include <math.h>

#define M_PI 3.14159265358979323846
#define WindowSize 500

void display() {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);


	double cnt = 120;
	double l = 60;
	double a = acos(-1) * 2.0 / 50.0;

	glBegin(GL_TRIANGLE_FAN);
	glColor3f(1, 0, 1);
	glVertex2f(250, 250); // вершина в центре круга
	for(int i = 0; i < cnt; i++) {
		glVertex2f(250 + cos(a * i) * l, 250 + sin(a * i) * l);
	}
	glEnd();
	glutSwapBuffers();
}

void reshape() {
	glViewport(0, 0, WindowSize, WindowSize);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, WindowSize, 0, WindowSize);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WindowSize, WindowSize);
	glutInitWindowPosition(550, 200);
	glutCreateWindow("Checkers");
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutMainLoop();

}