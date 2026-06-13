#pragma once
#include <chrono>
#include "../common/Commom.h"

/**
 * TimeSystem - 时间控制系统
 *
 * 职责：
 * 1. 管理游戏逻辑更新的固定时间步长
 * 2. 支持多档速度（SLOW / MEDIUM / FAST）
 * 3. 支持随得分动态加速
 * 4. 提供 shouldUpdate() 判定是否该执行下一帧逻辑
 *
 * 设计要点：
 * - 禁止 Sleep() 控制帧率，完全基于 std::chrono::steady_clock
 * - 输入采集与渲染独立于逻辑更新，实现输入-更新解耦
 */

class TimeSystem
{
public:
  using Clock = std::chrono::steady_clock;

  // 预设速度档位
  enum class Speed
  {
    SLOW,   // 200ms/tick
    MEDIUM, // 130ms/tick
    FAST    // 70ms/tick
  };

  TimeSystem();

  // 设置速度档位
  void setSpeed(Speed speed);
  void setSpeedMs(int ms);

  // 动态加速：每吃N个食物调用一次
  void increaseSpeed(int stepMs = 20);

  // 核心判定：从上一帧到现在是否经过了足够的时间？
  // 是 → 返回 true，内部自动标记本帧已更新
  bool shouldUpdate();

  // 手动标记本次更新完成（重设计时器）
  void markUpdated();

  // 获取当前 tick 间隔（毫秒）
  int getTickInterval() const { return tickIntervalMs; }

  // 获取帧率近似值
  int getFps() const;

  // 重置计时器（新游戏开始时调用）
  void reset();

private:
  Clock::time_point lastTick; // 上一次逻辑更新的时间点
  int tickIntervalMs;         // 每次逻辑更新的间隔（毫秒）
};
