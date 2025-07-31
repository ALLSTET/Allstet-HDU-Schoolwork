#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "string.h"
#include "math.h"
#include "conio.h"
#define size 4
/*2048需要什么？需要负责根据空白内容判定函数返回值判断游戏是否继续进行并定义二维数组且初始化为零的主函数，然后还需调用：
1.负责初始化生成空白游戏板的函数；2.负责二维数组初始内容生成的函数；
3.负责更新游戏板内容的函数；4.负责管理输入上下左右并反馈给合并内容函数的函数；
5.负责根据返回合并数组内容的函数；6.负责检查游戏板是否被填满的函数；
7.负责检测游戏当前最大数字的函数；8.负责每次输入后检索空白位置生成新内容的函数*/
void Initialize_GameBoard(void);
void Initialize_GameArray(int (*Game_Pointer)[size]);
void Change_GameBoard(const int Max_Num,const int (*Game_Pointer)[size]); 
char Arrow_Check(void);
void Sum_GameArray(const char Arrow,int (*Game_Pointer)[size]);
bool Blank_Check(const int (*Game_Pointer)[size]);
int Maxnum_Check(const int (*Game_Pointer)[size]);
void Add_GameArray(int Max_Num,int (*Game_Pointer)[size]);
int score=0;

int main(){
  bool Flag=false; 
  int Game_Array[size][size],Max_Num=1;
  char answer;
  memset(Game_Array,0,sizeof(Game_Array));
  int (*Game_Pointer)[size];
  Game_Pointer=Game_Array;
  Initialize_GameBoard();
  printf("START the GAME?\nPls PRINT Y(yes) or N(no):");
  answer=getch();
  printf("\n");
  if (answer=='Y' || answer=='y'){
    Flag=true;
    Initialize_GameArray(Game_Pointer);
  }
  else if (answer=='N' || answer=='n'){
    system("cls");
    goto JUMP1;
  }
  else{
    system("cls");
    printf("UNDEFINED INPUT!");
    goto JUMP2;
  }
  do{
    Change_GameBoard(Max_Num,Game_Pointer);
    Sum_GameArray(Arrow_Check(),Game_Pointer);
    Max_Num=Maxnum_Check(Game_Pointer);
    score=Max_Num*2;
    Change_GameBoard(Max_Num,Game_Pointer);
    if (Blank_Check(Game_Pointer)==true){
      Add_GameArray(Max_Num,Game_Pointer);
      continue;
    }
    else{
      Flag=false;
    }
  }while(Flag==true);
JUMP1:
  printf("GAME OVER!\nYour Final Score is:%d\n",score);
JUMP2:
  getchar();
  free(Game_Pointer);
  return 0;
}

void Initialize_GameBoard(void){
  for (int i=1;i<=9;i++){
    if (i%2!=0){
    printf("*********************\n");
    }
    else{
    printf("|    |    |    |    |\n"); 
    }
  }  
}
/*对于随机生成数组内容，首先控制生成内容次数随机但控制在2-3次，初始生成的数字暂不考虑最大数影响，只生成1，
如果随机到重复的位置，重新生成一个位置进行赋值*/
void Initialize_GameArray(int (*Game_Pointer)[size]){
  srand((unsigned)time(NULL));
  int count=(rand()%2)+1;
  for (int i=0;i<count;i++){
    int m=rand()%size+0,n=rand()%size+0;
    if (Game_Pointer[m][n]!=0){
      count++;
    }
    else{
      Game_Pointer[m][n]=1;
    }
  }
}

void Change_GameBoard(const int Max_Num,const int (*Game_Pointer)[size]){
  system("cls");
  for (int i=1;i<=9;i++){
    if (i%2!=0){
      printf("*********************");
      for(int k=Max_Num;k>=10;k=k/10){
        printf("*");
      }
      printf("\n");
    }
    else{
      for(int j=0;j<=3;j++){
        if(Game_Pointer[(i/2-1)][j]!=0){
          printf("|%4d",Game_Pointer[(i/2-1)][j]);
        }
        else{
          printf("|    ");
        }
      }
      printf("|\n");
    }
  }  
}

char Arrow_Check(void){
  int Arrow=getch();
  switch (Arrow){
    case 'w':
      return Arrow;
      break;
    case 'a':
      return Arrow;
      break;
    case 's':
      return Arrow;
      break;
    case 'd':
      return Arrow;
      break;
    default:
      return NULL;
      break; 
  }
}
/*倘若要向上合并，需要满足：向上一行无数字或者数字与本行相同，从上向下从左至右进行检查合并。其他同理*/
void Sum_GameArray(const char Arrow,int (*Game_Pointer)[size]){
  switch (Arrow){
    case 'w':
      for(int i=1;i<size;i++){
        for(int j=0;j<size;j++){
          if(Game_Pointer[i][j]!=0){
            if(Game_Pointer[i-1][j]==0){
              Game_Pointer[i-1][j]=Game_Pointer[i][j];
              Game_Pointer[i][j]=0;
              if(Game_Pointer[i-2][j]==0){
                Game_Pointer[i-2][j]=Game_Pointer[i-1][j];
                Game_Pointer[i-1][j]=0;
                if(Game_Pointer[i-3][j]==0){
                  Game_Pointer[i-3][j]=Game_Pointer[i-2][j];
                  Game_Pointer[i-2][j]=0;
                }
                else if(Game_Pointer[i-3][j]==Game_Pointer[i-2][j]){
                  Game_Pointer[i-3][j]+=Game_Pointer[i-2][j];
                  Game_Pointer[i-2][j]=0;
                }
                else{
                  continue;
                }
              }
              else if(Game_Pointer[i-2][j]==Game_Pointer[i-1][j]){
                Game_Pointer[i-2][j]+=Game_Pointer[i-1][j];
                Game_Pointer[i-1][j]=0;
              }
              else{
                continue;
              }
            }
            else if(Game_Pointer[i-1][j]==Game_Pointer[i][j]){
              Game_Pointer[i-1][j]+=Game_Pointer[i][j];
              Game_Pointer[i][j]=0;
            }
            else{
              continue;
            }         
          }
          else{
            continue;
          }
        }
      }
      break;
    case 'a':
      for(int i=0;i<size;i++){
        for(int j=1;j<size;j++){
          if(Game_Pointer[i][j]!=0){
            if(Game_Pointer[i][j-1]==0){
              Game_Pointer[i][j-1]=Game_Pointer[i][j];
              Game_Pointer[i][j]=0;
              if(Game_Pointer[i][j-2]==0){
                Game_Pointer[i][j-2]=Game_Pointer[i][j-1];
                Game_Pointer[i][j-1]=0;
                if(Game_Pointer[i][j-3]==0){
                  Game_Pointer[i][j-3]=Game_Pointer[i][j-2];
                  Game_Pointer[i][j-2]=0;
                }
                else if(Game_Pointer[i][j-3]==Game_Pointer[i][j-2]){
                  Game_Pointer[i][j-3]+=Game_Pointer[i][j-2];
                  Game_Pointer[i][j-2]=0;
                }
                else{
                  continue;
                }
              }
              else if(Game_Pointer[i][j-2]==Game_Pointer[i][j-1]){
                Game_Pointer[i][j-2]+=Game_Pointer[i][j-1];
                Game_Pointer[i][j-1]=0;
              }
              else{
                continue;
              }
            }
            else if(Game_Pointer[i][j-1]==Game_Pointer[i][j]){
              Game_Pointer[i][j-1]+=Game_Pointer[i][j];
              Game_Pointer[i][j]=0;
            }
            else{
              continue;
            }         
          }
          else{
            continue;
          }
        }
      }
      break;
    case 's':
      for(int i=(size-2);i>=0;i--){
        for(int j=0;j<size;j++){
          if(Game_Pointer[i][j]!=0){
            if(Game_Pointer[i+1][j]==0){
              Game_Pointer[i+1][j]=Game_Pointer[i][j];
              Game_Pointer[i][j]=0;
              if(Game_Pointer[i+2][j]==0){
                Game_Pointer[i+2][j]=Game_Pointer[i+1][j];
                Game_Pointer[i+1][j]=0;
                if(Game_Pointer[i+3][j]==0){
                  Game_Pointer[i+3][j]=Game_Pointer[i+2][j];
                  Game_Pointer[i+2][j]=0;
                }
                else if(Game_Pointer[i+3][j]==Game_Pointer[i+2][j]){
                  Game_Pointer[i+3][j]+=Game_Pointer[i+2][j];
                  Game_Pointer[i+2][j]=0;
                }
                else{
                  continue;
                }
              }
              else if(Game_Pointer[i+2][j]==Game_Pointer[i+1][j]){
                Game_Pointer[i+2][j]+=Game_Pointer[i+1][j];
                Game_Pointer[i+1][j]=0;
              }
              else{
                continue;
              }
            }
            else if(Game_Pointer[i+1][j]==Game_Pointer[i][j]){
              Game_Pointer[i+1][j]+=Game_Pointer[i][j];
              Game_Pointer[i][j]=0;
            }
            else{
              continue;
            }         
          }
          else{
            continue;
          }
        }
      }
      break;
    case 'd':
      for(int i=0;i<size;i++){
        for(int j=(size-2);j>=0;j--){
          if(Game_Pointer[i][j]!=0){
            if(Game_Pointer[i][j+1]==0){
              Game_Pointer[i][j+1]=Game_Pointer[i][j];
              Game_Pointer[i][j]=0;
              if(Game_Pointer[i][j+2]==0){
                Game_Pointer[i][j+2]=Game_Pointer[i][j+1];
                Game_Pointer[i][j+1]=0;
                if(Game_Pointer[i][j+3]==0){
                  Game_Pointer[i][j+3]=Game_Pointer[i][j+2];
                  Game_Pointer[i][j+2]=0;
                }
                else if(Game_Pointer[i][j+3]==Game_Pointer[i][j+2]){
                  Game_Pointer[i][j+3]+=Game_Pointer[i][j+2];
                  Game_Pointer[i][j+2]=0;
                }
                else{
                  continue;
                }
              }
              else if(Game_Pointer[i][j+2]==Game_Pointer[i][j+1]){
                Game_Pointer[i][j+2]+=Game_Pointer[i][j+1];
                Game_Pointer[i][j+1]=0;
              }
              else{
                continue;
              }
            }
            else if(Game_Pointer[i][j+1]==Game_Pointer[i][j]){
              Game_Pointer[i][j+1]+=Game_Pointer[i][j];
              Game_Pointer[i][j]=0;
            }
            else{
              continue;
            }         
          }
          else{
            continue;
          }
        }
      }
      break;
    default:
      break; 
  }
}

bool Blank_Check(const int(*Game_Pointer)[size]){
  int count=0;
  for (int i=0;i<size;i++){
    for (int j=0;j<size;j++){
      if (Game_Pointer[i][j]==0){
        count++;
      }
      else{
        continue;
      }
    }
  }
  if(count!=0){
    return true;
  }
  else{
    return false;
  }
}

int Maxnum_Check(const int (*Game_Pointer)[size]){
  int Maxnum=1;
  for (int i=0;i<size;i++){
    for (int j=0;j<size;j++){
      if(Game_Pointer[i][j]>Maxnum){
        Maxnum=Game_Pointer[i][j];
      }
    }
  }
  return Maxnum;
}
/*对于被多参数影响生成新内容的函数，生成次数为1-2次，内容根据场上最大数决定上下限*/
void Add_GameArray(int Max_Num,int (*Game_Pointer)[size]){
  srand((unsigned)time(NULL));
  int count=1;
  for (int i=0;i<count;i++){
    int m=rand()%size+0,n=rand()%size+0;
    if(Max_Num>=16 && Max_Num<=128){
      int Newnum=pow(2,(rand()%3)+3);
      if(Game_Pointer[m][n]==0){
        Game_Pointer[m][n]=Newnum;
      }
      else{
        count++;
        continue;
      }
    }
    else if(Max_Num<16){
      int Newnum=pow(2,rand()%3);
      if(Game_Pointer[m][n]==0){
        Game_Pointer[m][n]=Newnum;
      }
      else{
        count++;
        continue;
      }
    }
    else if(Max_Num>128){
      int Newnum=pow(2,(rand()%3)+6);
      if(Game_Pointer[m][n]==0){
        Game_Pointer[m][n]=Newnum;
      }
      else{
        count++;
        continue;
      }
    }
  }
}
/**/