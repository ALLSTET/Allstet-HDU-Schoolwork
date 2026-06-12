#pragma once

// 输入类型枚举
enum class InputAction
{
  NONE,
  MOVE_UP,
  MOVE_DOWN,
  MOVE_LEFT,
  MOVE_RIGHT,
  PAUSE,
  SAVE,
  LOAD,
  QUIT,
  CONFIRM,   // 确认/回车
  NEW_GAME,  // 菜单：新游戏
  LOAD_GAME, // 菜单：加载存档
  RESTART,   // 游戏结束后重新开始
  SLOT_0,    // 槽位 0（自动存档）
  SLOT_1,    // 槽位 1
  SLOT_2,    // 槽位 2
  SLOT_3,    // 槽位 3
  CANCEL     // 取消（ESC）
};

class Input
{
public:
  Input() = default;

  // 非阻塞检测键盘输入，返回对应的动作
  InputAction poll(bool isMenu = false, bool isGameOver = false);

  // 槽位选择模式：仅响应 0/1/2/3/ESC
  InputAction pollSlot();
};
