#pragma once
#include "Snake.h"
#include "Food.h"
#include "Renderer.h"
#include "Input.h"
#include "SaveSystem.h"
#include "common/Commom.h"

// 游戏状态机
enum class GameStatus
{
  MENU,      // 主菜单
  PLAYING,   // 游戏进行中
  PAUSED,    // 暂停
  GAME_OVER, // 游戏结束
  EXIT       // 退出
};

class Game
{
private:
  Snake snake;
  Food food;
  Renderer renderer;
  Input input;
  SaveSystem saveSystem;

  GameStatus status;
  int score;
  int speed; // 游戏速度（毫秒/帧），越小越快

  // 用于双缓冲的二维画面数组
  char frame[GAME_HEIGHT][GAME_WIDTH];

public:
  Game();
  void run(); // 主循环入口

private:
  void menuLoop();     // 菜单循环
  void gameLoop();     // 游戏主循环
  void pauseLoop();    // 暂停循环
  void gameOverLoop(); // 游戏结束循环

  void handleInput(InputAction action); // 处理输入
  void update();                        // 更新游戏逻辑
  void render();                        // 渲染画面
  void buildFrame();                    // 构建双缓冲帧
  void newGame();                       // 开始新游戏

  void quitToMenu();   // Q：自动保存 → 返回菜单
  void saveWithSlot(); // O：选择槽位 → 手动保存
  void loadWithSlot(); // 菜单2：选择槽位 → 加载存档

  void spawnFood(); // 生成食物（避开蛇身）
};
