#pragma once
#include "../common/Commom.h"

/**
 * InputSystem - 输入系统（本任务核心模块）
 *
 * ★ 采用“最新合法输入覆盖策略”：
 *   同一帧内遍历所有按键，只保留最后一个不违反方向约束的输入，
 *   其余输入全部丢弃。
 *
 * ── 为什么不用队列？
 *   队列会延迟执行历史输入。例如用户快速按 RIGHT → DOWN → LEFT，
 *   如果使用队列，蛇会在后续多帧中依次执行这些方向，
 *   但此时游戏状态早已变化，导致“按过头”的糟糕体验。
 *
 * ── 为什么不能只取第一个输入？
 *   第一帧输入可能是误触或过渡按键。用户连续快速按键时，
 *   “最后一个输入”才是用户的最终意图，覆盖策略更符合直觉。
 *
 * ── 180°反向过滤：
 *   所有方向输入都经过 isOpposite() 校验，防止蛇头撞身体。
 */

class InputSystem
{
public:
  // 一帧内收集到的所有输入信息
  struct FrameInput
  {
    Direction nextDirection = Direction::STOP; // STOP 表示无方向输入
    bool pausePressed = false;
    bool savePressed = false;
    bool quitPressed = false;
    bool confirmPressed = false;
  };

  InputSystem();

  // 设置当前蛇的移动方向（由 GameLoop 在每帧更新后同步）
  void setCurrentDirection(Direction d);

  // ★ 核心方法：清空输入缓冲区，返回本帧最终输入
  //   内部实现“最新合法输入覆盖策略”
  FrameInput poll();

  // 菜单/结束画面等阻塞式输入（等待一次有效按键）
  // 返回按键字符（数字键/字母键），ESC 返回 27
  int waitForKey();

private:
  Direction currentDirection;
  bool kbhit(); // 检测是否有按键
  int getch();  // 读取一个字符（含方向键处理，返回原始键码或负数表示方向键）
  bool isArrowKey(int ch);
  int readArrowDirection(int firstCh); // 读取方向键剩余字节→返回 Direction 的 int 编码
};
