#pragma once
#include <string>
#include <sstream>
#include <deque>
#include "../common/Commom.h"

/**
 * RenderSystem - 双缓冲控制台渲染系统
 *
 * 渲染流程：
 *   1. clear()      → 清空后台缓冲区
 *   2. drawXxx()    → 写入缓冲区（可多次调用）
 *   3. flush()      → 一次性输出到控制台（覆盖旧帧，无闪烁）
 *
 * 双缓冲原理：
 *   不是 system("cls") 清屏！
 *   而是光标归位 (0,0) 后逐行覆盖写入。
 *   因为每帧行数固定，旧内容被完全覆盖，实现无闪烁渲染。
 */

class RenderSystem
{
public:
  std::ostringstream buffer; // 后台缓冲区（公开供外部直接写入）

  RenderSystem();

  // 清空缓冲区，准备新一帧
  void clear();

  // 硬清屏（仅在画面模式切换时使用，如菜单→游戏→暂停）
  void clearScreen();

  // 双缓冲核心：光标归位 + 一次性覆盖输出（无闪烁）
  void flush();

  // ── 各画面绘制 ──
  void drawMenu(bool autoExists, bool s1, bool s2, bool s3);
  void drawGameFrame(const char frame[GAME_HEIGHT][GAME_WIDTH], int score, int fps);
  void drawGameOver(int finalScore);
  void drawPause();
  void drawSlotSelectSave();
  void drawSlotSelectLoad();
  void drawSaveSuccess();
  void drawLoadSuccess();
  void drawLoadFailed();

private:
  void homeCursor(); // 光标移到 (0,0)
  bool firstRender;  // 首次渲染标记
};
