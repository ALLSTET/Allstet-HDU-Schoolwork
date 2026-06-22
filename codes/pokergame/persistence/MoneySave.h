#pragma once

namespace MoneySave
{

  // 加载存档 + 每日登录奖励，返回最终金额
  int initMoney();

  // 保存当前金额（附带今日日期）
  void saveMoney(int amount);

} // namespace MoneySave
