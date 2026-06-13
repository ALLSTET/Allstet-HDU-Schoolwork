#include "SaveSystem.h"
#include <sstream>

SaveSystem::SaveSystem() {}

std::string SaveSystem::pathForSlot(int slot) const
{
  if (slot == SLOT_AUTO)
    return "snake_autosave.txt";
  return "snake_save_" + std::to_string(slot) + ".txt";
}

bool SaveSystem::saveImpl(const std::string &path, const SaveData &state)
{
  std::ofstream file(path);
  if (!file.is_open())
    return false;

  file << "score=" << state.score << "\n";

  int dirInt = 0;
  switch (state.direction)
  {
  case Direction::UP:
    dirInt = 0;
    break;
  case Direction::DOWN:
    dirInt = 1;
    break;
  case Direction::LEFT:
    dirInt = 2;
    break;
  case Direction::RIGHT:
    dirInt = 3;
    break;
  default:
    break;
  }
  file << "dir=" << dirInt << "\n";

  file << "snake=";
  for (size_t i = 0; i < state.snake.size(); ++i)
  {
    file << state.snake[i].x << "," << state.snake[i].y;
    if (i + 1 < state.snake.size())
      file << ";";
  }
  file << "\n";

  file << "food=" << state.food.x << "," << state.food.y << "\n";
  return true;
}

bool SaveSystem::loadImpl(const std::string &path, SaveData &state)
{
  std::ifstream file(path);
  if (!file.is_open())
    return false;

  bool hasScore = false, hasDir = false, hasSnake = false, hasFood = false;

  std::string line;
  while (std::getline(file, line))
  {
    if (line.empty())
      continue;
    size_t eqPos = line.find('=');
    if (eqPos == std::string::npos)
      continue;

    std::string key = line.substr(0, eqPos);
    std::string value = line.substr(eqPos + 1);

    if (key == "score")
    {
      try
      {
        state.score = std::stoi(value);
        hasScore = true;
      }
      catch (...)
      {
        return false;
      } // 存档损坏
    }
    else if (key == "dir")
    {
      try
      {
        int dirInt = std::stoi(value);
        switch (dirInt)
        {
        case 0:
          state.direction = Direction::UP;
          break;
        case 1:
          state.direction = Direction::DOWN;
          break;
        case 2:
          state.direction = Direction::LEFT;
          break;
        case 3:
          state.direction = Direction::RIGHT;
          break;
        default:
          return false; // 存档损坏
        }
        hasDir = true;
      }
      catch (...)
      {
        return false;
      }
    }
    else if (key == "snake")
    {
      state.snake.clear();
      std::stringstream ss(value);
      std::string segment;
      while (std::getline(ss, segment, ';'))
      {
        size_t commaPos = segment.find(',');
        if (commaPos == std::string::npos)
          return false;
        try
        {
          int x = std::stoi(segment.substr(0, commaPos));
          int y = std::stoi(segment.substr(commaPos + 1));
          state.snake.push_back({x, y});
        }
        catch (...)
        {
          return false;
        }
      }
      if (state.snake.empty())
        return false;
      hasSnake = true;
    }
    else if (key == "food")
    {
      size_t commaPos = value.find(',');
      if (commaPos == std::string::npos)
        return false;
      try
      {
        state.food.x = std::stoi(value.substr(0, commaPos));
        state.food.y = std::stoi(value.substr(commaPos + 1));
        hasFood = true;
      }
      catch (...)
      {
        return false;
      }
    }
  }

  // 完整性校验：所有字段必须存在
  return hasScore && hasDir && hasSnake && hasFood;
}

// ── 自动存档 ──
bool SaveSystem::autoSave(const SaveData &state) { return saveImpl(pathForSlot(SLOT_AUTO), state); }
bool SaveSystem::autoLoad(SaveData &state) { return loadImpl(pathForSlot(SLOT_AUTO), state); }
bool SaveSystem::autoSaveExists() const { return std::ifstream(pathForSlot(SLOT_AUTO)).good(); }

// ── 手动槽位存档 ──
bool SaveSystem::saveToSlot(int slot, const SaveData &state) { return saveImpl(pathForSlot(slot), state); }
bool SaveSystem::loadFromSlot(int slot, SaveData &state) { return loadImpl(pathForSlot(slot), state); }
bool SaveSystem::slotExists(int slot) const { return std::ifstream(pathForSlot(slot)).good(); }
