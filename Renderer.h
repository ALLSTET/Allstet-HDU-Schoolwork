#pragma once
#include <string>
#include <sstream>
#include <deque>
#include "common/Commom.h"

class Renderer
{
public:
  std::ostringstream buffer; // 双缓冲：后台缓冲区（公开供 Game 直接写入）
  Renderer();

  // 清空缓冲区，准备新一帧
  void clear();

  // 硬清屏（仅在画面模式切换时使用，如菜单→游戏→暂停）
  void clearScreen();

  // 绘制游戏边框和内容到缓冲区
  void drawBorder();
  void drawSnake(const std::deque<Point> &body);
  void drawFood(const Point &food);
  void drawScore(int score);
  void drawGameOver(int finalScore);
  void drawPause();
  void drawMenu(bool autoExists, bool slot1Exists, bool slot2Exists, bool slot3Exists);
  void drawSaveSuccess();
  void drawLoadSuccess();
  void drawSaveFailed();
  void drawLoadFailed();

  // 存档槽位选择
  void drawSlotSelectSave(); // 保存时选槽位
  void drawSlotSelectLoad(); // 加载时选槽位

  // ★ 双缓冲核心：光标归位 + 一次性覆盖输出（无闪烁）
  void flush();

private:
  void homeCursor(); // 光标移到 (0,0)
  bool firstRender;  // 首次渲染标记
};
