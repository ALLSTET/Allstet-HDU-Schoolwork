#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <time.h>
#include <conio.h>
#include <stdbool.h>

#define MAPWIDTH 40
#define MAPHEIGHT 20

// 定义结构体食物
typedef struct {
    int x;
    int y;
} Food;

// 定义结构体蛇
typedef struct {
    int speed;
    int len;
    int x[100];
    int y[100];
} Snake;

void drawMap(Food *food, Snake *snake);
void keyDown(int *key, Snake *snake, int *changeFlag);
void createFood(Food *food, Snake *snake, int *changeFlag, int *score);
bool snakeStatus(Snake *snake);
void gotoxy(int x, int y);

int main() {
    SetConsoleOutputCP(CP_UTF8);
    int key = 72;
    int changeFlag = 0;
    int score = 0;
    srand((unsigned int)time(NULL));
    Food food;
    Snake snake;
    drawMap(&food, &snake);
    while (1) {
        keyDown(&key, &snake, &changeFlag);
        if (!snakeStatus(&snake)) {
            break;
        }
        createFood(&food, &snake, &changeFlag, &score);
        Sleep(snake.speed);
    }
    gotoxy(MAPWIDTH / 2, MAPHEIGHT / 2);
    gotoxy(MAPWIDTH / 2, MAPHEIGHT / 2 + 1);
    printf("\n|   Game Over! ");
    printf("Your final Score is:%d", score);
    Sleep(4500);

    return 0;
}

// 生成游戏场地
void drawMap(Food *food, Snake *snake) {
    int i;
    // 绘制上边界
    for (i = 0; i <= MAPWIDTH; i += 2) {
        gotoxy(i, 0);
        printf("-");
    }
    // 绘制下边界
    for (i = 0; i <= MAPWIDTH; i += 2) {
        gotoxy(i, MAPHEIGHT - 1);
        printf("-");
    }
    // 绘制左边界
    for (i = 1; i < MAPHEIGHT - 1; i++) {
        gotoxy(0, i);
        printf("|");
    }
    // 绘制右边界
    for (i = 1; i < MAPHEIGHT - 1; i++) {
        gotoxy(MAPWIDTH - 1, i);
        printf("|");
    }

    // 随机生成初始食物
    do {
        food->x = rand() % (MAPWIDTH - 4) + 2;
        food->y = rand() % (MAPHEIGHT - 2) + 1;
    } while (food->x % 2 != 0);
    gotoxy(food->x, food->y);
    printf("$");

    // 绘制蛇的头
    snake->x[0] = MAPWIDTH / 2;
    snake->y[0] = MAPHEIGHT / 2;
    gotoxy(snake->x[0], snake->y[0]);
    printf("■");

    // 初始化蛇的长度和速度
    snake->len = 3;
    snake->speed = 200;

    // 初始化蛇的位置
    for (i = 1; i < snake->len; i++) {
        snake->x[i] = snake->x[i - 1] + 2;
        snake->y[i] = snake->y[i - 1];
        gotoxy(snake->x[i], snake->y[i]);
        printf("■");
    }
    gotoxy(MAPWIDTH - 2, 0);
}

// 定义上下左右按键输入
void keyDown(int *key, Snake *snake, int *changeFlag) {
    int pre_key = *key;
    if (_kbhit()) {
        fflush(stdin);
        _getch();
        *key = _getch();
    }
    if (*changeFlag == 0) {
        gotoxy(snake->x[snake->len - 1], snake->y[snake->len - 1]);
        printf(" ");
    }
    for (int i = snake->len - 1; i > 0; i--) {
        snake->x[i] = snake->x[i - 1];
        snake->y[i] = snake->y[i - 1];
    }
    if (pre_key == 72 && *key == 80) {
        *key = 72;
    }
    if (pre_key == 80 && *key == 72) {
        *key = 80;
    }
    if (pre_key == 75 && *key == 77) {
        *key = 75;
    }
    if (pre_key == 77 && *key == 75) {
        *key = 77;
    }
    switch (*key) {
        case 75:
            snake->x[0] = (snake->x[0] - 2 + MAPWIDTH) % MAPWIDTH;
            break;
        case 77:
            snake->x[0] = (snake->x[0] + 2) % MAPWIDTH;
            break;
        case 72:
            snake->y[0] = (snake->y[0] - 1 + MAPHEIGHT) % MAPHEIGHT;
            break;
        case 80:
            snake->y[0] = (snake->y[0] + 1) % MAPHEIGHT;
            break;
    }
    gotoxy(snake->x[0], snake->y[0]);
    printf("■");
    *changeFlag = 0;
}

// 随机生成食物
void createFood(Food *food, Snake *snake, int *changeFlag, int *score) {
    if (snake->x[0] == food->x && snake->y[0] == food->y) {
        int flag;
        do {
            flag = 1;
            food->x = rand() % (MAPWIDTH - 4) + 2;
            food->y = rand() % (MAPHEIGHT - 2) + 1;
            for (int i = 0; i < snake->len; i++) {
                if (snake->x[i] == food->x && snake->y[i] == food->y) {
                    flag = 0;
                    break;
                }
            }
        } while (!flag || food->x % 2 != 0);
        gotoxy(food->x, food->y);
        printf("$");
        snake->len++;
        *score += 10;
        snake->speed -= 5;
        *changeFlag = 1;

        // 绘制新的蛇身体部分
        gotoxy(snake->x[snake->len - 1], snake->y[snake->len - 1]);
    } else {
        // 如果没有吃到食物，清除蛇尾
        if (*changeFlag == 0) {
            gotoxy(snake->x[snake->len - 1], snake->y[snake->len - 1]);
            printf(" ");
        }
    }
}

// 蛇的状态
bool snakeStatus(Snake *snake) {
    if (snake->y[0] == 0 || snake->y[0] == MAPHEIGHT - 1) {
        return false;
    }
    if (snake->x[0] == 0 || snake->x[0] == MAPWIDTH - 1) {
        return false;
    }
    for (int i = 1; i < snake->len; i++) {
        if (snake->x[i] == snake->x[0] && snake->y[i] == snake->y[0]) {
            return false;
        }
    }
    return true;
}

// 改为自定义gotoxy（）函数，确保代码不会因为编译器未正确包含<conio.h>内容而报错
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}