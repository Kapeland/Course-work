#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <Windows.h>
#include <math.h>

#define DESK_SIZE 8
#define NO_EAT -1
#define EAT -2

// Начальная широта и высота окна
GLint Width = 900, Height = 900;
const int CubeSize = 100; // Размер квадрата

enum levels{
	first = 1,
	second,
	fird
};
enum place{
	None,
	UpLeft, UpRight,
	DownLeft, DownRight
};

typedef enum{
	black, white, red
}colors;
typedef enum{
	false, true
}logik;

colors turn = white;
logik cursor = false;
logik space_pressed = false; // Нажат ли пробел
logik WasMoved = false; //
int cur_cursor_x = 0, cur_cursor_y = 0;

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

Desk desk[DESK_SIZE][DESK_SIZE];
Pressed Move_from, Move_to;

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

void Display(void){
	int add_x = 0, i = 0, k = 0, add_y = 0;
	colors color = 0;
	// Устанавливаем цвет фона
	glClearColor(0.7, 1, 0.7, 1);
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
	glFinish();
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
	const int SUCCESS = 1, FAILURE = 0, WAIT = 2;
	colors side_of_eaten;
	int TOTAL = FAILURE;

	if(desk[Move_from.press_y][Move_from.press_x].checker_side == white)
		side_of_eaten = black;
	else
		side_of_eaten = white;

	if(desk[yy][xx].field_col == white || desk[yy][xx].field == occup)
		return FAILURE;

	if(desk[Move_from.press_y][Move_from.press_x].role == soldier)//desk[yy][xx]
	{
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
		if(Move_from.press_x > xx && Move_from.press_y > yy){
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

void MoveChecker(int EatOrNot, Pressed EatenOne){
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

void AIMoveChecker(int y, int x, int yy, int xx){
	//х,у - откуда. хх,уу - куда
	desk[y][x].field = empty;
	desk[yy][xx].field = occup;
	desk[yy][xx].checker_side = desk[y][x].checker_side;
	desk[yy][xx].role = desk[y][x].role;


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
	if(xx + 2 < DESK_SIZE && yy + 2 < DESK_SIZE){
		if((desk[yy + 2][xx + 2].field == empty) && \
            desk[yy + 1][xx + 1].field == occup && \
            desk[yy + 1][xx + 1].checker_side != turn)
			return UpRight;
	}
	if(xx - 2 >= 0 && yy + 2 < DESK_SIZE){
		if((desk[yy + 2][xx - 2].field == empty) && \
            desk[yy + 1][xx - 1].field == occup && \
            desk[yy + 1][xx - 1].checker_side != turn)
			return UpLeft;
	}
	return None;
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
	if(xx + 2 < DESK_SIZE && yy - 2 >= 0){
		if((desk[yy - 2][xx + 2].field == empty) && \
            desk[yy - 1][xx + 1].field == occup && \
            desk[yy - 1][xx + 1].checker_side != turn)
			return DownRight;
	}
	if(xx - 2 >= 0 && yy - 2 >= 0){
		if((desk[yy - 2][xx - 2].field == empty) && \
            desk[yy - 1][xx - 1].field == occup && \
            desk[yy - 1][xx - 1].checker_side != turn)
			return DownLeft;
	}
	return None;
}

int PossibilityToEatByQueen(const int yy, const int xx){
	const int POSSIBLE = 1, NOT_POSSIBLE = 0;
	int i = 0, j = 0;

	for(i = yy + 1, j = xx - 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i++, j--){
		if(desk[i][j].field == occup){
			if(desk[i][j].checker_side == turn)
				break;
			else if((j - 1) >= 0 && (i + 1) < DESK_SIZE && desk[i + 1][j - 1].field == empty)
				return POSSIBLE;
		}
	}

	for(i = yy + 1, j = xx + 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i++, j++){
		if(desk[i][j].field == occup){
			if(desk[i][j].checker_side == turn)
				break;
			else if((j + 1) < DESK_SIZE && (i + 1) < DESK_SIZE && desk[i + 1][j + 1].field == empty)
				return POSSIBLE;
		}
	}

	for(i = yy - 1, j = xx - 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i--, j--){
		if(desk[i][j].field == occup){
			if(desk[i][j].checker_side == turn)
				break;
			else if((j - 1) >= 0 && (i - 1) >= 0 && desk[i - 1][j - 1].field == empty)
				return POSSIBLE;
		}
	}

	for(i = yy - 1, j = xx + 1; i < DESK_SIZE && j < DESK_SIZE && i >= 0 && j >= 0; i--, j++){
		if(desk[i][j].field == occup){
			if(desk[i][j].checker_side == turn)
				break;
			else if((j + 1) < DESK_SIZE && (i - 1) >= 0 && desk[i - 1][j + 1].field == empty)
				return POSSIBLE;
		}
	}

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

void Keyboard(unsigned char key, int x, int y){
	const char ESCAPE = '\033'; // Символ ESCAPE

	if(key == ESCAPE)
		exit(EXIT_SUCCESS);
	// Enter
	if(key == 13 && WasMoved == true)//&& desk[Move_from.press_y][Move_from.press_x].focused.toMove == false // Условие, чтобы не срабатывал энтер раньше времени
	{
		desk[Move_to.press_y][Move_to.press_x].focused.toMove = false;
		if(turn == white)
			turn = black;
		else
			turn = white;
		WasMoved = false;
		space_pressed = false;

		return;
	}

	if((cursor == true) && (key == ' ')) // && (desk[cur_cursor_y][cur_cursor_x].field == occup)
	{
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
							&EatenOne) && ((EatOrNot == EAT && PossibilityToEat) || (EatOrNot == NO_EAT && !PossibilityToEat))) // Сязать через указатель possible и end_of_move
				{
					MoveChecker(EatOrNot, EatenOne);
					/*if (EatOrNot == EAT)
					{
						if (CheckEndEmpty())
						// Относится к меню
					}	*/

					if(turn == white && Move_to.press_y == DESK_SIZE - 1)
						MakeQueen();
					else if(turn == black && Move_to.press_y == 0)
						MakeQueen();

					if(EndOfMove(EatOrNot)){
						if(turn == white)
							turn = black;
						else
							turn = white;
						WasMoved = false;
					}else
						PressedToMove();
				}
			}
		}
	}
}

void SpecialKeyboard(int key, int x, int y){
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

void AIEat(int x, int y, int place, int role){
	if(role == soldier){
		if(place == UpRight){
			AIMoveChecker(y, x, y + 2, x + 2);
			desk[y + 1][x + 1].field = empty;
		}
		if(place == UpLeft){
			AIMoveChecker(y, x, y + 2, x - 2);
			desk[y + 1][x - 1].field = empty;
		}
		if(place == DownLeft){
			AIMoveChecker(y, x, y - 2, x - 2);
			desk[y - 1][x - 1].field = empty;
		}
		if(place == DownRight){
			AIMoveChecker(y, x, y - 2, x - 2);
			desk[y - 1][x + 1].field = empty;
		}
	}

}

void AIRecCall(int y, int x, int role){
	//TODO подумать, будет ли он правильно ходить назад, если нужно бить
	//TODO выяснить, можно ли ходить в ту же клетку, где была шашка
	if(role == soldier){
		int xx = 0, yy = 0; //x,y - from. xx, yy - to.
		while(xx < DESK_SIZE && yy < DESK_SIZE){
			if(xx < DESK_SIZE && yy < DESK_SIZE){
				/*if(Possible(xx, yy)){
					AIMoveChecker(y, x, yy, xx);
					WasMoved = true;
					AIRecCall(yy, xx, desk[yy][xx].role);
				}*/
			}
			xx++;
			if(xx >= DESK_SIZE){
				xx = 0;
				yy++;
			}
		}
	}

}

void AIPlayer(int lvl){
	//TODO доделать ход королевы
	if(lvl == first){
		WasMoved = false;
		for(int i = 0; i < DESK_SIZE; ++i){
			for(int j = 0; j < DESK_SIZE; ++j){
				if(desk[i][j].role == soldier){
					if(AIPossibilityToEatUp(i, j) == None){
						continue;
					}else if(AIPossibilityToEatUp(i, j) == UpRight){
						AIMoveChecker(i, j, i + 2, j + 2);
						AIEat(i, j, UpRight, soldier);
					}else if(AIPossibilityToEatUp(i, j) == UpLeft){
						AIMoveChecker(i, j, i + 2, j - 2);
						AIEat(i, j, UpLeft, soldier);
					}else if(AIPossibilityToEatDown(i, j) == None){
						continue;
					}else if(AIPossibilityToEatDown(i, j) == DownRight){
						AIMoveChecker(i, j, i - 2, j + 2);
						AIEat(i, j, DownRight, soldier);
					}else if(AIPossibilityToEatDown(i, j) == DownLeft){
						AIMoveChecker(i, j, i - 2, j - 2);
						AIEat(i, j, DownLeft, soldier);
					}
				}

			}
		}
	}
	if(lvl == second){
		for(int i = 0; i < DESK_SIZE; ++i){
			for(int j = 0; j < DESK_SIZE; ++j){
				if(desk[i][j].role == soldier){
					WasMoved = false;
					AIRecCall(i, j, soldier);
					if(WasMoved == true){
						turn = white;
						return;
					}

				}

			}
		}
	}
}

int main(int argc, char** argv){
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	setDesk(); // Начальное заполнение массива структур

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB);
	glutInitWindowSize(Width, Height);
	glutCreateWindow("Checkers.exe");

	glutDisplayFunc(Display);
	glutIdleFunc(Display);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeyboard);

	glutMainLoop();

	return EXIT_SUCCESS;
}