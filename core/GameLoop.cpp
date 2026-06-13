#include "GameLoop.h"
#include <thread>
#include <chrono>

GameLoop::GameLoop()
    : score(0)
{
  // 初始化帧数组
  for (int y = 0; y < GAME_HEIGHT; ++y)
    for (int x = 0; x < GAME_WIDTH; ++x)
      frame[y][x] = ' ';
}

// ==================== 程序入口 ====================
void GameLoop::run()
{
  while (!fsm.isExiting())
  {
    switch (fsm.getState())
    {
    case GameState::MENU:
      menuLoop();
      break;
    case GameState::RUNNING:
      gameLoop();
      break;
    case GameState::PAUSED:
      pauseLoop();
      break;
    case GameState::GAME_OVER:
      gameOverLoop();
      break;
    default:
      break;
    }
  }
}

// ==================== 菜单循环 ====================
void GameLoop::menuLoop()
{
  renderer.clearScreen();
  while (fsm.isMenu())
  {
    renderer.clear();
    renderer.drawMenu(
        saveSystem.autoSaveExists(),
        saveSystem.slotExists(1),
        saveSystem.slotExists(2),
        saveSystem.slotExists(3));
    renderer.flush();

    int key = inputSystem.waitForKey();
    switch (key)
    {
    case '1':
      newGame();
      fsm.transitionTo(GameState::RUNNING);
      return;
    case '2':
      loadWithSlot();
      if (fsm.isRunning())
        return;
      renderer.clearScreen();
      break;
    case '3':
    case 'q':
    case 'Q':
      fsm.transitionTo(GameState::EXIT);
      return;
    default:
      break;
    }
  }
}

// ==================== 游戏主循环（★ 标准 Game Loop 结构） ====================
void GameLoop::gameLoop()
{
  renderer.clearScreen();
  timeSystem.reset();
  inputSystem.setCurrentDirection(snake.getDirection());

  // ★ 立即渲染第一帧，避免黑屏等待
  renderScene();

  while (fsm.isRunning())
  {
    // ========== 第一步：processInput() ==========
    // 高频采集输入，清空缓冲区，应用最新合法输入覆盖策略
    processInput();

    // ========== 第二步：update() ==========
    // 按固定时间步长更新游戏逻辑
    if (timeSystem.shouldUpdate())
    {
      update();

      if (snake.isDead())
      {
        fsm.transitionTo(GameState::GAME_OVER);
        return;
      }

      // 同步当前方向到输入系统（用于下一帧的反向过滤）
      inputSystem.setCurrentDirection(snake.getDirection());
    }

    // ========== 第三步：render() ==========
    // 每帧渲染（输入响应即时可见）
    renderScene();

    // 短暂让出CPU（降低忙等待开销）
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
}

// ==================== 暂停循环 ====================
void GameLoop::pauseLoop()
{
  renderer.clearScreen();
  renderScene();
  renderer.drawPause();
  renderer.flush();

  while (fsm.isPaused())
  {
    int key = inputSystem.waitForKey();
    switch (key)
    {
    case 'p':
    case 'P':
      fsm.transitionTo(GameState::RUNNING);
      return;
    case 'o':
    case 'O':
      saveWithSlot();
      renderer.clearScreen();
      renderScene();
      renderer.drawPause();
      renderer.flush();
      break;
    case 'q':
    case 'Q':
      quitToMenu();
      return;
    default:
      break;
    }
  }
}

// ==================== 游戏结束循环 ====================
void GameLoop::gameOverLoop()
{
  renderer.clearScreen();
  renderScene();
  renderer.drawGameOver(score);
  renderer.flush();

  while (fsm.isGameOver())
  {
    int key = inputSystem.waitForKey();
    switch (key)
    {
    case 'r':
    case 'R':
      newGame();
      fsm.transitionTo(GameState::RUNNING);
      return;
    case 'q':
    case 'Q':
      fsm.transitionTo(GameState::MENU);
      return;
    default:
      break;
    }
  }
}

// ========== ★★★ 核心三步：processInput / update / render ★★★ ==========

void GameLoop::processInput()
{
  // ★ 调用 InputSystem::poll()，内部实现“最新合法输入覆盖策略”
  InputSystem::FrameInput fi = inputSystem.poll();

  // 处理方向输入（仅在 RUNNING 状态有效）
  if (fi.nextDirection != Direction::STOP)
  {
    applyDirection(fi.nextDirection);
  }

  // 处理功能键（与状态无关，立即响应）
  if (fi.pausePressed)
  {
    fsm.transitionTo(GameState::PAUSED);
  }
  if (fi.savePressed)
  {
    saveWithSlot();
  }
  if (fi.quitPressed)
  {
    quitToMenu();
  }
}

void GameLoop::update()
{
  snake.update();
  if (snake.isDead())
    return;

  // 吃食物检测
  if (food.isEaten(snake.getHead()))
  {
    snake.grow();
    score += 10;
    spawnFood();

    // 每吃5个食物动态加速
    if (score % 50 == 0)
    {
      timeSystem.increaseSpeed(20);
    }
  }
}

void GameLoop::renderScene()
{
  buildFrame();

  renderer.clear();
  renderer.drawGameFrame(frame, score, timeSystem.getFps());
  renderer.flush();
}

// ==================== 辅助方法 ====================

void GameLoop::buildFrame()
{
  // 清空画布
  for (int y = 0; y < GAME_HEIGHT; ++y)
    for (int x = 0; x < GAME_WIDTH; ++x)
      frame[y][x] = ' ';

  // 绘制蛇身
  const auto &body = snake.getBody();
  bool isHead = true;
  for (const auto &p : body)
  {
    if (p.x >= 0 && p.x < GAME_WIDTH && p.y >= 0 && p.y < GAME_HEIGHT)
    {
      frame[p.y][p.x] = isHead ? 'O' : 'o';
      isHead = false;
    }
  }

  // 绘制食物
  const Point &foodPos = food.getPosition();
  if (foodPos.x >= 0 && foodPos.x < GAME_WIDTH &&
      foodPos.y >= 0 && foodPos.y < GAME_HEIGHT)
  {
    frame[foodPos.y][foodPos.x] = '*';
  }
}

void GameLoop::applyDirection(Direction newDir)
{
  // 方向校验：检查是否与当前方向相反
  if (!isOpposite(newDir, snake.getDirection()))
  {
    snake.setDirection(newDir);
  }
}

void GameLoop::spawnFood()
{
  food.respawn(snake.getBody());
}

void GameLoop::newGame()
{
  snake.reset();
  score = 0;
  timeSystem.setSpeedMs(SPEED_SLOW);
  timeSystem.reset();
  spawnFood();
}

void GameLoop::quitToMenu()
{
  SaveData data;
  data.snake = snake.getBody();
  data.food = food.getPosition();
  data.direction = snake.getDirection();
  data.score = score;
  saveSystem.autoSave(data);

  fsm.transitionTo(GameState::MENU);
}

// ── 手动存档（O 键 → 选择槽位） ──
void GameLoop::saveWithSlot()
{
  renderer.clear();
  renderScene();
  renderer.drawSlotSelectSave();
  renderer.flush();

  int chosenSlot = -1;
  while (chosenSlot < 0)
  {
    int key = inputSystem.waitForKey();
    switch (key)
    {
    case '1':
      chosenSlot = 1;
      break;
    case '2':
      chosenSlot = 2;
      break;
    case '3':
      chosenSlot = 3;
      break;
    case 27:
    case 'q':
    case 'Q':
      return; // ESC/Q 取消
    default:
      break;
    }
  }

  SaveData data;
  data.snake = snake.getBody();
  data.food = food.getPosition();
  data.direction = snake.getDirection();
  data.score = score;

  saveSystem.saveToSlot(chosenSlot, data);

  // 保存成功提示
  renderer.clear();
  renderScene();
  renderer.drawSaveSuccess();
  renderer.flush();
  std::this_thread::sleep_for(std::chrono::milliseconds(600));
}

// ── 加载存档（菜单选项 2 → 选择槽位） ──
void GameLoop::loadWithSlot()
{
  renderer.clearScreen();
  renderer.clear();
  renderer.drawSlotSelectLoad();
  renderer.flush();

  int chosenSlot = -1;
  while (chosenSlot < 0)
  {
    int key = inputSystem.waitForKey();
    switch (key)
    {
    case '0':
      chosenSlot = 0;
      break;
    case '1':
      chosenSlot = 1;
      break;
    case '2':
      chosenSlot = 2;
      break;
    case '3':
      chosenSlot = 3;
      break;
    case 27:
    case 'q':
    case 'Q':
      return;
    default:
      break;
    }
  }

  SaveData data;
  bool ok = (chosenSlot == 0)
                ? saveSystem.autoLoad(data)
                : saveSystem.loadFromSlot(chosenSlot, data);

  if (!ok)
  {
    // 存档损坏/不存在（鲁棒性处理）
    renderer.clearScreen();
    renderer.clear();
    renderer.drawMenu(
        saveSystem.autoSaveExists(),
        saveSystem.slotExists(1),
        saveSystem.slotExists(2),
        saveSystem.slotExists(3));
    renderer.drawLoadFailed();
    renderer.flush();
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    renderer.clearScreen();
    return;
  }

  // 恢复存档数据
  snake.reset();
  snake.setBody(data.snake);
  snake.setDirection(data.direction);
  food.setPosition(data.food);
  score = data.score;
  timeSystem.setSpeedMs(SPEED_SLOW); // 存档不保存速度，统一从慢速开始

  renderer.clear();
  renderer.drawLoadSuccess();
  renderer.flush();
  std::this_thread::sleep_for(std::chrono::milliseconds(800));
  fsm.transitionTo(GameState::RUNNING);
  renderer.clearScreen();
}
