#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "string.h"
#include "math.h"
#include "conio.h"
#define size 4

void Initialize_GameBoard(void);
void Initialize_GameArray(int (*Game_Pointer)[size]);
void Change_GameBoard(const int Max_Num, const int (*Game_Pointer)[size]); 
char Arrow_Check(void);
void Sum_GameArray(const char Arrow, int (*Game_Pointer)[size]);
bool Blank_Check(const int (*Game_Pointer)[size]);
int Maxnum_Check(const int (*Game_Pointer)[size]);
void Add_GameArray(int Max_Num, int (*Game_Pointer)[size]);
int score = 0;


int main() {
  bool Flag = false; 
  int Game_Array[size][size], Max_Num = 1;
  char answer;
  memset(Game_Array, 0, sizeof(Game_Array));
  int (*Game_Pointer)[size];
  Game_Pointer = Game_Array;
  Initialize_GameBoard();
  printf("START the GAME?\nPls PRINT Y(yes) or N(no):");
  answer = getch();
  printf("\n");
  if (answer == 'Y' || answer == 'y') {
      Flag = true;
      Initialize_GameArray(Game_Pointer);
  }
  else if (answer == 'N' || answer == 'n') {
    system("cls");
    goto JUMP1;
  } 
  else {
    system("cls");
    printf("INVALID INPUT!");
    goto JUMP2;
  }
  do {
    Change_GameBoard(Max_Num, Game_Pointer);
    Sum_GameArray(Arrow_Check(), Game_Pointer);
    Max_Num = Maxnum_Check(Game_Pointer);
    score = Max_Num * 2;
    Change_GameBoard(Max_Num, Game_Pointer);
    if (Blank_Check(Game_Pointer)) {
      Add_GameArray(Max_Num, Game_Pointer);
      continue;
    } 
    else Flag = false;
    } while (Flag == true);
JUMP1:
  printf("GAME OVER!\nYour Final Score is:%d\n", score);
JUMP2:
  getchar();
  free(Game_Pointer);
  return 0;
}

void Initialize_GameBoard(void) {
  for (int i = 1; i <= 9; i++) {
    if (i % 2 != 0) {
      printf("*********************\n");
    } 
    else printf("|    |    |    |    |\n"); 
  }  
}

void Initialize_GameArray(int (*Game_Pointer)[size]) {
  srand((unsigned)time(NULL));
  int count = (rand() % 2) + 1;
  for (int i = 0; i < count; i++) {
    int m, n;
    do {
      m = rand() % size;
      n = rand() % size;
    } while (Game_Pointer[m][n] != 0);
    Game_Pointer[m][n] = 1;
  }
}

void Change_GameBoard(const int Max_Num, const int (*Game_Pointer)[size]) {
  system("cls");
  for (int i = 0; i < size; i++) {
    printf("*********************\n");
    for (int j = 0; j < size; j++) {
      if (Game_Pointer[i][j] != 0) {
        printf("|%4d", Game_Pointer[i][j]);
      }
      else printf("|    ");
    }
    printf("|\n");
  }
  printf("*********************\n");
}

char Arrow_Check(void) {
  int Arrow = getch();
  if (Arrow == 'w' || Arrow == 'a' || Arrow == 's' || Arrow == 'd') {
    return Arrow;
  }
  else return NULL;
}

void Sum_GameArray(const char Arrow, int (*Game_Pointer)[size]) {
  auto move_and_merge = [&](int start, int end, int step, int (*Game_Pointer)[size], int row_or_col) {
    for (int i = start; i != end; i += step) {
      for (int j = start; j != end; j += step) {
        int r = row_or_col ? i : j;
        int c = row_or_col ? j : i;
        if (Game_Pointer[r][c] != 0) {  
          if(row_or_col==0){   /*经分析此处若输入为w，r(j)为行数，c(i)为列数*/
            int next_r = r - step;
            int next_c = c;
            while (next_r >= 0 && next_r < size && next_c >= 0 && next_c < size && Game_Pointer[next_r][next_c] == 0) {
              Game_Pointer[next_r][next_c] = Game_Pointer[r][c];
              Game_Pointer[r][c] = 0;
              r = next_r;
              c = next_c;
              next_r -= step; 
            }
            if (next_r >= 0 && next_r < size && next_c >= 0 && next_c < size && Game_Pointer[next_r][next_c] == Game_Pointer[r][c]) {
              Game_Pointer[next_r][next_c] *= 2;
              Game_Pointer[r][c] = 0;
            }
          }
          else{
            int next_c = c - step;
            int next_r = r;
            while (next_r >= 0 && next_r < size && next_c >= 0 && next_c < size && Game_Pointer[next_r][next_c] == 0) {
              Game_Pointer[next_r][next_c] = Game_Pointer[r][c];
              Game_Pointer[r][c] = 0;
              r = next_r;
              c = next_c;
              next_c -= step; 
            }
            if (next_r >= 0 && next_r < size && next_c >= 0 && next_c < size && Game_Pointer[next_r][next_c] == Game_Pointer[r][c]) {
              Game_Pointer[next_r][next_c] *= 2;
              Game_Pointer[r][c] = 0;
            }
          }
        }
      }
    }
  };

  switch (Arrow) {
    case 'w':
      move_and_merge(0, size, 1, Game_Pointer, 0);
      break;
    case 'a':
      move_and_merge(0, size, 1, Game_Pointer, 1);
      break;
    case 's':
      move_and_merge(size - 1, -1, -1, Game_Pointer, 0);
      break;
    case 'd':
      move_and_merge(size - 1, -1, -1, Game_Pointer, 1);
      break;
    default:
      break;
  }
}

bool Blank_Check(const int (*Game_Pointer)[size]) {
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      if (Game_Pointer[i][j] == 0) {
        return true;
      }
    }
  }
  return false;
}

int Maxnum_Check(const int (*Game_Pointer)[size]) {
    int Maxnum = 1;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (Game_Pointer[i][j] > Maxnum) {
                Maxnum = Game_Pointer[i][j];
            }
        }
    }
    return Maxnum;
}

void Add_GameArray(int Max_Num, int (*Game_Pointer)[size]) {
  srand((unsigned)time(NULL));
  int m, n;
  do {
    m = rand() % size;
    n = rand() % size;
  } while (Game_Pointer[m][n] != 0);

  if (Max_Num >= 16 && Max_Num <= 128) {
    Game_Pointer[m][n] = pow(2, (rand() % 3) + 3);
  } 
  else if (Max_Num < 16) {
    Game_Pointer[m][n] = pow(2, rand() % 3);
  } 
  else if (Max_Num > 128) {
    Game_Pointer[m][n] = pow(2, (rand() % 3) + 6);
  }
}