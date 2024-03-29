#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>
#include <math.h>
#include <time.h>

#if defined(_WIN32) || defined (_WIN64)

#include <windows.h>

#define WINDOWS
#endif

#if defined(linux) || defined(__linux)
#include <unistd.h>
#define LINUX
#endif

#define TRUE 1
#define FALSE 0

#define DESK_SIZE 8
#define NO_EAT -1
#define EAT -2
#define SAVES "Saves.txt"
#define RECORDS "Records.txt"
#define NAME_LEN 11
#define BIG_STRING_NAME 20
#define SMALL_STRING 5
#define MAX_POSITIONS 15

#define MENU 1
#define GAME 2
#define DEVELOPERS 3
#define OPTIONS 7
#define AFTER_GAME 4
#define AFTER_GAME_LOOSE 6
#define SRECORDS 5

#define VSPEOPLE 9
#define LOWAI 11
#define MIDAI 12

int Status = GAME;
int Level = VSPEOPLE;
// Начальная широта и высота окна
GLint Width = 1050, Height = 800;
const int CubeSize = 100; // Размер квадрата

typedef enum{
	black, white
}colors;
typedef enum{
	false, true
}logik;
enum Moves{
	IMP, DR, DL, UR, UL
};
typedef enum{
	Debut, MittelSpiel, EndSpiel
}Phase_;

colors turn = white;
logik cursor = false;
logik space_pressed = false; // Нажат ли пробел
logik WasMoved = false; //
int cur_cursor_x = 0, cur_cursor_y = 0;
char Alphabet[] = "ABCDEFGHIGKLMNOPQRSTUVWXYZ";
char Name[NAME_LEN] = "";

typedef struct{
	int base_x;
	int base_y;
	int to_x;
	int to_y;
	int ROLE;
}Coordinates_;

typedef struct{ // 1 - x, 2 - y
	float Down_border;
	float Left_border;
	float Right_border;
	float Up_border;
	colors Color;
}Buttons_;

typedef struct{
	int press_x;
	int press_y;
}Pressed;

typedef struct{
	colors checker_side; // Сторона шашки
	enum{
		empty, occup
	}field; // Занято ли поле
	colors field_col; // Цвет поля
	enum{
		soldier, queen
	}role;
	struct{
		logik highlighted; // Курсор
		logik toMove; // Захват пробела
	}focused; // наведен ли курсор на поле logik focused;
	struct{
		float x;
		float y;
	}midLocation; // Координаты середины поля
}Desk;

typedef struct records_{
	char name_[NAME_LEN];
	int Moves;
	struct records_* next;
	struct records_* previous;
}records;

Desk desk[DESK_SIZE][DESK_SIZE];
Pressed Move_from, Move_to;
Buttons_ Restart_button = {0}, Save_Position_button = {0}, Recover_Position_button = {0}, Developers_button = {0}, Records_button = {0},
		Options_button = {0}, Easy_button = {0}, Medium_button = {0};
records* head = NULL;

char inChar(int a){
	return '0' + a;
}

void RecordListToFile(void){
	records* tmp = head;
	FILE* records = NULL;
	records = fopen(RECORDS, "w");
	if(records == NULL){
		perror("FAIL. CAN'T OPEN RECORDS\n");
		return;
	}
	while(tmp != NULL){
		fprintf(records, "%s %d\n", tmp->name_, tmp->Moves);
		tmp = tmp->next;
	}
}

int SwapElOfList(records* a, records* b){
	records* New = NULL;
	New = (records*)malloc((sizeof(records)));
	if(New == NULL){
		perror("FAILED! CAN'T ALLOCATE MEMORY!");
		return EXIT_FAILURE;
	}
	memcpy(New, a, (sizeof(records)));
	a->Moves = b->Moves;
	memcpy(a->name_, b->name_, NAME_LEN);

	b->Moves = New->Moves;
	memcpy(b->name_, New->name_, NAME_LEN);

	free(New);
	return EXIT_SUCCESS;
}

void SortList(records* pointer){
	int Cur = pointer->Moves;
	records* tmp = pointer->previous;
	while(tmp != NULL){
		if(tmp->Moves > Cur){
			if(SwapElOfList(tmp, pointer) == EXIT_FAILURE){
				perror("ERROR, CANT ALLOCATE MEMORY\n");
				return;
			}
			pointer = tmp;
		}
		tmp = tmp->previous;
	}
}

int AddToList(char* forName, int forMoves){
	records* tmp = head, * New = NULL;
	if(tmp->Moves == 0) // Первый добавляемый элемент списка
	{
		memcpy(tmp->name_, forName, NAME_LEN);
		tmp->Moves = forMoves;
		tmp->next = NULL;
	}else{
		while(tmp->next != NULL)
			tmp = tmp->next;
		New = (records*)malloc((sizeof(records)));
		if(New == NULL){
			perror("FAILED! CAN'T ALLOCATE MEMORY!");
			return EXIT_FAILURE;
		}
		tmp->next = New;
		memcpy(New->name_, forName, NAME_LEN);
		New->Moves = forMoves;
		New->previous = tmp;
		New->next = NULL;

		SortList(New);
	}
	return EXIT_SUCCESS;
}

int CreateList(void){
	head = (records*)malloc((sizeof(records)));
	if(head == NULL){
		perror("FAILED! CAN'T ALLOCATE MEMORY!");
		return EXIT_FAILURE;
	}
	memset(head->name_, 0, NAME_LEN);
	head->Moves = 0;
	head->previous = NULL;
	head->next = NULL;
	return EXIT_SUCCESS;
}

void freeList(void){
	records* tmp = NULL;
	if(head != NULL){
		while(head->next != NULL){
			tmp = head;
			head = head->next;
			free(tmp);
		}
		free(head);
	}
}

void AddMovesFromList(char* string, int Moves_, char* SmallString){
	int NumOfNums = 0, Moves_2 = Moves_;
	SmallString[0] = ' ';
	while(Moves_2){
		NumOfNums++;
		Moves_2 /= 10;
	}
	while(Moves_){
		SmallString[NumOfNums] = inChar(Moves_ % 10);
		NumOfNums--;
		Moves_ /= 10;
	}

	strcat(string, SmallString);
}

void GetStringOutOfList(records* tmp, char* string, int Nomer){
	char SmallString[SMALL_STRING] = {0};
	if(Nomer < 10) // номер записи
	{
		string[0] = inChar(Nomer);
		string[1] = ')';
		string[2] = ' ';
	}else{
		string[0] = '1';
		string[1] = '0';
		string[2] = ')';
		string[3] = ' ';
	}
	strcat(string, tmp->name_);
	AddMovesFromList(string, tmp->Moves, SmallString);
}

void openRecords(void){
	int count = 0;
	FILE* Records = NULL;
	// Временная память для считывания и записи в список
	char forName[NAME_LEN] = {0};
	int forMoves = 0;

	Records = fopen(RECORDS, "r");
	if(Records == NULL){
		// Создаем файл для дальнейшей записи
		Records = fopen(RECORDS, "w");
		fclose(Records);
		return;
	}
	if(Records == NULL){
		perror("FAIL. CAN'T OPEN RECORDS\n");
		return;
	}
	// Считывание в линейный список если файл существовал и был непуст
	if(CreateList() == EXIT_FAILURE)
		return;

	while((fscanf(Records, "%s%*c%d%*c", forName, &forMoves) != EOF) && count <= 10){
		if(AddToList(forName, forMoves) == EXIT_FAILURE)
			return;
		count++;
	}

	fclose(Records);
}

void drawText(const char* string, int posX, const int posY){
	const int add = 15, low_add = 8;
	int i = 0;
	int len = strlen(string);
	posX -= add * len / 2;
	while(string[i] != '\0'){
		glColor3f(0, 0, 0);
		if(Status == OPTIONS){
			if(Level == LOWAI && (!strcmp(string, "EASY")))
				glColor3ub(255, 0, 255);
			else if(Level == MIDAI && (!strcmp(string, "MEDIUM")))
				glColor3ub(255, 0, 255);
		}
		glRasterPos2i(posX, posY);
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
		if(string[i] == 'I')
			posX += add - low_add;
		else if(string[i] == 'M')
			posX += add + low_add / 2;
		else
			posX += add;
		i++;
	}
}

int firstBut(void){
	if(cursor == false){
		cursor = true;
		desk[0][0].focused.highlighted = true;
		cur_cursor_x = 0;
		cur_cursor_y = 0;
		return 1;
	}
	return 0;
}

void SetCheckers(const int bot_checkers, const int top_checkers, int add_x, int add_y, int i, int k){
	desk[i][k].field = occup;
	if(i <= bot_checkers)
		desk[i][k].checker_side = white;
	else
		desk[i][k].checker_side = black;
	desk[i][k].role = soldier;
}

void clearDesk(void){
	int i = 0, j = 0;
	for(i = 0; i < DESK_SIZE; i++){
		for(j = 0; j < DESK_SIZE; j++){
			desk[i][j].field = empty;
		}
	}
}

void setDesk(void){
	int i = 0, k = 0, add_y = 0, add_x = 0;
	const int bot_checkers = 2, top_checkers = 5; // Включительно
	for(i = 0; i < DESK_SIZE; i++){
		for(k = 0; k < DESK_SIZE; k++){
			desk[i][k].focused.highlighted = false;
			desk[i][k].focused.toMove = false;
			add_y = CubeSize * i;
			add_x = CubeSize * k;
			if((i + 1) % 2){
				if((k + 1) % 2){
					if(i <= bot_checkers || i >= top_checkers)
						SetCheckers(bot_checkers, top_checkers, add_x, add_y, i, k);
					desk[i][k].field_col = black;
					desk[i][k].midLocation.x = (float)(add_x + (CubeSize) / 2);
					desk[i][k].midLocation.y = (float)(add_y + (CubeSize) / 2);
				}else
					desk[i][k].field_col = white;
			}else{
				if((k + 1) % 2)
					desk[i][k].field_col = white;
				else{
					if(i <= bot_checkers || i >= top_checkers)
						SetCheckers(bot_checkers, top_checkers, add_x, add_y, i, k);
					desk[i][k].field_col = black;
					desk[i][k].midLocation.x = (float)(add_x + (CubeSize) / 2);
					desk[i][k].midLocation.y = (float)(add_y + (CubeSize) / 2);
				}
			}
		}
	}
}

void setButtons(void){
	int koef = 0, koef2 = 0;
	const int Distance = 25;
	koef = CubeSize / 3;
	koef2 = CubeSize / 2;
	// В окне игра
	// Первая
	Restart_button.Up_border = CubeSize * DESK_SIZE - CubeSize / 4;
	Restart_button.Down_border = Restart_button.Up_border - koef;
	Restart_button.Left_border = CubeSize * DESK_SIZE + koef2;
	Restart_button.Right_border = CubeSize * DESK_SIZE + CubeSize * 2;
	// Вторая
	Save_Position_button.Up_border = Restart_button.Up_border - koef2;
	Save_Position_button.Down_border = Save_Position_button.Up_border - koef;
	Save_Position_button.Left_border = Restart_button.Left_border;
	Save_Position_button.Right_border = Restart_button.Right_border;
	// Третяя
	Recover_Position_button.Up_border = Save_Position_button.Up_border - koef2;
	Recover_Position_button.Down_border = Recover_Position_button.Up_border - koef;
	Recover_Position_button.Left_border = Restart_button.Left_border;
	Recover_Position_button.Right_border = Restart_button.Right_border;
	// В окне меню
	Developers_button.Up_border = Height / 2 + CubeSize * 2 - Distance;
	Developers_button.Down_border = Developers_button.Up_border - koef;
	Developers_button.Left_border = Width / 2 - CubeSize;
	Developers_button.Right_border = Developers_button.Left_border + CubeSize * 2;

	Records_button.Up_border = Developers_button.Down_border - Distance;
	Records_button.Down_border = Records_button.Up_border - koef;
	Records_button.Left_border = Developers_button.Left_border;
	Records_button.Right_border = Developers_button.Right_border;

	Options_button.Up_border = Records_button.Down_border - Distance;
	Options_button.Down_border = Options_button.Up_border - koef;
	Options_button.Left_border = Developers_button.Left_border;
	Options_button.Right_border = Developers_button.Right_border;
	// В OPTIONS
	Easy_button.Up_border = 486;
	Easy_button.Down_border = 470;
	Easy_button.Left_border = 500;
	Easy_button.Right_border = 555;

	Medium_button.Up_border = 350;
	Medium_button.Down_border = 331;
	Medium_button.Left_border = 482;
	Medium_button.Right_border = 571;
}

void pole(int add_x, int add_y, colors color){
	glBegin(GL_QUADS);

	if(desk[add_y / CubeSize][add_x / CubeSize].focused.highlighted == false){
		if(color == white)
			glColor3ub(230, 230, 230);

		if(color == black)
			glColor3ub(50, 50, 50);
	}else
		glColor3ub(255, 140, 0);
	if(desk[add_y / CubeSize][add_x / CubeSize].focused.toMove == true)
		glColor3ub(255, 190, 0);

	glVertex2f(0 + add_x, 0 + add_y);
	glVertex2f(0 + add_x, CubeSize + add_y);
	glVertex2f(CubeSize + add_x, CubeSize + add_y);
	glVertex2f(CubeSize + add_x, 0 + add_y);

	glEnd();
}

void drawCheckers(int i, int k){
	const int max = 30;
	int Radius = 35;
	float a = 0;
	glLineWidth(10);
	glBegin(GL_LINE_LOOP);

	//glVertex2f(desk[i][k].midLocation.x, desk[i][k].midLocation.y); // Середина

	if(desk[i][k].checker_side == white)
		glColor3ub(230, 230, 230);

	else
		glColor3ub(178, 34, 34);//glColor3ub(150, 75, 0);

	for(int j = 0; j < max; j++){
		a = (float)j / (float)max * 3.1415 * 2.0;

		glVertex2f(desk[i][k].midLocation.x + cosf(a) * (float)Radius,
				   desk[i][k].midLocation.y + sinf(a) * (float)Radius);
	}

	glEnd();

	Radius = 25;

	glBegin(GL_POLYGON);

	for(int j = 0; j < max; j++){
		a = (float)j / (float)max * 3.1415 * 2.0;

		glVertex2f(desk[i][k].midLocation.x + cosf(a) * (float)Radius,
				   desk[i][k].midLocation.y + sinf(a) * (float)Radius);
	}

	glEnd();
	//desk[0][0].role = queen;
	if(desk[i][k].role == queen){
		const int left_right = 17, up_point = 15, down_point = 10;

		glBegin(GL_TRIANGLES);
		glColor3ub(50, 50, 50); // Черный
		// mid
		glVertex2f(desk[i][k].midLocation.x, desk[i][k].midLocation.y + up_point);
		glVertex2f(desk[i][k].midLocation.x - left_right, desk[i][k].midLocation.y - down_point);
		glVertex2f(desk[i][k].midLocation.x + left_right, desk[i][k].midLocation.y - down_point);
		//left
		glVertex2f(desk[i][k].midLocation.x - left_right, desk[i][k].midLocation.y + up_point);
		glVertex2f(desk[i][k].midLocation.x - left_right, desk[i][k].midLocation.y - down_point);
		glVertex2f(desk[i][k].midLocation.x, desk[i][k].midLocation.y - down_point);
		//right
		glVertex2f(desk[i][k].midLocation.x + left_right, desk[i][k].midLocation.y + up_point);
		glVertex2f(desk[i][k].midLocation.x + left_right, desk[i][k].midLocation.y - down_point);
		glVertex2f(desk[i][k].midLocation.x, desk[i][k].midLocation.y - down_point);

		glEnd();
	}
}

void drawButtons(void){
	glBegin(GL_QUADS);

	if(Restart_button.Color == 0)
		glColor3ub(189, 183, 107);
	else
		glColor3ub(165, 42, 42);
	glVertex2f(Restart_button.Left_border, Restart_button.Down_border);
	glVertex2f(Restart_button.Left_border, Restart_button.Up_border);
	glVertex2f(Restart_button.Right_border, Restart_button.Up_border);
	glVertex2f(Restart_button.Right_border, Restart_button.Down_border);

	if(Save_Position_button.Color == 0)
		glColor3ub(189, 183, 107);
	else
		glColor3ub(165, 42, 42);
	glVertex2f(Save_Position_button.Left_border, Save_Position_button.Down_border);
	glVertex2f(Save_Position_button.Left_border, Save_Position_button.Up_border);
	glVertex2f(Save_Position_button.Right_border, Save_Position_button.Up_border);
	glVertex2f(Save_Position_button.Right_border, Save_Position_button.Down_border);

	if(Recover_Position_button.Color == 0)
		glColor3ub(189, 183, 107);
	else
		glColor3ub(165, 42, 42);
	glVertex2f(Recover_Position_button.Left_border, Recover_Position_button.Down_border);
	glVertex2f(Recover_Position_button.Left_border, Recover_Position_button.Up_border);
	glVertex2f(Recover_Position_button.Right_border, Recover_Position_button.Up_border);
	glVertex2f(Recover_Position_button.Right_border, Recover_Position_button.Down_border);

	glEnd();
}

void drawButtonsMenu(void){
	glBegin(GL_QUADS);
	if(Developers_button.Color == 0) // glClearColor(0.7, 0.5, 0.2, 1);
		glColor3ub(189, 183, 107);
	else
		glColor3ub(165, 42, 42);
	glVertex2f(Developers_button.Left_border, Developers_button.Down_border);
	glVertex2f(Developers_button.Left_border, Developers_button.Up_border);
	glVertex2f(Developers_button.Right_border, Developers_button.Up_border);
	glVertex2f(Developers_button.Right_border, Developers_button.Down_border);

	if(Records_button.Color == 0)
		glColor3ub(189, 183, 107);
	else
		glColor3ub(165, 42, 42);
	glVertex2f(Records_button.Left_border, Records_button.Down_border);
	glVertex2f(Records_button.Left_border, Records_button.Up_border);
	glVertex2f(Records_button.Right_border, Records_button.Up_border);
	glVertex2f(Records_button.Right_border, Records_button.Down_border);

	if(Options_button.Color == 0)
		glColor3ub(189, 183, 107);
	else
		glColor3ub(165, 42, 42);
	glVertex2f(Options_button.Left_border, Options_button.Down_border);
	glVertex2f(Options_button.Left_border, Options_button.Up_border);
	glVertex2f(Options_button.Right_border, Options_button.Up_border);
	glVertex2f(Options_button.Right_border, Options_button.Down_border);
	glEnd();
}

void drawTextOnButtonsGame(void){
	const float koef = 0.4;
	int PosX = 0, PosY = 0;
	float AddY = 0;

	PosX = (Restart_button.Left_border + Restart_button.Right_border) / 2;
	AddY = koef * (Restart_button.Up_border - Restart_button.Down_border) / 2;
	PosY = (int)(Restart_button.Down_border + AddY);
	drawText("RESTART", PosX, PosY);

	PosX = (Save_Position_button.Left_border + Save_Position_button.Right_border) / 2;
	AddY = koef * (Save_Position_button.Up_border - Save_Position_button.Down_border) / 2;
	PosY = (int)(Save_Position_button.Down_border + AddY);
	drawText("SAVE", PosX, PosY);

	PosX = (Recover_Position_button.Left_border + Recover_Position_button.Right_border) / 2;
	AddY = koef * (Recover_Position_button.Up_border - Recover_Position_button.Down_border) / 2;
	PosY = (int)(Recover_Position_button.Down_border + AddY);
	drawText("RESTORE", PosX, PosY);
}

void drawTextOnButtonsMenu(void){
	const float koef = 0.4;
	int PosX = 0, PosY = 0;
	float AddY = 0;

	PosX = (Developers_button.Left_border + Developers_button.Right_border) / 2;
	AddY = koef * (Developers_button.Up_border - Developers_button.Down_border) / 2;
	PosY = (int)(Developers_button.Down_border + AddY);
	drawText("DEVELOPERS", PosX, PosY);

	PosX = (Records_button.Left_border + Records_button.Right_border) / 2;
	AddY = koef * (Records_button.Up_border - Records_button.Down_border) / 2;
	PosY = (int)(Records_button.Down_border + AddY);
	drawText("RECORDS", PosX, PosY);

	PosX = (Options_button.Left_border + Options_button.Right_border) / 2;
	AddY = koef * (Options_button.Up_border - Options_button.Down_border) / 2;
	PosY = (int)(Options_button.Down_border + AddY);
	drawText("OPTIONS", PosX, PosY);

	drawText("F1 TO RETURN", 100, 50);
}

void drawTextDevelopers(void){
	const int Distance = 30;
	float MidHeight = Height / 2 + 100;
	float MidWid = Width / 2;

	drawText("DEVELOPED BY GAVRICHKOV DANIIL", MidWid, MidHeight);
	MidHeight -= Distance;
	drawText("AND DIDKOVSKIY MAXIM", MidWid, MidHeight);
	MidHeight -= Distance;
	drawText("POLYTECHNIK UNIVERSITY", MidWid, MidHeight);
	MidHeight -= Distance;
	drawText("2021 YEAR", MidWid, MidHeight);

	drawText("F1 TO QUIT", 100, 50);
}

void drawTextRecords(void){
	if(head != NULL && head->Moves){
		int i = 1, count = 0;
		records* tmp = head;
		char string[BIG_STRING_NAME] = {0}; // Будет строка для записи
		const int Distance = 30;
		float MidHeight = Height / 2 + 200;
		float MidWid = Width / 2;

		while(tmp != NULL && count <= 10){
			GetStringOutOfList(tmp, string, i);
			drawText(string, MidWid, MidHeight);
			tmp = tmp->next;
			MidHeight -= Distance;
			i++;
			count++;
			memset(string, 0, BIG_STRING_NAME);
		}
	}
	drawText("F1 TO QUIT", 100, 50);
}

void drawMenu(void){
	float MidHeight = Height / 2;
	float MidWid = Width / 2;
	float AddHalfX = Width / 2;
	float AddHalfY = Height / 4;

	glColor3ub(240, 105, 30);
	glBegin(GL_QUADS);

	glVertex2f(MidWid - AddHalfX, MidHeight - AddHalfY);
	glVertex2f(MidWid - AddHalfX, MidHeight + AddHalfY);
	glVertex2f(MidWid + AddHalfX, MidHeight + AddHalfY);
	glVertex2f(MidWid + AddHalfX, MidHeight - AddHalfY);

	glEnd();

	drawButtonsMenu();
	drawTextOnButtonsMenu();
}

void drawEndGame(void){
	float MidH = Height / 2;
	float MidW = Width / 2;
	const float AddX = 200, AddY = 300, lowAdd = 20;

	glBegin(GL_QUADS);
	glColor3ub(240, 230, 140);
	glVertex2f(MidW - AddX, MidH - AddY);
	glVertex2f(MidW - AddX, MidH + AddY);
	glVertex2f(MidW + AddX, MidH + AddY);
	glVertex2f(MidW + AddX, MidH - AddY);

	glEnd();
	drawText(Name, MidW, MidH);
	drawText("ENTER NAME", MidW, MidH + AddY - lowAdd * 2);
	drawText("F1 TO QUIT", MidW, MidH - AddY + lowAdd);
}

void drawEndGameLoose(void){
	float MidH = Height / 2;
	float MidW = Width / 2;
	const float AddX = 200, AddY = 300;

	glBegin(GL_QUADS);
	glColor3ub(240, 230, 140);
	glVertex2f(MidW - AddX, MidH - AddY);
	glVertex2f(MidW - AddX, MidH + AddY);
	glVertex2f(MidW + AddX, MidH + AddY);
	glVertex2f(MidW + AddX, MidH - AddY);

	glEnd();
	drawText("YOU'VE LOST, TRY AGAIN", MidW, MidH);
}

void drawOptions(void){
	const int Constanta = 70, Constanta2 = 20;
	float MidH = Height / 2;
	float MidW = Width / 2;
	const float AddX = 200, AddY = 300;

	glBegin(GL_QUADS);
	glColor3ub(240, 230, 140);
	glVertex2f(MidW - AddX, MidH - AddY);
	glVertex2f(MidW - AddX, MidH + AddY);
	glVertex2f(MidW + AddX, MidH + AddY);
	glVertex2f(MidW + AddX, MidH - AddY);
	glEnd();
	drawText("F1 TO QUIT", MidW, MidH - AddY + Constanta2);
	drawText("EASY", MidW, MidH + Constanta);
	drawText("MEDIUM", MidW, MidH - Constanta);
}

void Display(void){
	if(Status == GAME || Status == AFTER_GAME || Status == AFTER_GAME_LOOSE){
		int add_x = 0, i = 0, k = 0, add_y = 0;
		colors color = 0;
		// Устанавливаем цвет фона
		glClearColor(0.7, 0.5, 0.2, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		// Отрисовка доски
		for(i = 0; i < DESK_SIZE; i++){
			for(k = 0; k < DESK_SIZE; k++){
				add_y = CubeSize * i;
				add_x = CubeSize * k;
				color = desk[i][k].field_col;
				pole(add_x, add_y, color);
			}
		}
		// Заполнение шашками
		for(i = 0; i < DESK_SIZE; i++){
			for(k = 0; k < DESK_SIZE; k++){
				if(desk[i][k].field == occup)
					drawCheckers(i, k);
			}
		}

		drawButtons();
		drawTextOnButtonsGame();
		drawText("F1 TO OPEN MENU", (int)(CubeSize * DESK_SIZE + CubeSize * 1.3), CubeSize * 2);

		if(Status == AFTER_GAME){
			drawEndGame();
		}else if(Status == AFTER_GAME_LOOSE){
			drawEndGameLoose();
		}

		glFinish();

		if(Status == AFTER_GAME_LOOSE){
#ifdef WINDOWS
			Sleep(2000);
#endif

#ifdef LINUX
			sleep(2);
#endif
			Status = GAME;
		}
	}else if(Status == MENU || Status == OPTIONS){
		glClearColor(0.7, 0.5, 0.2, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		drawMenu();
		if(Status == OPTIONS)
			drawOptions();
		glFinish();
	}else if(Status == DEVELOPERS){
		glClearColor(0.7, 0.5, 0.2, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		drawTextDevelopers();

		glFinish();
	}else if(Status == SRECORDS){
		glClearColor(0.7, 0.5, 0.2, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		drawTextRecords();

		glFinish();
	}
}

void Reshape(GLint w, GLint h){
	Width = w;
	Height = h;

	// Размеры области отображения
	glViewport(0, 0, w, h);

	// Ортографическая проекция
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1.0, 1.0); //-50, w-50.0, -50, h-50.0, -1.0, 1.0 Чтобы был отступ

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int EatUp(int xx, int yy, colors side_of_eaten, int* EatOrNot, Pressed* EatenOne){
	const int SUCCESS = 1, FAILURE = 0;
	if((Move_from.press_x + 2 == xx) && \
        desk[Move_from.press_y + 1][Move_from.press_x + 1].field == occup && \
        desk[Move_from.press_y + 1][Move_from.press_x + 1].checker_side == side_of_eaten){
		*EatOrNot = EAT;
		EatenOne->press_x = Move_from.press_x + 1;
		EatenOne->press_y = Move_from.press_y + 1;
		return SUCCESS;
	}else if((Move_from.press_x - 2 == xx) && \
        desk[Move_from.press_y + 1][Move_from.press_x - 1].field == occup && \
        desk[Move_from.press_y + 1][Move_from.press_x - 1].checker_side == side_of_eaten){
		*EatOrNot = EAT;
		EatenOne->press_x = Move_from.press_x - 1;
		EatenOne->press_y = Move_from.press_y + 1;
		return SUCCESS;
	}else
		return FAILURE;
}

int EatDown(int xx, int yy, colors side_of_eaten, int* EatOrNot, Pressed* EatenOne){
	const int SUCCESS = 1, FAILURE = 0;
	if((Move_from.press_x + 2 == xx) && \
        desk[Move_from.press_y - 1][Move_from.press_x + 1].field == occup && \
        desk[Move_from.press_y - 1][Move_from.press_x + 1].checker_side == side_of_eaten){
		*EatOrNot = EAT;
		EatenOne->press_x = Move_from.press_x + 1;
		EatenOne->press_y = Move_from.press_y - 1;
		return SUCCESS;
	}else if((Move_from.press_x - 2 == xx) && \
        desk[Move_from.press_y - 1][Move_from.press_x - 1].field == occup && \
        desk[Move_from.press_y - 1][Move_from.press_x - 1].checker_side == side_of_eaten){
		*EatOrNot = EAT;
		EatenOne->press_x = Move_from.press_x - 1;
		EatenOne->press_y = Move_from.press_y - 1;
		return SUCCESS;
	}else
		return FAILURE;
}

int Sub_for_Possible_for_queen(int xx, int yy, int i, int j, int* EnemyOnWay, int* EatOrNot, Pressed* EatenOne){
	const int SUCCESS = 1, FAILURE = 0, WAIT = 2;
	if(desk[i][j].field == occup && desk[i][j].checker_side != turn){
		(*EnemyOnWay)++;
		if((*EnemyOnWay) == 1){
			EatenOne->press_x = j;
			EatenOne->press_y = i;
		}
	}
	if(desk[i][j].field == occup && desk[i][j].checker_side == turn)
		return FAILURE;

	if(i == yy && xx == j){
		if(!(*EnemyOnWay)){
			*EatOrNot = NO_EAT;
			return SUCCESS;
		}
		if((*EnemyOnWay) == 1){
			*EatOrNot = EAT;
			return SUCCESS;
		}else
			return FAILURE;
	}
	return WAIT;
}

int Possible(int xx, int yy, int* EatOrNot, Pressed* EatenOne){
	// Move_from
	// xx && yy - координата поля, куда будет совершен ход
	// Координата откуда будет совершен ход в структуре move_from
	const int SUCCESS = 1, FAILURE = 0;
	colors side_of_eaten;

	if(desk[Move_from.press_y][Move_from.press_x].checker_side == white)
		side_of_eaten = black;
	else
		side_of_eaten = white;

	if(desk[yy][xx].field_col == white || desk[yy][xx].field == occup)
		return FAILURE;

	if(desk[Move_from.press_y][Move_from.press_x].role == soldier){
		if(desk[Move_from.press_y][Move_from.press_x].checker_side == white){
			// Ход без битья
			if((Move_from.press_y + 1 == yy) && ((Move_from.press_x + 1 == xx) || (Move_from.press_x - 1 == xx)))
				return SUCCESS;
		}else{
			if((Move_from.press_y - 1 == yy) && ((Move_from.press_x + 1 == xx) || (Move_from.press_x - 1 == xx)))
				return SUCCESS;
		}
		// Ход со съеданием
		if(Move_from.press_y + 2 == yy){
			return EatUp(xx, yy, side_of_eaten, EatOrNot, EatenOne);
		}else if(Move_from.press_y - 2 == yy){
			return EatDown(xx, yy, side_of_eaten, EatOrNot, EatenOne);
		}else
			return FAILURE;
	}
		// Просчет для дамки
	else if(desk[Move_from.press_y][Move_from.press_x].role == queen){
		int i = 0, j = 0, EnemyOnWay = 0, SUB = 0;
		i = Move_from.press_y;
		j = Move_from.press_x;
		if(Move_from.press_x > xx && Move_from.press_y > yy){//DL
			EnemyOnWay = 0;
			for(i = Move_from.press_y - 1, j = Move_from.press_x - 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i--, j--){
				SUB = Sub_for_Possible_for_queen(xx, yy, i, j, &EnemyOnWay, EatOrNot, EatenOne);
				if(SUB == FAILURE)
					return FAILURE;
				if(SUB == SUCCESS)
					return SUCCESS;
			}
		}else if(Move_from.press_x > xx && Move_from.press_y < yy){
			EnemyOnWay = 0;
			for(i = Move_from.press_y + 1, j = Move_from.press_x - 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i++, j--){
				SUB = Sub_for_Possible_for_queen(xx, yy, i, j, &EnemyOnWay, EatOrNot, EatenOne);
				if(SUB == FAILURE)
					return FAILURE;
				if(SUB == SUCCESS)
					return SUCCESS;
			}
		}else if(Move_from.press_x < xx && Move_from.press_y > yy){
			EnemyOnWay = 0;
			for(i = Move_from.press_y - 1, j = Move_from.press_x + 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i--, j++){
				SUB = Sub_for_Possible_for_queen(xx, yy, i, j, &EnemyOnWay, EatOrNot, EatenOne);
				if(SUB == FAILURE)
					return FAILURE;
				if(SUB == SUCCESS)
					return SUCCESS;
			}
		}else if(Move_from.press_x < xx && Move_from.press_y < yy){
			EnemyOnWay = 0;
			for(i = Move_from.press_y + 1, j = Move_from.press_x + 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i++, j++){
				SUB = Sub_for_Possible_for_queen(xx, yy, i, j, &EnemyOnWay, EatOrNot, EatenOne);
				if(SUB == FAILURE)
					return FAILURE;
				if(SUB == SUCCESS)
					return SUCCESS;
			}
		}
	}
	return SUCCESS;
}

void MoveChecker(const int EatOrNot, Pressed EatenOne){
	Move_to.press_x = cur_cursor_x;
	Move_to.press_y = cur_cursor_y;
	desk[Move_from.press_y][Move_from.press_x].field = empty;
	desk[Move_from.press_y][Move_from.press_x].focused.toMove = false;

	desk[Move_to.press_y][Move_to.press_x].field = occup;
	desk[Move_to.press_y][Move_to.press_x].checker_side = desk[Move_from.press_y][Move_from.press_x].checker_side;
	desk[Move_to.press_y][Move_to.press_x].role = desk[Move_from.press_y][Move_from.press_x].role;
	space_pressed = false;

	if(EatOrNot == EAT){
		desk[EatenOne.press_y][EatenOne.press_x].field = empty;
	}

	WasMoved = true;
}

void AIMoveChecker(const int y, const int x, const int EatOrNot, Pressed EatenOne){
	Move_to.press_x = x;
	Move_to.press_y = y;
	desk[Move_from.press_y][Move_from.press_x].field = empty;
	desk[Move_from.press_y][Move_from.press_x].focused.toMove = false;

	desk[Move_to.press_y][Move_to.press_x].field = occup;
	desk[Move_to.press_y][Move_to.press_x].checker_side = desk[Move_from.press_y][Move_from.press_x].checker_side;
	desk[Move_to.press_y][Move_to.press_x].role = desk[Move_from.press_y][Move_from.press_x].role;
	space_pressed = false;

	if(EatOrNot == EAT){
		desk[EatenOne.press_y][EatenOne.press_x].field = empty;
	}

	WasMoved = true;
}

void PressedToMove(void){
	space_pressed = true;
	Move_from.press_x = cur_cursor_x;
	Move_from.press_y = cur_cursor_y;
	desk[Move_from.press_y][Move_from.press_x].focused.toMove = true;
}

int PossibilityToEatUp(const int yy, const int xx){
	const int POSSIBLE = 1, NOT_POSSIBLE = 0;
	if(xx + 2 <= DESK_SIZE - 1 && yy + 2 <= DESK_SIZE - 1){
		if((desk[yy + 2][xx + 2].field == empty) && \
            desk[yy + 1][xx + 1].field == occup && \
            desk[yy + 1][xx + 1].checker_side != turn)
			return POSSIBLE;
	}
	if(xx - 2 >= 0 && yy + 2 <= DESK_SIZE - 1){
		if((desk[yy + 2][xx - 2].field == empty) && \
            desk[yy + 1][xx - 1].field == occup && \
            desk[yy + 1][xx - 1].checker_side != turn)
			return POSSIBLE;
	}
	return NOT_POSSIBLE;
}

int AIPossibilityToEatUp(const int yy, const int xx){
	if(xx + 2 <= DESK_SIZE - 1 && yy + 2 <= DESK_SIZE - 1){
		if((desk[yy + 2][xx + 2].field == empty) && \
            desk[yy + 1][xx + 1].field == occup && \
            desk[yy + 1][xx + 1].checker_side != turn)
			return UR;
	}
	if(xx - 2 >= 0 && yy + 2 <= DESK_SIZE - 1){
		if((desk[yy + 2][xx - 2].field == empty) && \
            desk[yy + 1][xx - 1].field == occup && \
            desk[yy + 1][xx - 1].checker_side != turn)
			return UL;
	}
	return IMP;
}

int PossibilityToEatDown(const int yy, const int xx){
	const int POSSIBLE = 1, NOT_POSSIBLE = 0;
	if(xx + 2 <= DESK_SIZE - 1 && yy - 2 >= 0){
		if((desk[yy - 2][xx + 2].field == empty) && \
            desk[yy - 1][xx + 1].field == occup && \
            desk[yy - 1][xx + 1].checker_side != turn)
			return POSSIBLE;
	}
	if(xx - 2 >= 0 && yy - 2 >= 0){
		if((desk[yy - 2][xx - 2].field == empty) && \
            desk[yy - 1][xx - 1].field == occup && \
            desk[yy - 1][xx - 1].checker_side != turn)
			return POSSIBLE;
	}
	return NOT_POSSIBLE;
}

int AIPossibilityToEatDown(const int yy, const int xx){

	if(xx + 2 <= DESK_SIZE - 1 && yy - 2 >= 0){
		if((desk[yy - 2][xx + 2].field == empty) && \
            desk[yy - 1][xx + 1].field == occup && \
            desk[yy - 1][xx + 1].checker_side != turn)
			return DR;
	}
	if(xx - 2 >= 0 && yy - 2 >= 0){
		if((desk[yy - 2][xx - 2].field == empty) && \
            desk[yy - 1][xx - 1].field == occup && \
            desk[yy - 1][xx - 1].checker_side != turn)
			return DL;
	}
	return IMP;
}

int PossibilityToEatByQueen(const int yy, const int xx){
	const int POSSIBLE = 1, NOT_POSSIBLE = 0;
	int EnemyOnWay = 0;
	int i = 0, j = 0;

	for(i = yy + 1, j = xx - 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i++, j--){
		if(desk[i][j].field == occup){
			if(desk[i][j].checker_side != turn){
				EnemyOnWay++;
				if(EnemyOnWay > 1)
					break;
			}
			if(desk[i][j].checker_side == turn)
				break;
			else if((j - 1) >= 0 && (i + 1) < DESK_SIZE && desk[i + 1][j - 1].field == empty)
				return POSSIBLE;
		}
	}
	EnemyOnWay = 0;
	for(i = yy + 1, j = xx + 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i++, j++){
		if(desk[i][j].field == occup){
			if(desk[i][j].checker_side != turn){
				EnemyOnWay++;
				if(EnemyOnWay > 1)
					break;
			}
			if(desk[i][j].checker_side == turn)
				break;
			else if((j + 1) < DESK_SIZE && (i + 1) < DESK_SIZE && desk[i + 1][j + 1].field == empty)
				return POSSIBLE;
		}
	}
	EnemyOnWay = 0;
	for(i = yy - 1, j = xx - 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i--, j--){
		if(desk[i][j].field == occup){
			if(desk[i][j].checker_side != turn){
				EnemyOnWay++;
				if(EnemyOnWay > 1)
					break;
			}
			if(desk[i][j].checker_side == turn)
				break;
			else if((j - 1) >= 0 && (i - 1) >= 0 && desk[i - 1][j - 1].field == empty)
				return POSSIBLE;
		}
	}
	EnemyOnWay = 0;
	for(i = yy - 1, j = xx + 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i--, j++){
		if(desk[i][j].field == occup){
			if(desk[i][j].checker_side != turn){
				EnemyOnWay++;
				if(EnemyOnWay > 1)
					break;
			}
			if(desk[i][j].checker_side == turn)
				break;
			else if((j + 1) < DESK_SIZE && (i - 1) >= 0 && desk[i - 1][j + 1].field == empty)
				return POSSIBLE;
		}
	}

	return NOT_POSSIBLE;
}

int AIPossibilityToEatByQueen(const int yy, const int xx, int* y, int* x, int* direction){
	//yy & xx - pos of Q, y & x - pos of Q after move
	const int POSSIBLE = 1, NOT_POSSIBLE = 0;
	int EnemyOnWay = 0;
	int i = 0, j = 0;
	*y = -100;
	*x = -100;
	for(i = yy + 1, j = xx - 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i++, j--){//UL
		if(desk[i][j].field == occup){
			if(desk[i][j].checker_side != turn){
				EnemyOnWay++;
				if(EnemyOnWay > 1)
					break;
			}
			if(desk[i][j].checker_side == turn)
				break;
			else if((j - 1) >= 0 && (i + 1) < DESK_SIZE && desk[i + 1][j - 1].field == empty){
				*direction = UL;
				*y = i + 1;
				*x = j - 1;
				return POSSIBLE;
			}
		}
	}
	EnemyOnWay = 0;
	for(i = yy + 1, j = xx + 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i++, j++){//UR
		if(desk[i][j].field == occup){
			if(desk[i][j].checker_side != turn){
				EnemyOnWay++;
				if(EnemyOnWay > 1)
					break;
			}
			if(desk[i][j].checker_side == turn)
				break;
			else if((j + 1) < DESK_SIZE && (i + 1) < DESK_SIZE && desk[i + 1][j + 1].field == empty){
				*direction = UR;
				*y = i + 1;
				*x = j + 1;
				return POSSIBLE;
			}
		}
	}
	EnemyOnWay = 0;
	for(i = yy - 1, j = xx - 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i--, j--){//DL
		if(desk[i][j].field == occup){
			if(desk[i][j].checker_side != turn){
				EnemyOnWay++;
				if(EnemyOnWay > 1)
					break;
			}
			if(desk[i][j].checker_side == turn)
				break;
			else if((j - 1) >= 0 && (i - 1) >= 0 && desk[i - 1][j - 1].field == empty){
				*direction = DL;
				*y = i - 1;
				*x = j - 1;
				return POSSIBLE;
			}
		}
	}
	EnemyOnWay = 0;
	for(i = yy - 1, j = xx + 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i--, j++){//DR
		if(desk[i][j].field == occup){
			if(desk[i][j].checker_side != turn){
				EnemyOnWay++;
				if(EnemyOnWay > 1)
					break;
			}
			if(desk[i][j].checker_side == turn)
				break;
			else if((j + 1) < DESK_SIZE && (i - 1) >= 0 && desk[i - 1][j + 1].field == empty){
				*direction = DR;
				*y = i - 1;
				*x = j + 1;
				return POSSIBLE;
			}
		}
	}
	*direction = IMP;
	return NOT_POSSIBLE;
}

int PossibleToEat(void){
	// turn - чей ход, глобальная переменная
	int i = 0, j = 0;
	const int POSSIBLE = 1, NOT_POSSIBLE = 0;

	for(i = 0; i < DESK_SIZE; i++){
		for(j = 0; j < DESK_SIZE; j++){
			if(desk[i][j].field == occup && desk[i][j].checker_side == turn && desk[i][j].role == soldier){
				if(PossibilityToEatDown(i, j) || PossibilityToEatUp(i, j))
					return POSSIBLE;
			}else if(desk[i][j].field == occup && desk[i][j].checker_side == turn && desk[i][j].role == queen){
				if(PossibilityToEatByQueen(i, j))
					return POSSIBLE;
			}
		}
	}
	return NOT_POSSIBLE;
}

int PossibleToGoUpOrDown(void){
	const int POSSIBLE = 1, NOT_POSSIBLE = 0;
	for(int i = 0; i < DESK_SIZE; i++){
		for(int j = 0; j < DESK_SIZE; j++){
			if(desk[i][j].field == occup && desk[i][j].checker_side == turn && desk[i][j].role == soldier){// простой ход шашкой вперед
				if(desk[i][j].checker_side == white){
					if(i + 1 < DESK_SIZE && j - 1 >= 0 && desk[i + 1][j - 1].field == empty)
						return POSSIBLE;
					if(i + 1 < DESK_SIZE && j + 1 < DESK_SIZE && desk[i + 1][j + 1].field == empty)
						return POSSIBLE;
				}else{
					if(i - 1 >= 0 && j - 1 >= 0 && desk[i - 1][j - 1].field == empty)
						return POSSIBLE;
					if(i - 1 >= 0 && j + 1 < DESK_SIZE && desk[i - 1][j + 1].field == empty)
						return POSSIBLE;
				}
			}else if(desk[i][j].field == occup && desk[i][j].checker_side == turn && desk[i][j].role == queen){
				if(i + 1 < DESK_SIZE && j - 1 >= 0 && desk[i + 1][j - 1].field == empty){//UL
					return POSSIBLE;
				}
				if(j + 1 < DESK_SIZE && i + 1 < DESK_SIZE && desk[i + 1][j + 1].field == empty){//UR
					return POSSIBLE;
				}
				if(i - 1 >= 0 && j - 1 >= 0 && desk[i - 1][j - 1].field == empty){//DL
					return POSSIBLE;
				}
				if(j + 1 < DESK_SIZE && i - 1 >= 0 && desk[i - 1][j + 1].field == empty){//DR
					return POSSIBLE;
				}
			}
		}
	}
	return NOT_POSSIBLE;
}

int EndOfMove(int EatOrNot){
	// Поскольку ход был совершен -- координаты ходившей шашки в Move_to
	const int END = 1, NOT_END = 0;
	int cur_place_y = Move_to.press_y, cur_place_x = Move_to.press_x;

	if(EatOrNot == NO_EAT)
		return END;

	if(desk[cur_place_y][cur_place_x].role == soldier){
		if(PossibilityToEatUp(cur_place_y, cur_place_x))
			return NOT_END;
		if(PossibilityToEatDown(cur_place_y, cur_place_x))
			return NOT_END;
	}else // Если дамка
	{
		if(PossibilityToEatByQueen(cur_place_y, cur_place_x))
			return NOT_END;
	}
	return END; // Если действительно конец хода
}

void MakeQueen(void){
	desk[Move_to.press_y][Move_to.press_x].role = queen;
}

int CheckEndEmpty(void){
	const int END = 1, CONTINUE = 0;
	int i = 0, j = 0;
	for(i = 0; i < DESK_SIZE; i++){
		for(j = 0; j < DESK_SIZE; j++){
			if(desk[i][j].checker_side != turn && desk[i][j].field == occup)
				return CONTINUE;
		}
	}
	return END;
}

int CheckEndNoMove(void){
	const int END = 1, CONTINUE = 0;

	if(PossibleToEat() || PossibleToGoUpOrDown())
		return CONTINUE;

	return END;
}

void Enter(void){
	desk[Move_to.press_y][Move_to.press_x].focused.toMove = false;
	if(turn == white)
		turn = black;
	else
		turn = white;
	WasMoved = false;
	space_pressed = false;
}

void EatAsMuchAsPossible(const int y, const int x, int role){
	//y && x - cur pos of Checker
	if(x < 0 || x >= DESK_SIZE || y < 0 || y >= DESK_SIZE){
		return;
	}

	if(role == soldier){
		int direction = -1, EatOrNot = NO_EAT;

		if((direction = AIPossibilityToEatDown(y, x)) ||
				(direction = AIPossibilityToEatUp(y, x))){
			EatOrNot = EAT;
		}else{
			return;
		}

		Pressed EatenOne = {0, 0};
		cur_cursor_x = x;
		cur_cursor_y = y;
		PressedToMove();
		if(direction == DR){
			EatenOne.press_x = cur_cursor_x + 1;
			EatenOne.press_y = cur_cursor_y - 1;
#ifdef WINDOWS
			Sleep(700);
#endif

#ifdef LINUX
			sleep(0.7);
#endif
			AIMoveChecker(cur_cursor_y - 2, cur_cursor_x + 2, EatOrNot, EatenOne);
			if(Move_to.press_y == 0){
				MakeQueen();
				Display();
				EatAsMuchAsPossible(cur_cursor_y - 2, cur_cursor_x + 2, queen);
			}else{
				Display();
				EatAsMuchAsPossible(cur_cursor_y - 2, cur_cursor_x + 2, role);
			}
		}
		if(direction == DL){
			EatenOne.press_x = cur_cursor_x - 1;
			EatenOne.press_y = cur_cursor_y - 1;
#ifdef WINDOWS
			Sleep(700);
#endif

#ifdef LINUX
			sleep(0.7);
#endif
			AIMoveChecker(cur_cursor_y - 2, cur_cursor_x - 2, EatOrNot, EatenOne);
			if(Move_to.press_y == 0){
				MakeQueen();
				Display();
				EatAsMuchAsPossible(cur_cursor_y - 2, cur_cursor_x + 2, queen);
			}else{
				Display();
				EatAsMuchAsPossible(cur_cursor_y - 2, cur_cursor_x + 2, role);
			}
		}
		if(direction == UR){
			EatenOne.press_x = cur_cursor_x + 1;
			EatenOne.press_y = cur_cursor_y + 1;
#ifdef WINDOWS
			Sleep(700);
#endif

#ifdef LINUX
			sleep(0.7);
#endif
			AIMoveChecker(cur_cursor_y + 2, cur_cursor_x + 2, EatOrNot, EatenOne);
			Display();
			EatAsMuchAsPossible(cur_cursor_y + 2, cur_cursor_x + 2, role);
		}
		if(direction == UL){
			EatenOne.press_x = cur_cursor_x - 1;
			EatenOne.press_y = cur_cursor_y + 1;
#ifdef WINDOWS
			Sleep(700);
#endif

#ifdef LINUX
			sleep(0.7);
#endif
			AIMoveChecker(cur_cursor_y + 2, cur_cursor_x - 2, EatOrNot, EatenOne);
			Display();
			EatAsMuchAsPossible(cur_cursor_y + 2, cur_cursor_x - 2, role);
		}
	}
	if(role == queen){
		int direction = -1, EatOrNot = NO_EAT;
		Pressed EatenOne = {0, 0};
		int xx = -100, yy = -100;
		if(AIPossibilityToEatByQueen(y, x, &yy, &xx, &direction)){
			EatOrNot = EAT;
			cur_cursor_x = x;
			cur_cursor_y = y;
			PressedToMove();
			int EnemyOnWay = 0;
			//если мы можем сходить так, чтобы потом иметь возможность срубить, то в циклах for ищется эта позиция
			if(direction == UL){
				EatenOne.press_x = xx + 1;
				EatenOne.press_y = yy - 1;
				EnemyOnWay = 0;
				for(int i = yy + 1, j = xx - 1; i < DESK_SIZE && j >= 0; i++, j--){//UL
					if(desk[i][j].field == empty){
						int X = -100, Y = -100;
						direction = -1;
						if(AIPossibilityToEatByQueen(i, j, &Y, &X, &direction)){
#ifdef WINDOWS
							Sleep(700);
#endif

#ifdef LINUX
							sleep(0.7);
#endif
							AIMoveChecker(i, j, EatOrNot, EatenOne);
							Display();
							EatAsMuchAsPossible(i, j, role);
							return;
						}
					}else{
						break;
					}
				}
			}
			if(direction == UR){
				EatenOne.press_x = xx - 1;
				EatenOne.press_y = yy - 1;
				EnemyOnWay = 0;
				for(int i = yy + 1, j = xx + 1; i < DESK_SIZE && j < DESK_SIZE; i++, j++){//UR
					if(desk[i][j].field == empty){
						int X = -100, Y = -100;
						direction = -1;
						if(AIPossibilityToEatByQueen(i, j, &Y, &X, &direction)){
#ifdef WINDOWS
							Sleep(700);
#endif

#ifdef LINUX
							sleep(0.7);
#endif
							AIMoveChecker(i, j, EatOrNot, EatenOne);
							Display();
							EatAsMuchAsPossible(i, j, role);
							return;
						}
					}else{
						break;
					}
				}
			}
			if(direction == DL){
				EatenOne.press_x = xx + 1;
				EatenOne.press_y = yy + 1;
				EnemyOnWay = 0;
				for(int i = yy - 1, j = xx - 1; i >= 0 && j >= 0; i--, j--){//DL
					if(desk[i][j].field == empty){
						int X = -100, Y = -100;
						direction = -1;
						if(AIPossibilityToEatByQueen(i, j, &Y, &X, &direction)){
#ifdef WINDOWS
							Sleep(700);
#endif

#ifdef LINUX
							sleep(0.7);
#endif
							AIMoveChecker(i, j, EatOrNot, EatenOne);
							Display();
							EatAsMuchAsPossible(i, j, role);
							return;
						}
					}else{
						break;
					}
				}
			}
			if(direction == DR){
				EatenOne.press_x = xx - 1;
				EatenOne.press_y = yy + 1;
				EnemyOnWay = 0;
				for(int i = yy - 1, j = xx + 1; j < DESK_SIZE && i >= 0; i--, j++){//DR
					if(desk[i][j].field == empty){
						int X = -100, Y = -100;
						direction = -1;
						if(AIPossibilityToEatByQueen(i, j, &Y, &X, &direction)){
#ifdef WINDOWS
							Sleep(700);
#endif

#ifdef LINUX
							sleep(0.7);
#endif
							AIMoveChecker(i, j, EatOrNot, EatenOne);
							Display();
							EatAsMuchAsPossible(i, j, role);
							return;
						}
					}else{
						break;
					}
				}
			}
#ifdef WINDOWS
			Sleep(700);
#endif

#ifdef LINUX
			sleep(0.7);
#endif
			AIMoveChecker(yy, xx, EatOrNot, EatenOne);
			Display();
			EatAsMuchAsPossible(yy, xx, role);
			return;
		}else{
			return;
		}
	}
}

void AIProPlayer(void){

	int EatOrNot = NO_EAT, PossibilityToEat = 0;
	Pressed EatenOne = {0, 0}; // Координаты съеденной, если таковая будет
	PossibilityToEat = PossibleToEat();

	if(!PossibilityToEat){
		const int SOLDIER = 0, QUEEN = 1;
		int TOTAL_ROLE = 0, QUIT = FALSE, TOTAL_VAR = -1;
		int tmp = 0;
		unsigned int Schet = 0;
		Coordinates_ Coordinates[MAX_POSITIONS];
		for(int i = 0; i < MAX_POSITIONS; i++){
			Coordinates[i].base_x = -1;
			Coordinates[i].base_y = -1;
			Coordinates[i].ROLE = -1;
			Coordinates[i].to_x = -1;
			Coordinates[i].to_y = -1;
		}
		EatOrNot = NO_EAT;
		for(cur_cursor_y = DESK_SIZE - 1; cur_cursor_y >= 0; --cur_cursor_y){
			for(cur_cursor_x = DESK_SIZE - 1; cur_cursor_x >= 0; --cur_cursor_x){
				if((space_pressed == false) && (desk[cur_cursor_y][cur_cursor_x].field == occup) && desk[cur_cursor_y][cur_cursor_x].checker_side == turn){//только начало хода
					if(desk[cur_cursor_y][cur_cursor_x].role == soldier){

						if(cur_cursor_y - 1 >= 0 && cur_cursor_x - 1 >= 0 && desk[cur_cursor_y - 1][cur_cursor_x - 1].field == empty){//DL
							if(Schet < MAX_POSITIONS){
								Coordinates[Schet].base_x = cur_cursor_x;
								Coordinates[Schet].base_y = cur_cursor_y;
								Coordinates[Schet].to_x = cur_cursor_x - 1;
								Coordinates[Schet].to_y = cur_cursor_y - 1;
								Coordinates[Schet].ROLE = SOLDIER;
								Schet++;
							}else if(rand() % 2){
								tmp = rand() % MAX_POSITIONS;
								Coordinates[tmp].base_x = cur_cursor_x;
								Coordinates[tmp].base_y = cur_cursor_y;
								Coordinates[tmp].to_x = cur_cursor_x - 1;
								Coordinates[tmp].to_y = cur_cursor_y - 1;
								Coordinates[tmp].ROLE = SOLDIER;
							}
						}
						if(cur_cursor_y - 1 >= 0 && cur_cursor_x + 1 < DESK_SIZE && desk[cur_cursor_y - 1][cur_cursor_x + 1].field == empty){//DR
							if(Schet < MAX_POSITIONS){
								Coordinates[Schet].base_x = cur_cursor_x;
								Coordinates[Schet].base_y = cur_cursor_y;
								Coordinates[Schet].to_x = cur_cursor_x + 1;
								Coordinates[Schet].to_y = cur_cursor_y - 1;
								Coordinates[Schet].ROLE = SOLDIER;
								Schet++;
							}else if(rand() % 2){
								tmp = rand() % MAX_POSITIONS;
								Coordinates[tmp].base_x = cur_cursor_x;
								Coordinates[tmp].base_y = cur_cursor_y;
								Coordinates[tmp].to_x = cur_cursor_x + 1;
								Coordinates[tmp].to_y = cur_cursor_y - 1;
								Coordinates[tmp].ROLE = SOLDIER;
							}
						}
					}else{
						// Для дамки
						for(int i = cur_cursor_y + 1, j = cur_cursor_x - 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i++, j--){
							if(desk[i][j].field == occup){
								break;
							}else{
								if(rand() % 2){
									Coordinates[MAX_POSITIONS - 1].base_x = cur_cursor_x;
									Coordinates[MAX_POSITIONS - 1].base_y = cur_cursor_y;
									Coordinates[MAX_POSITIONS - 1].to_x = j;
									Coordinates[MAX_POSITIONS - 1].to_y = i;
									Coordinates[MAX_POSITIONS - 1].ROLE = QUEEN;
								}
							}
						}
						for(int i = cur_cursor_y + 1, j = cur_cursor_x + 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i++, j++){
							if(desk[i][j].field == occup){
								break;
							}else{
								if(rand() % 2){
									Coordinates[MAX_POSITIONS - 2].base_x = cur_cursor_x;
									Coordinates[MAX_POSITIONS - 2].base_y = cur_cursor_y;
									Coordinates[MAX_POSITIONS - 2].to_x = j;
									Coordinates[MAX_POSITIONS - 2].to_y = i;
									Coordinates[MAX_POSITIONS - 2].ROLE = QUEEN;
								}
							}
						}
						for(int i = cur_cursor_y - 1, j = cur_cursor_x - 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i--, j--){
							if(desk[i][j].field == occup){
								break;
							}else{
								if(rand() % 2){
									Coordinates[MAX_POSITIONS - 3].base_x = cur_cursor_x;
									Coordinates[MAX_POSITIONS - 3].base_y = cur_cursor_y;
									Coordinates[MAX_POSITIONS - 3].to_x = j;
									Coordinates[MAX_POSITIONS - 3].to_y = i;
									Coordinates[MAX_POSITIONS - 3].ROLE = QUEEN;
								}
							}
						}
						for(int i = cur_cursor_y - 1, j = cur_cursor_x + 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i--, j++){
							if(desk[i][j].field == occup){
								break;
							}else{
								if(rand() % 2){
									Coordinates[MAX_POSITIONS - 4].base_x = cur_cursor_x;
									Coordinates[MAX_POSITIONS - 4].base_y = cur_cursor_y;
									Coordinates[MAX_POSITIONS - 4].to_x = j;
									Coordinates[MAX_POSITIONS - 4].to_y = i;
									Coordinates[MAX_POSITIONS - 4].ROLE = QUEEN;
								}
							}
						}
					}
				}
			}
		}
		// Выбор из выбранных
		while(!QUIT){
			TOTAL_VAR = rand() % MAX_POSITIONS;
			if(Coordinates[TOTAL_VAR].base_x != -1)
				QUIT = TRUE;
		}
		cur_cursor_y = Coordinates[TOTAL_VAR].base_y;
		cur_cursor_x = Coordinates[TOTAL_VAR].base_x;
		PressedToMove();
		AIMoveChecker(Coordinates[TOTAL_VAR].to_y, Coordinates[TOTAL_VAR].to_x, EatOrNot, EatenOne);
		if(Move_to.press_y == 0 && TOTAL_ROLE == SOLDIER)
			MakeQueen();
		Display();
		return;
	}

	for(cur_cursor_y = DESK_SIZE - 1; cur_cursor_y >= 0; --cur_cursor_y){
		for(cur_cursor_x = DESK_SIZE - 1; cur_cursor_x >= 0; --cur_cursor_x){

			if((space_pressed == false) && (desk[cur_cursor_y][cur_cursor_x].field == occup) && desk[cur_cursor_y][cur_cursor_x].checker_side == turn){//только начало хода
				if(desk[cur_cursor_y][cur_cursor_x].role == soldier){
					int direction = -1;
					if((direction = AIPossibilityToEatDown(cur_cursor_y, cur_cursor_x))){
						PressedToMove();
						EatOrNot = EAT;
						if(direction == DR){
							EatenOne.press_x = cur_cursor_x + 1;
							EatenOne.press_y = cur_cursor_y - 1;
							AIMoveChecker(cur_cursor_y - 2, cur_cursor_x + 2, EatOrNot, EatenOne);
							if(Move_to.press_y == 0)
								MakeQueen();
							Display();
							EatAsMuchAsPossible(cur_cursor_y - 2, cur_cursor_x + 2, soldier);
						}
						if(direction == DL){
							EatenOne.press_x = cur_cursor_x - 1;
							EatenOne.press_y = cur_cursor_y - 1;
							AIMoveChecker(cur_cursor_y - 2, cur_cursor_x - 2, EatOrNot, EatenOne);
							if(Move_to.press_y == 0)
								MakeQueen();
							Display();
							EatAsMuchAsPossible(cur_cursor_y - 2, cur_cursor_x - 2, soldier);
						}
						return;
					}else if((direction = AIPossibilityToEatUp(cur_cursor_y, cur_cursor_x))){
						PressedToMove();
						EatOrNot = EAT;
						if(direction == UR){
							EatenOne.press_x = cur_cursor_x + 1;
							EatenOne.press_y = cur_cursor_y + 1;
							AIMoveChecker(cur_cursor_y + 2, cur_cursor_x + 2, EatOrNot, EatenOne);
							Display();
							EatAsMuchAsPossible(cur_cursor_y + 2, cur_cursor_x + 2, soldier);
						}
						if(direction == UL){
							EatenOne.press_x = cur_cursor_x - 1;
							EatenOne.press_y = cur_cursor_y + 1;
							AIMoveChecker(cur_cursor_y + 2, cur_cursor_x - 2, EatOrNot, EatenOne);
							Display();
							EatAsMuchAsPossible(cur_cursor_y + 2, cur_cursor_x - 2, soldier);
						}
						return;
					}

				}else if(desk[cur_cursor_y][cur_cursor_x].role == queen){
					int x = -100, y = -100, direction = -1;
					if(AIPossibilityToEatByQueen(cur_cursor_y,
												 cur_cursor_x,
												 &y,
												 &x,
												 &direction)){
						EatOrNot = EAT;
						PressedToMove();

						int EnemyOnWay = 0;
						//если мы можем сходить так, чтобы потом иметь возможность срубить, то в циклах for ищется эта позиция
						if(direction == UL){
							EatenOne.press_x = x + 1;
							EatenOne.press_y = y - 1;
							EnemyOnWay = 0;
							for(int i = y + 1, j = x - 1; i < DESK_SIZE && j >= 0; i++, j--){//UL
								if(desk[i][j].field == empty){
									int X = -100, Y = -100;
									direction = -1;
									if(AIPossibilityToEatByQueen(i, j, &Y, &X, &direction)){
										AIMoveChecker(i, j, EatOrNot, EatenOne);
										Display();
										EatAsMuchAsPossible(i, j, queen);
										return;
									}
								}else{
									break;
								}
							}
						}
						if(direction == UR){
							EatenOne.press_x = x - 1;
							EatenOne.press_y = y - 1;
							EnemyOnWay = 0;
							for(int i = y + 1, j = x + 1; i < DESK_SIZE && j < DESK_SIZE; i++, j++){//UR
								if(desk[i][j].field == empty){
									int X = -100, Y = -100;
									direction = -1;
									if(AIPossibilityToEatByQueen(i, j, &Y, &X, &direction)){
										AIMoveChecker(i, j, EatOrNot, EatenOne);
										Display();
										EatAsMuchAsPossible(i, j, queen);
										return;
									}
								}else{
									break;
								}
							}
						}
						if(direction == DL){
							EatenOne.press_x = x + 1;
							EatenOne.press_y = y + 1;
							EnemyOnWay = 0;
							for(int i = y - 1, j = x - 1; i >= 0 && j >= 0; i--, j--){//DL
								if(desk[i][j].field == empty){
									int X = -100, Y = -100;
									direction = -1;
									if(AIPossibilityToEatByQueen(i, j, &Y, &X, &direction)){

										AIMoveChecker(i, j, EatOrNot, EatenOne);
										Display();
										EatAsMuchAsPossible(i, j, queen);
										return;
									}
								}else{
									break;
								}
							}
						}
						if(direction == DR){
							EatenOne.press_x = x - 1;
							EatenOne.press_y = y + 1;
							EnemyOnWay = 0;
							for(int i = y - 1, j = x + 1; j < DESK_SIZE && i >= 0; i--, j++){//DR
								if(desk[i][j].field == empty){
									int X = -100, Y = -100;
									direction = -1;
									if(AIPossibilityToEatByQueen(i, j, &Y, &X, &direction)){
										AIMoveChecker(i, j, EatOrNot, EatenOne);
										Display();
										EatAsMuchAsPossible(i, j, queen);
										return;
									}
								}else{
									break;
								}
							}
						}
						AIMoveChecker(y, x, EatOrNot, EatenOne);
						Display();
						EatAsMuchAsPossible(y, x, queen);
						return;
					}
				}
			}
		}
	}
}

void CountCheckers(int* White, int* Black){
	int i = 0, j = 0;
	for(i = 0; i < DESK_SIZE; i++){
		for(j = 0; j < DESK_SIZE; j++){
			if(desk[i][j].field == occup){
				if(desk[i][j].checker_side == white)
					(*White)++;
				else
					(*Black)++;
			}
		}
	}
}

int PhaseFoo(int White, int Black){
//Debut, MittelSpiel, EndSpiel
	if(White <= 5 || Black <= 5)
		return EndSpiel;
	else if(White < 10 || Black < 10)
		return MittelSpiel;
	else
		return Debut;
}

int ExChange(int yy, int xx, int* y_to, int* x_to){
	if(desk[yy][xx].role == soldier){
		if(yy + 2 < DESK_SIZE && xx + 2 < DESK_SIZE && xx - 2 >= 0 && yy - 2 >= 0){
			// DL
			if(desk[yy + 2][xx + 2].field == occup && desk[yy + 2][xx + 2].checker_side == turn &&
					desk[yy - 1][xx - 1].field == empty && desk[yy + 1][xx + 1].field == occup
					&& desk[yy + 1][xx + 1].checker_side == turn && desk[yy - 2][xx - 2].field == occup && desk[yy - 2][xx - 2].checker_side != turn){
				// Необходима проверка, не будет ли у врага серии
				if(!((desk[yy - 1][xx + 1].field == occup && desk[yy - 1][xx + 1].checker_side == turn && desk[yy - 2][xx + 2].field == empty)
						|| (desk[yy + 1][xx - 1].field == occup && desk[yy + 1][xx - 1].checker_side == turn && desk[yy + 2][xx - 2].field == empty))){
					(*y_to) = yy - 1;
					(*x_to) = xx - 1;
					return TRUE;
				}
			}
			// DR
			if(desk[yy + 2][xx - 2].field == occup && desk[yy + 2][xx - 2].checker_side == turn &&
					desk[yy - 1][xx + 1].field == empty && desk[yy + 1][xx - 1].field == occup
					&& desk[yy + 1][xx - 1].checker_side == turn && desk[yy - 2][xx + 2].field == occup && desk[yy - 2][xx + 2].checker_side != turn){
				if(!((desk[yy - 1][xx - 1].field == occup && desk[yy - 1][xx - 1].checker_side == turn && desk[yy - 2][xx - 2].field == empty)
						|| (desk[yy + 1][xx + 1].field == occup && desk[yy + 1][xx + 1].checker_side == turn && desk[yy + 2][xx + 2].field == empty))){
					(*y_to) = yy - 1;
					(*x_to) = xx + 1;
					return TRUE;
				}
			}
		}
		// DR
		if(xx - 4 >= 0 && (desk[yy - 2][xx - 2].field == empty || desk[yy - 2][xx - 2].checker_side == turn) && yy - 2 >= 0 && yy + 2 < DESK_SIZE
				&& desk[yy - 1][xx - 1].field == empty
				&& desk[yy - 2][xx].field == occup && desk[yy - 2][xx].checker_side != turn &&
				desk[yy + 1][xx - 1].field == occup && desk[yy + 1][xx - 1].checker_side == turn &&
				desk[yy + 2][xx].field == occup && desk[yy + 2][xx].checker_side == turn &&
				desk[yy + 1][xx - 3].field == occup && desk[yy - 1][xx - 3].checker_side == turn &&
				desk[yy + 2][xx - 4].field == occup && desk[yy + 2][xx - 4].checker_side == turn &&
				(desk[yy - 1][xx - 3].field == empty || (desk[yy - 1][xx - 3].field == occup && desk[yy - 1][xx - 3].checker_side == turn
						&& desk[yy - 2][xx - 4].field == occup && desk[yy - 2][xx - 4].checker_side == turn))){
			(*y_to) = yy - 1;
			(*x_to) = xx - 1;
			return TRUE;
		}
		// DL
		if(yy - 2 >= 0 && yy + 2 < DESK_SIZE &&
				xx + 4 < DESK_SIZE && desk[yy - 1][xx + 1].field == empty && (desk[yy - 2][xx + 2].field == empty || desk[yy - 2][xx + 2].checker_side == turn)
				&& desk[yy - 2][xx].field == occup && desk[yy - 2][xx].checker_side != turn &&
				desk[yy + 1][xx + 1].field == occup && desk[yy + 1][xx + 1].checker_side == turn &&
				desk[yy + 2][xx].field == occup && desk[yy + 2][xx].checker_side == turn &&
				desk[yy + 1][xx + 3].field == occup && desk[yy + 1][xx + 3].checker_side == turn &&
				desk[yy + 2][xx + 4].field == occup && desk[yy + 2][xx + 4].checker_side == turn &&
				(desk[yy - 1][xx + 3].field == empty || (desk[yy - 1][xx + 3].field == occup && desk[yy - 1][xx + 3].checker_side == turn
						&& desk[yy - 2][xx + 4].field == occup && desk[yy - 2][xx + 4].checker_side == turn))){
			(*y_to) = yy - 1;
			(*x_to) = xx + 1;
			return TRUE;
		}
	}

	return FALSE;
}

int MayBeEaten(int yy, int xx){ //IMP, DR, DL, UR, UL
	int i = 0, j = 0;
	if(yy - 1 >= 0 && xx - 1 >= 0 && yy + 1 < DESK_SIZE && xx + 1 < DESK_SIZE){
		if(desk[yy + 1][xx + 1].field == empty){
			if(desk[yy - 1][xx - 1].field == occup && desk[yy - 1][xx - 1].checker_side != turn)
				return TRUE;
			for(i = yy - 1, j = xx - 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i--, j--){
				if(desk[i][j].field == occup && desk[i][j].checker_side == turn)
					break;
				if(desk[i][j].field == occup && desk[i][j].checker_side != turn && desk[i][j].role == queen)
					return UR;
			}
		}
		if(desk[yy + 1][xx - 1].field == empty){
			if(desk[yy - 1][xx + 1].field == occup && desk[yy - 1][xx + 1].checker_side != turn)
				return TRUE;
			for(i = yy - 1, j = xx + 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i--, j++){
				if(desk[i][j].field == occup && desk[i][j].checker_side == turn)
					break;
				if(desk[i][j].field == occup && desk[i][j].checker_side != turn && desk[i][j].role == queen)
					return UL;
			}
		}
		if(desk[yy - 1][xx + 1].field == empty){
			if(desk[yy + 1][xx - 1].field == occup && desk[yy + 1][xx - 1].checker_side != turn)
				return TRUE;
			for(i = yy + 1, j = xx - 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i++, j--){
				if(desk[i][j].field == occup && desk[i][j].checker_side == turn)
					break;
				if(desk[i][j].field == occup && desk[i][j].checker_side != turn && desk[i][j].role == queen)
					return DR;
			}
		}
		if(desk[yy - 1][xx - 1].field == empty){
			if(desk[yy + 1][xx + 1].field == occup && desk[yy + 1][xx + 1].checker_side != turn)
				return TRUE;
			for(i = yy + 1, j = xx + 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i++, j++){
				if(desk[i][j].field == occup && desk[i][j].checker_side == turn)
					break;
				if(desk[i][j].field == occup && desk[i][j].checker_side != turn && desk[i][j].role == queen)
					return DL;
			}
		}
	}
	return IMP;
}

int ConquerorBorders(int* y_to, int* x_to){
	int yy = 0;
	int COMRADE1 = 0, COMRADE2 = 0, COMREXIST = FALSE;
	struct MaxProfit{
		int From_x;
		int From_y;
		int To_x;
		int To_y;
	}Max;
	Max.To_x = -1;
	Max.To_y = 100;
	for(yy = DESK_SIZE - 1; yy >= 0; --yy){
		if((space_pressed == false) && (desk[yy][1].field == occup) &&
				desk[yy][1].checker_side == turn && yy - 1 >= 0 && desk[yy - 1][0].field == empty){
			if(yy - 1 < Max.To_y){
				Max.From_x = 1;
				Max.From_y = yy;
				Max.To_x = 0;
				Max.To_y = yy - 1;
			}
		}
		if((space_pressed == false) && (desk[yy][DESK_SIZE - 2].field == occup) &&
				desk[yy][DESK_SIZE - 2].checker_side == turn && yy - 1 >= 0 && desk[yy - 1][7].field == empty){
			if(yy - 1 < Max.To_y){
				Max.From_x = 6;
				Max.From_y = yy;
				Max.To_x = 7;
				Max.To_y = yy - 1;
			}
		}
	}
	if(Max.To_x != -1){
		//
		if(Max.To_x == 0 && desk[Max.From_y - 1][Max.From_x + 1].field == occup &&
				desk[Max.From_y - 1][Max.From_x + 1].checker_side == turn){
			COMREXIST = TRUE;
			COMRADE1 = MayBeEaten(Max.From_y - 1, Max.From_x + 1);
			desk[Max.From_y][Max.From_x].field = empty;
			COMRADE2 = MayBeEaten(Max.From_y - 1, Max.From_x + 1);
			desk[Max.From_y][Max.From_x].field = occup;
		}
		if(Max.To_x == 7 && desk[Max.From_y - 1][Max.From_x - 1].field == occup &&
				desk[Max.From_y - 1][Max.From_x - 1].checker_side == turn){
			COMREXIST = TRUE;
			COMRADE1 = MayBeEaten(Max.From_y - 1, Max.From_x - 1);
			desk[Max.From_y][Max.From_x].field = empty;
			COMRADE2 = MayBeEaten(Max.From_y - 1, Max.From_x - 1);
			desk[Max.From_y][Max.From_x].field = occup;
		}
		//
		if(COMREXIST && (COMRADE1 != COMRADE2)){
			return FALSE;
		}
		cur_cursor_y = Max.From_y;
		cur_cursor_x = Max.From_x;
		(*y_to) = Max.To_y;
		(*x_to) = Max.To_x;
		return TRUE;
	}
	return FALSE;
}

int TryToFixLoose(int yy, int xx, enum Moves warn, int* y_to, int* x_to){
	if(warn == DL || warn == DR){
		if(yy - 2 >= 0 && desk[yy - 2][xx].field == empty && !(xx - 2 >= 0 && desk[yy - 2][xx - 2].field == occup && desk[yy - 2][xx - 2].checker_side != turn)){
			if(desk[yy - 1][xx - 1].field == empty){
				cur_cursor_y = yy;
				cur_cursor_x = xx;
				(*y_to) = yy - 1;
				(*x_to) = xx - 1;
				return TRUE;
			}
		}
		if(yy - 2 >= 0 && desk[yy - 2][xx].field == empty && !(xx + 2 >= 0 && desk[yy - 2][xx + 2].field == occup && desk[yy - 2][xx + 2].checker_side != turn)){
			if(desk[yy - 1][xx + 1].field == empty){
				cur_cursor_y = yy;
				cur_cursor_x = xx;
				(*y_to) = yy - 1;
				(*x_to) = xx + 1;
				return TRUE;
			}
		}
	}else if(warn == UR){
		if(yy + 2 < DESK_SIZE && desk[yy + 2][xx].field == occup && desk[yy + 2][xx].checker_side == turn){
			cur_cursor_y = yy + 2;
			cur_cursor_x = xx;
			(*y_to) = yy + 1;
			(*x_to) = xx + 1;
			return TRUE;
		}
		if(xx + 2 < DESK_SIZE && yy + 2 < DESK_SIZE && desk[yy + 2][xx + 2].field == occup && desk[yy + 2][xx + 2].checker_side == turn){
			cur_cursor_y = yy + 2;
			cur_cursor_x = xx + 2;
			(*y_to) = yy + 1;
			(*x_to) = xx + 1;
			return TRUE;
		}
	}else if(warn == UL){
		if(yy + 2 < DESK_SIZE && desk[yy + 2][xx].field == occup && desk[yy + 2][xx].checker_side == turn){
			cur_cursor_y = yy + 2;
			cur_cursor_x = xx;
			(*y_to) = yy + 1;
			(*x_to) = xx - 1;
			return TRUE;
		}
		if(yy + 2 < DESK_SIZE && xx - 2 >= 0 && desk[yy + 2][xx - 2].field == occup && desk[yy + 2][xx - 2].checker_side == turn){
			cur_cursor_y = yy + 2;
			cur_cursor_x = xx - 2;
			(*y_to) = yy + 1;
			(*x_to) = xx - 1;
			return TRUE;
		}
	}
	return FALSE;
}

int NoLoses(int* y_to, int* x_to){
	// Поиск хода без потерь/Защиты
	enum Moves warning = -1;
	int CONTINUE = FALSE;
	int tmpX = 0, tmpY = 0;

	for(cur_cursor_y = DESK_SIZE - 1; cur_cursor_y >= 0; --cur_cursor_y){
		for(cur_cursor_x = DESK_SIZE - 1; cur_cursor_x >= 0; --cur_cursor_x){
			if((space_pressed == false) && (desk[cur_cursor_y][cur_cursor_x].field == occup) && desk[cur_cursor_y][cur_cursor_x].checker_side == turn){
				warning = MayBeEaten(cur_cursor_y, cur_cursor_x);
				if(warning != IMP){
					if(TryToFixLoose(cur_cursor_y, cur_cursor_x, warning, y_to, x_to))
						return TRUE;
				}
			}
		}
	}
	// Ход без потерь// Пока soldiers only
	int COMRADE1 = 0, COMRADE2 = 0, COMREXIST = FALSE;
	for(cur_cursor_y = 0; cur_cursor_y < DESK_SIZE; ++cur_cursor_y){
		for(cur_cursor_x = 0; cur_cursor_x < DESK_SIZE; ++cur_cursor_x){

			if((space_pressed == false) && (desk[cur_cursor_y][cur_cursor_x].field == occup) &&
					desk[cur_cursor_y][cur_cursor_x].checker_side == turn){
				if(desk[cur_cursor_y][cur_cursor_x].role == soldier && cur_cursor_y - 1 >= 0){
					if(cur_cursor_x - 1 >= 0 && desk[cur_cursor_y - 1][cur_cursor_x - 1].field == empty){
						tmpX = cur_cursor_x;
						tmpY = cur_cursor_y;
						if(cur_cursor_x + 1 < DESK_SIZE && desk[tmpY - 1][tmpX + 1].field == occup &&
								desk[tmpY - 1][tmpX + 1].checker_side == turn)
							COMREXIST = TRUE;
						if(COMREXIST)
							COMRADE1 = MayBeEaten(cur_cursor_y - 1, cur_cursor_x + 1);
						desk[tmpY][tmpX].field = empty;
						if(COMREXIST)
							COMRADE2 = MayBeEaten(cur_cursor_y - 1, cur_cursor_x + 1);
						if(!MayBeEaten(cur_cursor_y - 1, cur_cursor_x - 1) && COMRADE1 == COMRADE2){
							(*y_to) = cur_cursor_y - 1;
							(*x_to) = cur_cursor_x - 1;
							return TRUE;
						}
						desk[tmpY][tmpX].field = occup;
						COMREXIST = FALSE;
					}
					if(cur_cursor_x + 1 < DESK_SIZE && desk[cur_cursor_y - 1][cur_cursor_x + 1].field == empty){
						tmpX = cur_cursor_x;
						tmpY = cur_cursor_y;
						if(cur_cursor_x - 1 >= 0 && desk[tmpY - 1][tmpX - 1].field == occup &&
								desk[tmpY - 1][tmpX - 1].checker_side == turn)
							COMREXIST = TRUE;
						if(COMREXIST)
							COMRADE1 = MayBeEaten(cur_cursor_y - 1, cur_cursor_x - 1);
						desk[tmpY][tmpX].field = empty;
						if(COMREXIST)
							COMRADE1 = MayBeEaten(cur_cursor_y - 1, cur_cursor_x - 1);
						if(!MayBeEaten(cur_cursor_y - 1, cur_cursor_x + 1) && COMRADE1 == COMRADE2){
							(*y_to) = cur_cursor_y - 1;
							(*x_to) = cur_cursor_x + 1;
							return TRUE;
						}
						desk[tmpY][tmpX].field = occup;
						COMREXIST = FALSE;
					}
				}
			}
		}
	}
	return FALSE;
}

int ProfitQueen(const int tmpY, const int tmpX, const int ii, const int jj, int* y_to, int* x_to){
	int i = 0, j = 0;
	int FLAG = FALSE, CONTINUE = TRUE;
	desk[tmpY][tmpX].field = empty; // Изначальная координата, i,j - куда двигаем
	if(!MayBeEaten(ii, jj)){
		// DL
		for(i = ii - 1, j = jj - 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0 && CONTINUE; i--, j--){
			if(desk[i][j].field == occup && desk[i][j].checker_side != turn && i - 1 >= 0 && j - 1 >= 0 && desk[i - 1][j - 1].field == empty){
				CONTINUE = FALSE;
				FLAG = TRUE;
				break;
			}
			if(desk[i][j].field == occup)
				break;
			if(i - 1 >= 0 && desk[i - 1][j].field == occup && desk[i - 1][j].checker_side != turn && i - 1 <= 5){
				CONTINUE = FALSE;
				FLAG = TRUE;
				break;
			}
		}
		// UL
		for(i = ii + 1, j = jj - 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0 && CONTINUE; i++, j--){
			if(desk[i][j].field == occup && desk[i][j].checker_side != turn && i + 1 < DESK_SIZE && j - 1 >= 0 && desk[i + 1][j - 1].field == empty){
				CONTINUE = FALSE;
				FLAG = TRUE;
				break;
			}
			if(desk[i][j].field == occup)
				break;
			if(i - 1 >= 0 && j - 1 >= 0 && desk[i - 1][j - 1].field == occup && desk[i - 1][j - 1].checker_side != turn && i - 1 <= 5){
				CONTINUE = FALSE;
				FLAG = TRUE;
				break;
			}
		}
		// UR
		for(i = ii + 1, j = jj + 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0 && CONTINUE; i++, j++){
			if(desk[i][j].field == occup && desk[i][j].checker_side != turn && i - 1 >= 0 && j - 1 >= 0 && desk[i + 1][j + 1].field == empty){
				CONTINUE = FALSE;
				FLAG = TRUE;
				break;
			}
			if(desk[i][j].field == occup)
				break;
			if(i - 1 >= 0 && j + 1 < DESK_SIZE && desk[i - 1][j + 1].field == occup && desk[i - 1][j + 1].checker_side != turn && i - 1 <= 5){
				CONTINUE = FALSE;
				FLAG = TRUE;
				break;
			}
		}
		// DR
		for(i = ii - 1, j = jj + 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0 && CONTINUE; i--, j++){
			if(desk[i][j].field == occup && desk[i][j].checker_side != turn && i - 1 >= 0 && j - 1 >= 0 && desk[i - 1][j + 1].field == empty){
				CONTINUE = FALSE;
				FLAG = TRUE;
				break;
			}
			if(desk[i][j].field == occup)
				break;
			if(i - 1 >= 0 && desk[i - 1][j].field == occup && desk[i - 1][j].checker_side != turn && i - 1 <= 5){
				CONTINUE = FALSE;
				FLAG = TRUE;
				break;
			}
		}
		if(FLAG){
			(*y_to) = ii;
			(*x_to) = jj;
			desk[tmpY][tmpX].field = occup;
			return TRUE;
		}
	}
	desk[tmpY][tmpX].field = occup;
	return FALSE;
}

int QueenAttack(int* y_to, int* x_to){
	int i = 0, j = 0;
	int tmpX = 0, tmpY = 0;
	for(cur_cursor_y = 0; cur_cursor_y < DESK_SIZE; ++cur_cursor_y){
		for(cur_cursor_x = 0; cur_cursor_x < DESK_SIZE; ++cur_cursor_x){
			if((space_pressed == false) && (desk[cur_cursor_y][cur_cursor_x].field == occup) &&
					desk[cur_cursor_y][cur_cursor_x].checker_side == turn && desk[cur_cursor_y][cur_cursor_x].role == queen){
				tmpX = cur_cursor_x;
				tmpY = cur_cursor_y;
				// UR
				for(i = tmpY + 1, j = tmpX + 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i++, j++){
					if(desk[i][j].field == occup)
						break;
					else if(ProfitQueen(tmpY, tmpX, i, j, y_to, x_to))
						return TRUE;
				}
				// UL
				for(i = tmpY + 1, j = tmpX - 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i++, j--){
					if(desk[i][j].field == occup)
						break;
					else if(ProfitQueen(tmpY, tmpX, i, j, y_to, x_to))
						return TRUE;
				}
				// DR
				for(i = tmpY - 1, j = tmpX - 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i--, j--){
					if(desk[i][j].field == occup)
						break;
					else if(ProfitQueen(tmpY, tmpX, i, j, y_to, x_to))
						return TRUE;
				}
				// DL
				for(i = tmpY - 1, j = tmpX + 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i--, j++){
					if(desk[i][j].field == occup)
						break;
					else if(ProfitQueen(tmpY, tmpX, i, j, y_to, x_to))
						return TRUE;
				}
			}
		}
	}
	return FALSE;
}

void AIProPlayerAdvanced(void){
	const int SOLDIER = 0, QUEEN = 1;
	Phase_ Phase = 0;
	int EatOrNot = NO_EAT, PossibilityToEat = 0, NumWhite = 0, NumBlack = 0, x_to = 0, y_to = 0;
	Pressed EatenOne = {0, 0}; // Координаты съеденной, если таковая будет

	CountCheckers(&NumWhite, &NumBlack);
	Phase = PhaseFoo(NumWhite, NumBlack);
	PossibilityToEat = PossibleToEat();

	if(!PossibilityToEat){
		EatOrNot = NO_EAT;
		if(Phase == Debut){
			// Поиск размена (может быть ходом навстречу)
			for(cur_cursor_y = DESK_SIZE - 1; cur_cursor_y >= 0; --cur_cursor_y){
				for(cur_cursor_x = DESK_SIZE - 1; cur_cursor_x >= 0; --cur_cursor_x){
					if((space_pressed == false) && (desk[cur_cursor_y][cur_cursor_x].field == occup) && desk[cur_cursor_y][cur_cursor_x].checker_side == turn){
						if(ExChange(cur_cursor_y, cur_cursor_x, &y_to, &x_to)){
							// make move
							PressedToMove();
							AIMoveChecker(y_to, x_to, EatOrNot, EatenOne);
							Display();
							return;
						}
					}
				}
			}
			// Захват краевых позиций как можно дальше
			if(ConquerorBorders(&y_to, &x_to)){
				// make move
				PressedToMove();
				AIMoveChecker(y_to, x_to, EatOrNot, EatenOne);
				Display();
				return;
			}
			// Ход без потерь
			if(NoLoses(&y_to, &x_to)){
				// make move
				PressedToMove();
				AIMoveChecker(y_to, x_to, EatOrNot, EatenOne);
				Display();
				return;
			}
			// Ничего не сделать, деградация до простого бота
			AIProPlayer();
			return;
		}else if(Phase == MittelSpiel){
			if(ConquerorBorders(&y_to, &x_to)){
				PressedToMove();
				AIMoveChecker(y_to, x_to, EatOrNot, EatenOne);
				if(Move_to.press_y == 0 && desk[Move_from.press_y][Move_from.press_x].role == soldier)
					MakeQueen();
				Display();
				return;
			}
			if(NoLoses(&y_to, &x_to)){
				PressedToMove();
				AIMoveChecker(y_to, x_to, EatOrNot, EatenOne);
				if(Move_to.press_y == 0 && desk[Move_from.press_y][Move_from.press_x].role == soldier)
					MakeQueen();
				Display();
				return;
			}
			for(cur_cursor_y = DESK_SIZE - 1; cur_cursor_y >= 0; --cur_cursor_y){
				for(cur_cursor_x = DESK_SIZE - 1; cur_cursor_x >= 0; --cur_cursor_x){
					if((space_pressed == false) && (desk[cur_cursor_y][cur_cursor_x].field == occup) && desk[cur_cursor_y][cur_cursor_x].checker_side == turn){
						if(ExChange(cur_cursor_y, cur_cursor_x, &y_to, &x_to)){
							// make move
							PressedToMove();
							AIMoveChecker(y_to, x_to, EatOrNot, EatenOne);
							if(Move_to.press_y == 0 && desk[Move_from.press_y][Move_from.press_x].role == soldier)
								MakeQueen();
							Display();
							return;
						}
					}
				}
			}
			AIProPlayer();
			return;
		}else if(Phase == EndSpiel){
			if(rand() % 2 && QueenAttack(&y_to, &x_to)){
				PressedToMove();
				AIMoveChecker(y_to, x_to, EatOrNot, EatenOne);
				Display();
				return;
			}
			if(NoLoses(&y_to, &x_to)){
				PressedToMove();
				AIMoveChecker(y_to, x_to, EatOrNot, EatenOne);
				if(Move_to.press_y == 0 && desk[Move_from.press_y][Move_from.press_x].role == soldier)
					MakeQueen();
				Display();
				return;
			}
			AIProPlayer();
			return;
		}
	}else{
		// Надо кушать // пока старое
		for(cur_cursor_y = DESK_SIZE - 1; cur_cursor_y >= 0; --cur_cursor_y){
			for(cur_cursor_x = DESK_SIZE - 1; cur_cursor_x >= 0; --cur_cursor_x){
				if((space_pressed == false) && (desk[cur_cursor_y][cur_cursor_x].field == occup) && desk[cur_cursor_y][cur_cursor_x].checker_side == turn){//только начало хода
					if(desk[cur_cursor_y][cur_cursor_x].role == soldier){
						int direction = -1;
						if((direction = AIPossibilityToEatDown(cur_cursor_y, cur_cursor_x))){
							PressedToMove();
							EatOrNot = EAT;
							if(direction == DR){
								EatenOne.press_x = cur_cursor_x + 1;
								EatenOne.press_y = cur_cursor_y - 1;
								AIMoveChecker(cur_cursor_y - 2, cur_cursor_x + 2, EatOrNot, EatenOne);
								if(Move_to.press_y == 0)
									MakeQueen();
								Display();
								EatAsMuchAsPossible(cur_cursor_y - 2, cur_cursor_x + 2, soldier);
							}
							if(direction == DL){
								EatenOne.press_x = cur_cursor_x - 1;
								EatenOne.press_y = cur_cursor_y - 1;
								AIMoveChecker(cur_cursor_y - 2, cur_cursor_x - 2, EatOrNot, EatenOne);
								if(Move_to.press_y == 0)
									MakeQueen();
								Display();
								EatAsMuchAsPossible(cur_cursor_y - 2, cur_cursor_x - 2, soldier);
							}
							return;
						}else if((direction = AIPossibilityToEatUp(cur_cursor_y, cur_cursor_x))){
							PressedToMove();
							EatOrNot = EAT;
							if(direction == UR){
								EatenOne.press_x = cur_cursor_x + 1;
								EatenOne.press_y = cur_cursor_y + 1;
								AIMoveChecker(cur_cursor_y + 2, cur_cursor_x + 2, EatOrNot, EatenOne);
								Display();
								EatAsMuchAsPossible(cur_cursor_y + 2, cur_cursor_x + 2, soldier);
							}
							if(direction == UL){
								EatenOne.press_x = cur_cursor_x - 1;
								EatenOne.press_y = cur_cursor_y + 1;
								AIMoveChecker(cur_cursor_y + 2, cur_cursor_x - 2, EatOrNot, EatenOne);
								Display();
								EatAsMuchAsPossible(cur_cursor_y + 2, cur_cursor_x - 2, soldier);
							}
							return;
						}
					}else if(desk[cur_cursor_y][cur_cursor_x].role == queen){
						int x = -100, y = -100, direction = -1;
						if(AIPossibilityToEatByQueen(cur_cursor_y,
													 cur_cursor_x,
													 &y,
													 &x,
													 &direction)){
							EatOrNot = EAT;
							PressedToMove();
							int EnemyOnWay = 0;
							//если мы можем сходить так, чтобы потом иметь возможность срубить, то в циклах for ищется эта позиция
							if(direction == UL){
								EatenOne.press_x = x + 1;
								EatenOne.press_y = y - 1;
								EnemyOnWay = 0;
								for(int i = y + 1, j = x - 1; i < DESK_SIZE && j >= 0; i++, j--){//UL
									if(desk[i][j].field == empty){
										int X = -100, Y = -100;
										direction = -1;
										if(AIPossibilityToEatByQueen(i, j, &Y, &X, &direction)){
											AIMoveChecker(i, j, EatOrNot, EatenOne);
											Display();
											EatAsMuchAsPossible(i, j, queen);
											return;
										}
									}else{
										break;
									}
								}
							}
							if(direction == UR){
								EatenOne.press_x = x - 1;
								EatenOne.press_y = y - 1;
								EnemyOnWay = 0;
								for(int i = y + 1, j = x + 1; i < DESK_SIZE && j < DESK_SIZE; i++, j++){//UR
									if(desk[i][j].field == empty){
										int X = -100, Y = -100;
										direction = -1;
										if(AIPossibilityToEatByQueen(i, j, &Y, &X, &direction)){
											AIMoveChecker(i, j, EatOrNot, EatenOne);
											Display();
											EatAsMuchAsPossible(i, j, queen);
											return;
										}
									}else{
										break;
									}
								}
							}
							if(direction == DL){
								EatenOne.press_x = x + 1;
								EatenOne.press_y = y + 1;
								EnemyOnWay = 0;
								for(int i = y - 1, j = x - 1; i >= 0 && j >= 0; i--, j--){//DL
									if(desk[i][j].field == empty){
										int X = -100, Y = -100;
										direction = -1;
										if(AIPossibilityToEatByQueen(i, j, &Y, &X, &direction)){
											AIMoveChecker(i, j, EatOrNot, EatenOne);
											Display();
											EatAsMuchAsPossible(i, j, queen);
											return;
										}
									}else{
										break;
									}
								}
							}
							if(direction == DR){
								EatenOne.press_x = x - 1;
								EatenOne.press_y = y + 1;
								EnemyOnWay = 0;
								for(int i = y - 1, j = x + 1; j < DESK_SIZE && i >= 0; i--, j++){//DR
									if(desk[i][j].field == empty){
										int X = -100, Y = -100;
										direction = -1;
										if(AIPossibilityToEatByQueen(i, j, &Y, &X, &direction)){
											AIMoveChecker(i, j, EatOrNot, EatenOne);
											Display();
											EatAsMuchAsPossible(i, j, queen);
											return;
										}
									}else{
										break;
									}
								}
							}
							AIMoveChecker(y, x, EatOrNot, EatenOne);
							Display();
							EatAsMuchAsPossible(y, x, queen);
							return;
						}
					}
				}
			}
		}
	}
}

void Keyboard(unsigned char key, int x, int y){
	static int NumOfMoves = 0;
	const char ESCAPE = '\033'; // Символ ESCAPE

	if(key == ESCAPE){
		freeList();
		exit(EXIT_SUCCESS);
	}
	if(Status == AFTER_GAME){
		int i = 0, NameLen = strlen(Name);
		char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
		char nums[] = "0123456789";

		if(key == 13){// Enter
			Status = GAME;
			//.Запись
			if(head == NULL)
				CreateList();
			AddToList(Name, NumOfMoves);
			RecordListToFile();
			// Обнуление
			for(i = 0; i < NAME_LEN; i++)
				Name[i] = '\0';
			NumOfMoves = 0;
			return;
		}
		if(key == 0x08){// backspace
			if(NameLen){
				Name[NameLen - 1] = '\0';
			}
		}
		for(i = 0; i < strlen(alphabet); i++){
			if(alphabet[i] == key || Alphabet[i] == key){
				if(NameLen < NAME_LEN){
					Name[NameLen] = Alphabet[i];
					return;
				}
			}
		}
		for(i = 0; i < strlen(nums); i++){
			if(nums[i] == key){
				if(NameLen < NAME_LEN){
					Name[NameLen] = nums[i];
					return;
				}
			}
		}
	}
	// Enter
	if(Status == GAME){
		if(key == 13 && WasMoved == true){
			if(Level == VSPEOPLE)
				Enter();
			else if(Level == LOWAI || Level == MIDAI){
				int QUIT = 0;
				if(turn == white)
					NumOfMoves++;
				space_pressed = false;
				// Необходимо для того, чтобы снять подсвеченность
				for(int i = 0; i < DESK_SIZE; i++){
					for(int j = 0; j < DESK_SIZE; j++){
						if(desk[i][j].focused.toMove == true){
							desk[i][j].focused.toMove = false;
							QUIT = 1;
							break;
						}
					}
					if(QUIT)
						break;
				}

				Display();
#ifdef WINDOWS
				Sleep(700);
#endif

#ifdef LINUX
				sleep(0.7);
#endif
				turn = black;
				WasMoved = false;

				if(CheckEndNoMove()){
					if(turn == black){
						Status = AFTER_GAME;
						return;
					}else{
						Status = AFTER_GAME_LOOSE;
						return;
					}
				}

				//тут вот должен вызываться компьютер
				int tmp_cur_x = cur_cursor_x, tmp_cur_y = cur_cursor_y;
				if(Level == LOWAI)
					AIProPlayer();
				else if(Level == MIDAI)
					AIProPlayerAdvanced();
				cur_cursor_x = tmp_cur_x;
				cur_cursor_y = tmp_cur_y;
				WasMoved = false;
				turn = white;

				if(CheckEndNoMove()){
					if(turn == black){
						Status = AFTER_GAME;
						return;
					}else{
						Status = AFTER_GAME_LOOSE;
						return;
					}
				}
			}
			return;
		}
		if((cursor == true) && (key == ' ')){
			if((space_pressed == false) && (desk[cur_cursor_y][cur_cursor_x].field == occup) && desk[cur_cursor_y][cur_cursor_x].checker_side == turn){
				PressedToMove();
			}else if(space_pressed == true){
				if((Move_from.press_x == cur_cursor_x) && (Move_from.press_y == cur_cursor_y)) // Второе нажатие на поле снимает фокус
				{
					desk[Move_from.press_y][Move_from.press_x].focused.toMove = false;
					space_pressed = false;
				}else{
					// Проверка, возможно ли / если нет - убираем начальную подсвеченность
					int EatOrNot = NO_EAT, PossibilityToEat = 0; // Было ли съедение или нет
					Pressed EatenOne = {0, 0}; // Координаты съеденной, если таковая будет

					PossibilityToEat = PossibleToEat();

					if(Possible(cur_cursor_x,
								cur_cursor_y,
								&EatOrNot,
								&EatenOne) && ((EatOrNot == EAT && PossibilityToEat) || \
                            (EatOrNot == NO_EAT && !PossibilityToEat))){
						MoveChecker(EatOrNot, EatenOne);
						// Конец игры
						if(EatOrNot == EAT){
							if(CheckEndEmpty()){
								if(turn == white){
									Status = AFTER_GAME;
									return;
								}else{
									Status = AFTER_GAME_LOOSE;
									return;
								}
							}
						}

						if(turn == white && Move_to.press_y == DESK_SIZE - 1)
							MakeQueen();
						else if(turn == black && Move_to.press_y == 0)
							MakeQueen();

						if(EndOfMove(EatOrNot)){
							if(Level == VSPEOPLE){
								if(turn == white)
									NumOfMoves++;

								if(turn == white)
									turn = black;
								else
									turn = white;
								WasMoved = false;

								if(CheckEndNoMove()){
									if(turn == black){
										Status = AFTER_GAME;
										return;
									}else{
										Status = AFTER_GAME_LOOSE;
										return;
									}
								}
							}else{
								if(turn == white)
									NumOfMoves++;

								Display();
#ifdef WINDOWS
								Sleep(700);
#endif

#ifdef LINUX
								sleep(0.7);
#endif
								turn = black;
								WasMoved = false;

								if(CheckEndNoMove()){
									if(turn == black){
										Status = AFTER_GAME;
										return;
									}else{
										Status = AFTER_GAME_LOOSE;
										return;
									}
								}

								//тут вот должен вызываться компьютер
								int tmp_cur_x = cur_cursor_x, tmp_cur_y = cur_cursor_y;
								if(Level == LOWAI)
									AIProPlayer();
								else if(Level == MIDAI)
									AIProPlayerAdvanced();
								cur_cursor_x = tmp_cur_x;
								cur_cursor_y = tmp_cur_y;
								WasMoved = false;
								turn = white;

								if(CheckEndNoMove()){
									if(turn == black){
										Status = AFTER_GAME;
										return;
									}else{
										Status = AFTER_GAME_LOOSE;
										return;
									}
								}
							}
						}else
							PressedToMove();
					}
				}
			}
		}

	}
}

void SpecialKeyboard(int key, int x, int y){
	if(key == GLUT_KEY_F1){
		if(Status == GAME){
			Restart_button.Color = 0;
			Save_Position_button.Color = 0;
			Recover_Position_button.Color = 0;
			Status = MENU;
		}else if(Status == MENU)
			Status = GAME;
		else if(Status == DEVELOPERS)
			Status = MENU;
		else if(Status == AFTER_GAME)
			Status = GAME;
		else if(Status == SRECORDS)
			Status = MENU;
		else if(Status == OPTIONS)
			Status = MENU;
	}

	if(Status == GAME){
		if(key == GLUT_KEY_LEFT){
			if(!firstBut()){
				if(cur_cursor_x != 0){
					desk[cur_cursor_y][cur_cursor_x].focused.highlighted = false;
					cur_cursor_x--;
					desk[cur_cursor_y][cur_cursor_x].focused.highlighted = true;
				}
			}
		}
		if(key == GLUT_KEY_RIGHT){
			if(!firstBut()){
				if(cur_cursor_x != DESK_SIZE - 1){
					desk[cur_cursor_y][cur_cursor_x].focused.highlighted = false;
					cur_cursor_x++;
					desk[cur_cursor_y][cur_cursor_x].focused.highlighted = true;
				}
			}
		}
		if(key == GLUT_KEY_UP){
			if(!firstBut()){
				if(cur_cursor_y != DESK_SIZE - 1){
					desk[cur_cursor_y][cur_cursor_x].focused.highlighted = false;
					cur_cursor_y++;
					desk[cur_cursor_y][cur_cursor_x].focused.highlighted = true;
				}
			}
		}
		if(key == GLUT_KEY_DOWN){
			if(!firstBut()){
				if(cur_cursor_y != 0){
					desk[cur_cursor_y][cur_cursor_x].focused.highlighted = false;
					cur_cursor_y--;
					desk[cur_cursor_y][cur_cursor_x].focused.highlighted = true;
				}
			}
		}
	}
}

void Mouse(int button, int state, int x, int y){
	int Y = Height - y; // При растяжении вверх расст. до кнопки растет на столько же
	// Координаты от левого верхнего угла
	if(Status == GAME){
		if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && x >= Restart_button.Left_border && x <= Restart_button.Right_border && \
            Y <= Restart_button.Up_border && Y >= Restart_button.Down_border){
			cur_cursor_x = 0;
			cur_cursor_y = 0;
			space_pressed = false;
			WasMoved = false;
			turn = white;
			clearDesk();
			setDesk();
			return;
		}
		if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && x >= Save_Position_button.Left_border && x <= Save_Position_button.Right_border && \
            Y <= Save_Position_button.Up_border && Y >= Save_Position_button.Down_border){
			int i = 0, j = 0;
			FILE* sav = NULL;
			sav = fopen(SAVES, "w");
			if(sav == NULL){
				perror("Error\n");
				return;
			}
			fprintf(sav, "%d %d\n", turn, WasMoved);
			for(i = 0; i < DESK_SIZE; i++){
				for(j = 0; j < DESK_SIZE; j++){
					if(desk[i][j].field == occup)
						fprintf(sav,
								"%d %d %d %d %d\n",
								desk[i][j].checker_side,
								desk[i][j].role,
								desk[i][j].focused.toMove,
								j,
								i);
				}
			}
			fclose(sav);
			return;
		}
		if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && x >= Recover_Position_button.Left_border && x <= Recover_Position_button.Right_border && \
            Y <= Recover_Position_button.Up_border && Y >= Recover_Position_button.Down_border){
			int i = 0, j = 0, Side = 0, Role = 0, Focus = 0, X = 0, YY = 0, Turn = 0, WASmoved = 0;
			FILE* sav = NULL;
			sav = fopen(SAVES, "r");
			if(sav == NULL){
				perror("Error\n");
				return;
			}

			cur_cursor_x = 0;
			cur_cursor_y = 0;
			space_pressed = false;
			WasMoved = false;
			turn = white;
			clearDesk();

			if(fscanf(sav, "%d%*c%d%*c", &Turn, &WASmoved) == EOF){
				fclose(sav);
				return;
			}
			turn = Turn;
			WasMoved = WASmoved;
			for(i = 0; i < DESK_SIZE; i++){
				for(j = 0; j < DESK_SIZE; j++){
					if(fscanf(sav, "%d%*c%d%*c%d%*c%d%*c%d", &Side, &Role, &Focus, &X, &YY) == EOF){
						fclose(sav);
						return;
					}
					desk[YY][X].field = occup;
					desk[YY][X].checker_side = Side;
					desk[YY][X].role = Role;
					desk[YY][X].focused.toMove = Focus;
				}
			}
			fclose(sav);
			return;
		}
	}else if(Status == MENU){
		if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && x >= Developers_button.Left_border && x <= Developers_button.Right_border && \
            Y <= Developers_button.Up_border && Y >= Developers_button.Down_border){
			Status = DEVELOPERS;
			Developers_button.Color = 0;
		}
		if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && x >= Records_button.Left_border && x <= Records_button.Right_border && \
            Y <= Records_button.Up_border && Y >= Records_button.Down_border){
			Status = SRECORDS;
			Records_button.Color = 0;
		}
		if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && x >= Options_button.Left_border && x <= Options_button.Right_border && \
            Y <= Options_button.Up_border && Y >= Options_button.Down_border){
			Status = OPTIONS;
			Options_button.Color = 0;
		}
	}else if(Status == OPTIONS){
		if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && x >= Easy_button.Left_border && x <= Easy_button.Right_border && \
            Y <= Easy_button.Up_border && Y >= Easy_button.Down_border){
			Level = LOWAI;
		}
		if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && x >= Medium_button.Left_border && x <= Medium_button.Right_border && \
            Y <= Medium_button.Up_border && Y >= Medium_button.Down_border){
			Level = MIDAI;
		}
	}
}

void Mouse_Passive(int x, int y){
	int Y = Height - y;
	if(Status == GAME){
		if(x >= Restart_button.Left_border && x <= Restart_button.Right_border && \
            Y <= Restart_button.Up_border && Y >= Restart_button.Down_border){
			Restart_button.Color = 1;
		}else
			Restart_button.Color = 0;

		if(x >= Save_Position_button.Left_border && x <= Save_Position_button.Right_border && \
            Y <= Save_Position_button.Up_border && Y >= Save_Position_button.Down_border){
			Save_Position_button.Color = 1;
		}else
			Save_Position_button.Color = 0;

		if(x >= Recover_Position_button.Left_border && x <= Recover_Position_button.Right_border && \
            Y <= Recover_Position_button.Up_border && Y >= Recover_Position_button.Down_border){
			Recover_Position_button.Color = 1;
		}else
			Recover_Position_button.Color = 0;
	}else if(Status == MENU){
		if(x >= Developers_button.Left_border && x <= Developers_button.Right_border && \
            Y <= Developers_button.Up_border && Y >= Developers_button.Down_border){
			Developers_button.Color = 1;
		}else
			Developers_button.Color = 0;

		if(x >= Records_button.Left_border && x <= Records_button.Right_border && \
            Y <= Records_button.Up_border && Y >= Records_button.Down_border){
			Records_button.Color = 1;
		}else
			Records_button.Color = 0;
		if(x >= Options_button.Left_border && x <= Options_button.Right_border && \
            Y <= Options_button.Up_border && Y >= Options_button.Down_border){
			Options_button.Color = 1;
		}else
			Options_button.Color = 0;
	}
}

int main(int argc, char** argv){
	srand(time(NULL));
	openRecords();
	setDesk(); // Начальное заполнение массива структур
	setButtons();

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB);
	glutInitWindowSize(Width, Height);
	glutCreateWindow("Checkers.exe");

	glutDisplayFunc(Display);
	glutIdleFunc(Display);

	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeyboard);
	glutMouseFunc(Mouse);
	glutPassiveMotionFunc(Mouse_Passive);
	glutMainLoop();

	freeList();

	return EXIT_SUCCESS;
}