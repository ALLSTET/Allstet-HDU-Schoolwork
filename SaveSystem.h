#pragma once
#include <string>
#include <deque>
#include <fstream>
#include "common/Commom.h"

// 游戏存档数据结构
struct GameState
{
  std::deque<Point> snake;
  Point food;
  Direction direction;
  int score;
};

// 存档槽位：0=自动存档(临时), 1-3=手动槽位(永久)
constexpr int SLOT_AUTO = 0;
constexpr int SLOT_MIN = 1;
constexpr int SLOT_MAX = 3;

class SaveSystem
{
public:
  SaveSystem();

  // ---------- 自动存档（临时，Q退出时自动保存） ----------
  bool autoSave(const GameState &state);
  bool autoLoad(GameState &state);
  bool autoSaveExists() const;

  // ---------- 手动槽位存档（永久，按键保存） ----------
  bool saveToSlot(int slot, const GameState &state);
  bool loadFromSlot(int slot, GameState &state);
  bool slotExists(int slot) const;

private:
  std::string pathForSlot(int slot) const;
  bool saveImpl(const std::string &path, const GameState &state);
  bool loadImpl(const std::string &path, GameState &state);
};
