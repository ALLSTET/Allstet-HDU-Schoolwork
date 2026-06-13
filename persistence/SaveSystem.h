#pragma once
#include <string>
#include <deque>
#include <fstream>
#include "../common/Commom.h"

/**
 * SaveSystem - 存档/读档系统
 *
 * 存档格式（文本序列化）：
 *   score=<分数>
 *   dir=<方向码: 0=UP,1=DOWN,2=LEFT,3=RIGHT>
 *   snake=<x1>,<y1>;<x2>,<y2>;...
 *   food=<x>,<y>
 *
 * 鲁棒性设计：
 * - 文件打开失败 → 返回 false
 * - 格式校验（字段完整、坐标合法）
 * - 存档损坏时优雅降级（提示用户而非崩溃）
 */

struct SaveData
{
  std::deque<Point> snake;
  Point food;
  Direction direction;
  int score;
};

constexpr int SLOT_AUTO = 0;
constexpr int SLOT_MIN = 1;
constexpr int SLOT_MAX = 3;

class SaveSystem
{
public:
  SaveSystem();

  // 自动存档（Q退出时自动保存到临时槽位）
  bool autoSave(const SaveData &state);
  bool autoLoad(SaveData &state);
  bool autoSaveExists() const;

  // 手动槽位存档（O键保存到永久槽位 1-3）
  bool saveToSlot(int slot, const SaveData &state);
  bool loadFromSlot(int slot, SaveData &state);
  bool slotExists(int slot) const;

private:
  std::string pathForSlot(int slot) const;
  bool saveImpl(const std::string &path, const SaveData &state);
  bool loadImpl(const std::string &path, SaveData &state);
};
