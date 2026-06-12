/**
 * GreedySnake - 贪吃蛇游戏 (工程化实现)
 *
 * 项目结构：
 *   main.cpp          - 程序入口
 *   Game.h / Game.cpp - 游戏主逻辑（状态机、循环、渲染调度）
 *   Snake.h / Snake.cpp       - 蛇（数据结构：deque，移动/增长/碰撞）
 *   Food.h / Food.cpp         - 食物（随机生成/位置管理）
 *   Renderer.h / Renderer.cpp - 双缓冲控制台渲染
 *   Input.h / Input.cpp       - 键盘输入处理（Windows _kbhit / Linux termios）
 *   SaveSystem.h / SaveSystem.cpp - 存档/读档（文本格式序列化）
 *   common/Commom.h           - 公共定义（Point / Direction / 常量）
 *   CMakeLists.txt            - CMake 构建配置
 *
 * 控制方式：
 *   WASD / 方向键 - 移动
 *   P           - 暂停/继续
 *   O           - 保存存档
 *   L           - 读取存档
 *   Q           - 退出到菜单
 *
 * 编译（Windows MSYS2/MinGW）：
 *   cd Snakes && mkdir build && cd build && cmake .. -G "MinGW Makefiles" && make
 *
 * 直接编译（g++单条命令）：
 *   g++ -std=c++17 GreedySnake.cpp Game.cpp Snake.cpp Food.cpp Renderer.cpp Input.cpp SaveSystem.cpp -o snake.exe
 */

#include "Game.h"

int main()
{
  Game game;
  game.run();
  return 0;
}
