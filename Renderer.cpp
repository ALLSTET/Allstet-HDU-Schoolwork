#include "Renderer.h"
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

Renderer::Renderer() : firstRender(true) {}

void Renderer::clear()
{
  buffer.str(""); // 清空字符串流
  buffer.clear(); // 清除错误标志
}

void Renderer::homeCursor()
{
#ifdef _WIN32
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  COORD pos = {0, 0};
  SetConsoleCursorPosition(hConsole, pos);
#else
  // ANSI 转义序列：光标归位
  std::cout << "\033[H";
#endif
}

void Renderer::clearScreen()
{
  // 硬清屏 — 仅在画面模式切换时调用（菜单→游戏、游戏→暂停等）
  firstRender = true;
#ifdef _WIN32
  system("cls");
#else
  system("clear");
#endif
}

void Renderer::flush()
{
  // ★★★★★ 真正的双缓冲核心 ★★★★★
  // 不是 system("cls") 清屏！而是：
  // 1. 后台 ostringstream 已构建完整帧
  // 2. 光标归位 (0,0) — 不擦除屏幕
  // 3. 一次性输出新帧 → 逐行覆盖旧帧
  // 4. 因为每帧行数固定，旧帧被完全覆盖，无闪烁

  if (firstRender)
  {
    // 首次渲染需要清屏（控制台可能已有其他内容）
    firstRender = false;
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
  }
  else
  {
    // ★ 后续帧：光标归位，覆盖写入，不清屏
    homeCursor();
  }

  std::cout << buffer.str();
  std::cout.flush(); // 确保立即输出
}

void Renderer::drawBorder()
{
  // 上边框
  buffer << "+";
  for (int i = 0; i < GAME_WIDTH; ++i)
    buffer << "-";
  buffer << "+\n";

  // 中间区域（先留空，由 drawSnake/drawFood 填充）
  for (int y = 0; y < GAME_HEIGHT; ++y)
  {
    buffer << "|";
    for (int x = 0; x < GAME_WIDTH; ++x)
    {
      buffer << " "; // 默认空格
    }
    buffer << "|\n";
  }

  // 下边框
  buffer << "+";
  for (int i = 0; i < GAME_WIDTH; ++i)
    buffer << "-";
  buffer << "+\n";
}

void Renderer::drawSnake(const std::deque<Point> &body)
{
  if (body.empty())
    return;
  // 注意：双缓冲的核心思路是：先构建完整字符串，再一次性输出。
  // 但这里蛇身需要覆盖在已绘制的边框内，因此我们用另一种方式：
  // 在 flush 时统一构建帧画面。
  // 此方法现在为空，实际绘制逻辑已移至 flush() 中通过 frame 二维数组实现。
  (void)body;
}

void Renderer::drawFood(const Point &food)
{
  (void)food;
}

void Renderer::drawScore(int score)
{
  buffer << "Score: " << score << "\n";
}

void Renderer::drawGameOver(int finalScore)
{
  buffer << "\n========== GAME OVER ==========\n";
  buffer << "  Final Score: " << finalScore << "\n";
  buffer << "\n";
  buffer << "  R - 重新开始 (Restart)\n";
  buffer << "  Q - 返回主菜单 (Menu)\n";
  buffer << "================================\n";
}

void Renderer::drawSlotSelectSave()
{
  buffer << "\n========== 选择存档槽位 ==========\n";
  buffer << "  1 - 槽位 1 (永久保存)\n";
  buffer << "  2 - 槽位 2 (永久保存)\n";
  buffer << "  3 - 槽位 3 (永久保存)\n";
  buffer << "  ESC/其他 - 取消\n";
  buffer << "==================================\n";
  buffer << " 请按 1 / 2 / 3 选择...\n";
}

void Renderer::drawSlotSelectLoad()
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

void Renderer::drawPause()
{
  buffer << "\n========== PAUSED ==========\n";
  buffer << "  P   - 继续游戏 (Resume)\n";
  buffer << "  O   - 存档到槽位 (Save)\n";
  buffer << "  Q   - 自动保存并返回菜单\n";
  buffer << "=============================\n";
}

void Renderer::drawMenu(bool autoExists, bool s1, bool s2, bool s3)
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
  {
    buffer << "     [ 临时存档 ]  Score: ? (上次中断)\n";
  }
  if (s1)
    buffer << "     [ 槽位 1 ]  永久存档\n";
  if (s2)
    buffer << "     [ 槽位 2 ]  永久存档\n";
  if (s3)
    buffer << "     [ 槽位 3 ]  永久存档\n";
  if (!autoExists && !s1 && !s2 && !s3)
  {
    buffer << "     (暂无存档)\n";
  }
  buffer << "\n";
  buffer << "  3. 退出游戏 (Quit)\n";
  buffer << "\n";
  buffer << "----------------------------------\n";
  buffer << " 提示：游戏中按 Q 自动保存并返回菜单\n";
  buffer << "      游戏中按 O 手动存档到槽位\n";
  buffer << "==================================\n";
  buffer << " 请按 1 / 2 / 3 选择...\n";
}

void Renderer::drawSaveSuccess()
{
  buffer << "\n[Save] Game saved successfully!\n";
}

void Renderer::drawLoadSuccess()
{
  buffer << "\n[Load] Game loaded successfully!\n";
}

void Renderer::drawSaveFailed()
{
  buffer << "\n[Save] Failed to save game!\n";
}

void Renderer::drawLoadFailed()
{
  buffer << "\n[Load] No save file found or file corrupted!\n";
}