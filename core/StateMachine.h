#pragma once

/**
 * StateMachine - 有限状态机（FSM）
 *
 * 状态转换图：
 *
 *     ┌──────────┐
 *     │   MENU   │ ← ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐
 *     └────┬─────┘                               │
 *          │ 1.新游戏                              │ Q 退出到菜单
 *          ▼                                      │
 *     ┌──────────┐    P 暂停     ┌──────────┐     │
 *     │ RUNNING  │ ─ ─ ─ ─ ─ →  │  PAUSED  │     │
 *     └────┬─────┘              └────┬─────┘     │
 *          │ 死亡              P 继续 │  Q 退出   │
 *          ▼                         │           │
 *     ┌──────────┐                   │           │
 *     │GAME_OVER │                   │           │
 *     └────┬─────┘                   │           │
 *          │ R 重开                   │           │
 *          │                         │           │
 *          ▼                         ▼           │
 *     ┌──────────┐              ┌──────────┐     │
 *     │ RUNNING  │              │   MENU   │ ────┘
 *     └──────────┘              └────┬─────┘
 *                                    │ 3.退出
 *                                    ▼
 *                               ┌──────────┐
 *                               │   EXIT   │
 *                               └──────────┘
 *
 * 输入行为与状态绑定：
 * - MENU：方向键无效，仅 1/2/3 有效
 * - RUNNING：方向键/PAUSE/SAVE/QUIT 有效
 * - PAUSED：方向键无效，仅 P(继续)/O(存档)/Q(退出)
 * - GAME_OVER：方向键无效，仅 R(重开)/Q(退出)
 */

enum class GameState
{
  MENU,
  RUNNING,
  PAUSED,
  GAME_OVER,
  EXIT
};

class StateMachine
{
public:
  StateMachine();

  GameState getState() const;
  void transitionTo(GameState newState);

  // 便利查询
  bool isRunning() const;
  bool isPaused() const;
  bool isGameOver() const;
  bool isMenu() const;
  bool isExiting() const;

  // 方向输入是否应该被处理
  bool canProcessDirection() const;

private:
  GameState currentState;
};
