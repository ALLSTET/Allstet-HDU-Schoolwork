#include "RenderSystem.h"
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

RenderSystem::RenderSystem() : firstRender(true) {}

void RenderSystem::clear()
{
  buffer.str("");
  buffer.clear();
}

void RenderSystem::homeCursor()
{
#ifdef _WIN32
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  COORD pos = {0, 0};
  SetConsoleCursorPosition(hConsole, pos);
#else
  std::cout << "\033[H";
#endif
}

void RenderSystem::clearScreen()
{
  firstRender = true;
#ifdef _WIN32
  system("cls");
#else
  system("clear");
#endif
}

void RenderSystem::flush()
{
  if (firstRender)
  {
    firstRender = false;
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
  }
  else
  {
    homeCursor();
  }

  std::cout << buffer.str();
  std::cout.flush();
}

void RenderSystem::drawMenu(bool autoExists, bool s1, bool s2, bool s3)
{
  buffer << "\n";
  buffer << "==================================\n";
  buffer << "          SNAKE  GAME             \n";
  buffer << "==================================\n";
  buffer << "\n";
  buffer << "  1. 开始新游戏 (New Game)\n";
  buffer << "\n";
  buffer << "  2. 加载游戏 (Load Game)\n";
  if (autoExists)
    buffer << "     [ 临时存档 ]  (上次中断)\n";
  if (s1)
    buffer << "     [ 槽位 1 ]  永久存档\n";
  if (s2)
    buffer << "     [ 槽位 2 ]  永久存档\n";
  if (s3)
    buffer << "     [ 槽位 3 ]  永久存档\n";
  if (!autoExists && !s1 && !s2 && !s3)
    buffer << "     (暂无存档)\n";
  buffer << "\n";
  buffer << "  3. 退出游戏 (Quit)\n";
  buffer << "\n";
  buffer << "----------------------------------\n";
  buffer << " 提示：游戏中按 Q 自动保存并返回菜单\n";
  buffer << "      游戏中按 O 手动存档到槽位\n";
  buffer << "==================================\n";
  buffer << " 请按 1 / 2 / 3 选择...\n";
}

void RenderSystem::drawGameFrame(const char frame[GAME_HEIGHT][GAME_WIDTH], int score, int fps)
{
  // 上边框
  buffer << "+";
  for (int i = 0; i < GAME_WIDTH; ++i)
    buffer << "-";
  buffer << "+\n";

  // 游戏内容
  for (int y = 0; y < GAME_HEIGHT; ++y)
  {
    buffer << "|";
    for (int x = 0; x < GAME_WIDTH; ++x)
      buffer << frame[y][x];
    buffer << "|\n";
  }

  // 下边框
  buffer << "+";
  for (int i = 0; i < GAME_WIDTH; ++i)
    buffer << "-";
  buffer << "+\n";

  // 状态栏
  buffer << "\nScore: " << score;
  buffer << "   Speed: " << fps << "fps\n";
  buffer << "WASD/方向键:移动 | P:暂停 | O:存档到槽位 | Q:自动保存并返回菜单\n";
}

void RenderSystem::drawGameOver(int finalScore)
{
  buffer << "\n========== GAME OVER ==========\n";
  buffer << "  Final Score: " << finalScore << "\n";
  buffer << "\n";
  buffer << "  R - 重新开始 (Restart)\n";
  buffer << "  Q - 返回主菜单 (Menu)\n";
  buffer << "================================\n";
}

void RenderSystem::drawPause()
{
  buffer << "\n========== PAUSED ==========\n";
  buffer << "  P   - 继续游戏 (Resume)\n";
  buffer << "  O   - 存档到槽位 (Save)\n";
  buffer << "  Q   - 自动保存并返回菜单\n";
  buffer << "=============================\n";
}

void RenderSystem::drawSlotSelectSave()
{
  buffer << "\n========== 选择存档槽位 ==========\n";
  buffer << "  1 - 槽位 1 (永久保存)\n";
  buffer << "  2 - 槽位 2 (永久保存)\n";
  buffer << "  3 - 槽位 3 (永久保存)\n";
  buffer << "  ESC/其他 - 取消\n";
  buffer << "==================================\n";
  buffer << " 请按 1 / 2 / 3 选择...\n";
}

void RenderSystem::drawSlotSelectLoad()
{
  buffer << "\n========== 选择读取槽位 ==========\n";
  buffer << "  0 - 临时存档 (上次中断)\n";
  buffer << "  1 - 槽位 1 (永久)\n";
  buffer << "  2 - 槽位 2 (永久)\n";
  buffer << "  3 - 槽位 3 (永久)\n";
  buffer << "  ESC/其他 - 取消\n";
  buffer << "==================================\n";
  buffer << " 请按 0 / 1 / 2 / 3 选择...\n";
}

void RenderSystem::drawSaveSuccess()
{
  buffer << "\n[Save] Game saved successfully!\n";
}

void RenderSystem::drawLoadSuccess()
{
  buffer << "\n[Load] Game loaded successfully!\n";
}

void RenderSystem::drawLoadFailed()
{
  buffer << "\n[ERROR] 存档损坏或不存在！\n";
}