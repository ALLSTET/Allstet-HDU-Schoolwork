/**
 * Snake Game - 主入口 (工程化重构版)
 *
 * 项目结构：
 *   core/
 *     GameLoop.h/.cpp      - 游戏主循环（processInput → update → render）
 *     StateMachine.h/.cpp  - 有限状态机（MENU→RUNNING→PAUSED→GAME_OVER→EXIT）
 *   systems/
 *     InputSystem.h/.cpp   - 输入系统（最新合法输入覆盖策略）
 *     RenderSystem.h/.cpp  - 双缓冲控制台渲染
 *     TimeSystem.h/.cpp    - 时间控制系统（固定时间步长 + 多档速度）
 *   gameplay/
 *     Snake.h/.cpp         - 蛇实体（deque + 碰撞检测）
 *     Food.h/.cpp          - 食物（随机生成）
 *   persistence/
 *     SaveSystem.h/.cpp    - 存档/读档（文本序列化 + 鲁棒性校验）
 *   common/
 *     Commom.h             - 公共定义（Point / Direction / 常量）
 *
 * 控制方式：
 *   WASD / 方向键 - 移动
 *   P              - 暂停/继续
 *   O              - 保存存档
 *   Q              - 退出到菜单（自动保存）
 *
 * 核心设计：
 *   ★ 标准 Game Loop：while(running) { processInput(); update(); render(); }
 *   ★ 最新合法输入覆盖策略
 *   ★ 固定时间步长（基于 std::chrono）
 *   ★ FSM 状态机
 *   ★ 输入-更新-渲染 完全解耦
 */

#include "core/GameLoop.h"

int main()
{
  GameLoop game;
  game.run();
  return 0;
}
