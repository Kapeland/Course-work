#include <GL/glut.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

#define mapW 10
#define mapH 10
#define WindowSize 500

typedef struct {
  int mine;
  int flag;
  int open;
  int cntAround;
} TCell;

TCell map[mapW][mapH];
int mines;

int IsCellInMap(int x, int y) {
	return (x >= 0) && (y >= 0) && (x < mapW) && (y < mapH);
}

void MousePressed(int button, int state, int x, int y) {
	//glutSetWindowTitle("MousePressed");
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		printf("X:%d\tY:%d\t", x, y);
		if(IsCellInMap((int)(x / 50), (int)(y / 50))) {//потому что поле 500 на 500 и 10 клеток
			//printf("HERE");
			map[(int)(x / 50)][(int)(y / 50)].open = 1;
			//printf("%d\n", map[(int)(x /50)][(int)(y /50)].open);
		}
	}
}

void Game_New() {
	srand(time(NULL));
	memset(map, 0, sizeof(map));
	mines = 20;

	for(int i = 0; i < mines; ++i) {
		int x = rand() % mapW;
		int y = rand() % mapH;
		if(map[x][y].mine) {
			i--;
		} else {
			map[x][y].mine = 1;
			for(int dx = -1; dx < 2; ++dx) {//цикл, который обходит соседнии клетки
				for(int dy = -1; dy < 2; ++dy) {
					if(IsCellInMap(x + dx, y + dy)) {
						map[x + dx][y + dy].cntAround++; //увеличивается счётчик мин для соседних полей
					}
				}
			}
		}
	}
}

void Line(double x1, double y1, double x2, double y2) {
	glVertex2f(x1 * 100, y1 * 100);
	glVertex2f(x2 * 100, y2 * 100);
}

void ShowCount(int a) {
	glLineWidth(3);
	glColor3f(1, 1, 0);
	glBegin(GL_LINES);
	if((a != 1) && (a != 4)) {Line(0.3, 0.85, 0.7, 0.85);}
	if((a != 0) && (a != 1) && (a != 7)) {Line(0.3, 0.5, 0.7, 0.5);}
	if((a != 1) && (a != 4) && (a != 7)) {Line(0.3, 0.15, 0.7, 0.15);}

	if((a != 5) && (a != 6)) {Line(0.7, 0.5, 0.7, 0.85);}
	if((a != 2)) {Line(0.7, 0.5, 0.7, 0.15);}

	if((a != 1) && (a != 2) && (a != 3) && (a != 7)) {Line(0.3, 0.5, 0.3, 0.85);}
	if((a == 0) || (a == 2) || (a == 6) || (a == 8)) {Line(0.3, 0.5, 0.3, 0.15);}
	glEnd();
}

void ShowMine() {
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(0, 0, 0);
	glVertex2f(30, 30);
	glVertex2f(70, 30);
	glVertex2f(70, 70);
	glVertex2f(30, 70);
	glEnd();
}

void ShowField() {
	glBegin(GL_TRIANGLE_STRIP);//рисование квадратика в поле, где может быть мина
	glColor3f(0.8, 0.8, 0.8);
	glVertex2f(0, 100);
	glColor3f(0.7, 0.7, 0.7);
	glVertex2f(100, 100);
	glVertex2f(0, 0);
	glColor3f(0.6, 0.6, 0.6);
	glVertex2f(100, 0);
	glEnd();
}

void ShowFieldOpen() {
	glBegin(GL_TRIANGLE_STRIP);//рисование квадратика в поле, где может быть мина
	glColor3f(0.3, 0.7, 0.3);
	glVertex2f(0, 100);
	glColor3f(0.3, 0.6, 0.3);
	glVertex2f(100, 100);
	glVertex2f(0, 0);
	glColor3f(0.3, 0.5, 0.3);
	glVertex2f(100, 0);
	glEnd();
}

void Game_Show() {
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	glScalef(5.0 / mapW, 5.0 / mapH, 1);//changes size of field
	glutMouseFunc(MousePressed);
	for(int i = 0; i < mapW; ++i) {
		for(int j = 0; j < mapH; ++j) {
			glPushMatrix();
			glTranslatef(i * 100, j * 100, 0);//тут сто потому что первый квадрат с вершиной (100,100)

			if(map[i][j].open) {
				ShowFieldOpen();
				if(map[i][j].mine) {
					ShowMine();
				} else if(map[i][j].cntAround > 0) {
					ShowCount(map[i][j].cntAround);
				}
			} else {
				ShowField();
			}
			glPopMatrix();
		}
	}
	glutSwapBuffers(); // может её нужно поставить перед попом?
}

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
	Game_New();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WindowSize, WindowSize);
	glutInitWindowPosition(550, 200);
	glutCreateWindow("Sapper");
	glutReshapeFunc(reshape);

	glutDisplayFunc(Game_Show);

	glutMainLoop();
	return EXIT_SUCCESS;
}