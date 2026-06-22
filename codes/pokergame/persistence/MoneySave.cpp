#include "MoneySave.h"
#include <fstream>
#include <ctime>

namespace
{
  std::string getSavePath()
  {
    return "money_save.dat";
  }

  int getToday()
  {
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    return (t->tm_year + 1900) * 10000 + (t->tm_mon + 1) * 100 + t->tm_mday;
  }
}

int MoneySave::initMoney()
{
  std::ifstream file(getSavePath());
  int money = 0;
  int lastDate = 0;
  bool hasSave = file.is_open();

  if (hasSave)
  {
    file >> money >> lastDate;
    file.close();
    // 如果读到的日期无效，视为无存档
    if (lastDate <= 0)
      hasSave = false;
  }

  int today = getToday();

  if (!hasSave)
  {
    // 第一次进入：3000 起步
    money = 3000;
    saveMoney(money);
  }
  else if (today > lastDate)
  {
    // 新的一天：+3000
    money += 3000;
    saveMoney(money);
  }

  return money;
}

void MoneySave::saveMoney(int amount)
{
  std::ofstream file(getSavePath());
  if (!file.is_open())
    return;
  file << amount << " " << getToday();
  file.close();
}
