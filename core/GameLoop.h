#pragma once
#include "../common/Commom.h"
#include "StateMachine.h"
#include "../systems/InputSystem.h"
#include "../systems/TimeSystem.h"
#include "../systems/RenderSystem.h"
#include "../gameplay/Snake.h"
#include "../gameplay/Food.h"
#include "../persistence/SaveSystem.h"

/**
 * GameLoop - 游戏主循环（工程化核心）
 *
 * 采用标准游戏循环结构：
 *
 *   while (running) {
 *       processInput();   // 输入采集（高频、非阻塞）
 *       if (shouldUpdate) {
 *           update();      // 状态更新（固定时间步长）
 *       }
 *       render();          // 渲染输出（每帧）
 *   }
 *
 * 关键设计：
 * - 输入采集与逻辑更新完全解耦
 * - 输入可以高频发生，但逻辑按固定节奏更新
 * - 渲染与逻辑更新频率独立
 */

class GameLoop
{
public:
  GameLoop();
  void run(); // 程序入口

private:
  // ── 子系统 ──
  StateMachine fsm;
  InputSystem inputSystem;
  TimeSystem timeSystem;
  RenderSystem renderer;
  SaveSystem saveSystem;

  // ── 游戏对象 ──
  Snake snake;
  Food food;

  // ── 游戏状态 ──
  int score;
  char frame[GAME_HEIGHT][GAME_WIDTH];

  // ── 循环 ──
  void menuLoop();
  void gameLoop();
  void pauseLoop();
  void gameOverLoop();

  // ── 核心三步 ──
  void processInput(); // 输入采集
  void update();       // 状态更新
  void renderScene();  // 渲染输出

  // ── 辅助 ──
  void buildFrame(); // 构建双缓冲帧
  void spawnFood();
  void newGame();
  void quitToMenu();
  void saveWithSlot();
  void loadWithSlot();
  void applyDirection(Direction newDir);
};
