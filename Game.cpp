#include "Game.h"
#include <chrono>
#include <thread>

Game::Game()
    : status(GameStatus::MENU), score(0), speed(200) // 初始速度 200ms/帧
{
  // 初始化 frame 数组
  for (int y = 0; y < GAME_HEIGHT; ++y)
    for (int x = 0; x < GAME_WIDTH; ++x)
      frame[y][x] = ' ';
}

void Game::run()
{
  while (status != GameStatus::EXIT)
  {
    switch (status)
    {
    case GameStatus::MENU:
      menuLoop();
      break;
    case GameStatus::PLAYING:
      gameLoop();
      break;
    case GameStatus::PAUSED:
      pauseLoop();
      break;
    case GameStatus::GAME_OVER:
      gameOverLoop();
      break;
    default:
      break;
    }
  }
}

// ===================== 菜单循环 =====================
void Game::menuLoop()
{
  renderer.clearScreen(); // ★ 画面切换时硬清屏一次
  while (status == GameStatus::MENU)
  {
    renderer.clear();
    renderer.drawMenu(
        saveSystem.autoSaveExists(),
        saveSystem.slotExists(1),
        saveSystem.slotExists(2),
        saveSystem.slotExists(3));
    renderer.flush();

    InputAction action;
    do
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      action = input.poll(true, false);
    } while (action == InputAction::NONE);

    switch (action)
    {
    case InputAction::NEW_GAME:
      newGame();
      status = GameStatus::PLAYING;
      return;
    case InputAction::LOAD_GAME:
      loadWithSlot(); // ★ 弹出槽位选择 → 加载存档
      if (status == GameStatus::PLAYING)
        return;
      // 加载失败或取消 → 留在菜单，重新绘制
      renderer.clearScreen();
      break;
    case InputAction::QUIT:
      status = GameStatus::EXIT;
      return;
    default:
      break;
    }
  }
}

// ===================== 游戏主循环 =====================
void Game::gameLoop()
{
  renderer.clearScreen(); // ★ 从菜单/暂停进入游戏时硬清屏
  render();               // ★ 立即渲染第一帧，避免黑屏等待
  auto lastUpdate = std::chrono::steady_clock::now();

  while (status == GameStatus::PLAYING)
  {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate).count();

    // 处理输入（非阻塞）
    InputAction action = input.poll();
    handleInput(action);

    // 按帧率更新
    if (elapsed >= speed)
    {
      lastUpdate = now;
      update();

      if (snake.isDead())
      {
        status = GameStatus::GAME_OVER;
        return;
      }
    }

    // 渲染（按帧率渲染以保持响应）
    if (elapsed >= speed)
    {
      render();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

// ===================== 暂停循环 =====================
void Game::pauseLoop()
{
  renderer.clearScreen(); // ★ 进入暂停时硬清屏（画面高度会变化）

  // 先渲染暂停画面
  render();
  renderer.drawPause();
  renderer.flush();

  while (status == GameStatus::PAUSED)
  {
    InputAction action;
    do
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      action = input.poll();
    } while (action == InputAction::NONE);

    switch (action)
    {
    case InputAction::PAUSE:
      status = GameStatus::PLAYING;
      return;
    case InputAction::SAVE: // O 键 → 选择槽位保存
      saveWithSlot();
      // 保存完成后恢复暂停画面
      renderer.clearScreen();
      render();
      renderer.drawPause();
      renderer.flush();
      break;
    case InputAction::QUIT: // Q 键 → 自动保存并返回菜单
      quitToMenu();
      return;
    default:
      break;
    }
  }
}

// ===================== 游戏结束循环 =====================
void Game::gameOverLoop()
{
  renderer.clearScreen(); // ★ 进入结束画面时硬清屏
  render();
  renderer.drawGameOver(score);
  renderer.flush();

  while (status == GameStatus::GAME_OVER)
  {
    InputAction action;
    do
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      action = input.poll(false, true);
    } while (action == InputAction::NONE);

    switch (action)
    {
    case InputAction::RESTART:
      newGame();
      status = GameStatus::PLAYING;
      return;
    case InputAction::QUIT:
      status = GameStatus::MENU;
      return;
    default:
      break;
    }
  }
}

// ===================== 输入处理 =====================
void Game::handleInput(InputAction action)
{
  switch (action)
  {
  case InputAction::MOVE_UP:
    snake.setDirection(Direction::UP);
    break;
  case InputAction::MOVE_DOWN:
    snake.setDirection(Direction::DOWN);
    break;
  case InputAction::MOVE_LEFT:
    snake.setDirection(Direction::LEFT);
    break;
  case InputAction::MOVE_RIGHT:
    snake.setDirection(Direction::RIGHT);
    break;
  case InputAction::PAUSE:
    status = GameStatus::PAUSED;
    break;
  case InputAction::SAVE:
    saveWithSlot();
    break; // O = 选择槽位保存
  case InputAction::QUIT:
    quitToMenu();
    break; // Q = 自动保存返回菜单
  default:
    break;
  }
}

// ===================== 游戏逻辑更新 =====================
void Game::update()
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

    // 每吃5个食物加速
    if (score % 50 == 0 && speed > 60)
    {
      speed -= 20;
    }
  }
}

// ===================== 渲染（双缓冲） =====================
void Game::buildFrame()
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

void Game::render()
{
  buildFrame();

  renderer.clear();

  // 上边框
  renderer.buffer << "+";
  for (int i = 0; i < GAME_WIDTH; ++i)
    renderer.buffer << "-";
  renderer.buffer << "+\n";

  // 游戏区域
  for (int y = 0; y < GAME_HEIGHT; ++y)
  {
    renderer.buffer << "|";
    for (int x = 0; x < GAME_WIDTH; ++x)
    {
      renderer.buffer << frame[y][x];
    }
    renderer.buffer << "|\n";
  }

  // 下边框
  renderer.buffer << "+";
  for (int i = 0; i < GAME_WIDTH; ++i)
    renderer.buffer << "-";
  renderer.buffer << "+\n";

  // 分数与操作提示
  renderer.buffer << "\nScore: " << score;
  renderer.buffer << "   Speed: " << (1000 / speed) << "fps\n";
  renderer.buffer << "WASD/↑↓←→:移动 | P:暂停 | O:存档到槽位 | Q:自动保存并返回菜单\n";

  renderer.flush();
}

// ===================== 游戏管理 =====================
void Game::newGame()
{
  snake.reset();
  score = 0;
  speed = 200;
  spawnFood();
}

void Game::spawnFood()
{
  food.respawn(snake.getBody());
}

// ===================== 退出到菜单（自动保存） =====================
void Game::quitToMenu()
{
  // 自动保存到临时槽位
  GameState state;
  state.snake = snake.getBody();
  state.food = food.getPosition();
  state.direction = snake.getDirection();
  state.score = score;
  saveSystem.autoSave(state);

  status = GameStatus::MENU;
}

// ===================== 手动存档（O 键 → 选择槽位） =====================
void Game::saveWithSlot()
{
  // 显示槽位选择界面
  renderer.clear();
  render();
  renderer.drawSlotSelectSave();
  renderer.flush();

  int chosenSlot = -1;
  while (chosenSlot < 0)
  {
    InputAction act;
    do
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      act = input.pollSlot();
    } while (act == InputAction::NONE);

    switch (act)
    {
    case InputAction::SLOT_1:
      chosenSlot = 1;
      break;
    case InputAction::SLOT_2:
      chosenSlot = 2;
      break;
    case InputAction::SLOT_3:
      chosenSlot = 3;
      break;
    case InputAction::CANCEL:
      return; // 取消
    default:
      break;
    }
  }

  GameState state;
  state.snake = snake.getBody();
  state.food = food.getPosition();
  state.direction = snake.getDirection();
  state.score = score;

  saveSystem.saveToSlot(chosenSlot, state);

  // 短暂显示保存成功
  renderer.clear();
  render();
  renderer.drawSaveSuccess();
  renderer.flush();
  std::this_thread::sleep_for(std::chrono::milliseconds(600));
}

// ===================== 加载存档（菜单选项 2 → 选择槽位） =====================
void Game::loadWithSlot()
{
  renderer.clearScreen();
  renderer.clear();
  renderer.drawSlotSelectLoad();
  renderer.flush();

  int chosenSlot = -1;
  while (chosenSlot < 0)
  {
    InputAction act;
    do
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
      act = input.pollSlot();
    } while (act == InputAction::NONE);

    switch (act)
    {
    case InputAction::SLOT_0:
      chosenSlot = 0;
      break;
    case InputAction::SLOT_1:
      chosenSlot = 1;
      break;
    case InputAction::SLOT_2:
      chosenSlot = 2;
      break;
    case InputAction::SLOT_3:
      chosenSlot = 3;
      break;
    case InputAction::CANCEL:
      return; // 取消 → 回到菜单
    default:
      break;
    }
  }

  GameState state;
  bool ok = (chosenSlot == 0)
                ? saveSystem.autoLoad(state)
                : saveSystem.loadFromSlot(chosenSlot, state);

  if (!ok)
  {
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

  // ★ 先重置蛇的全部内部状态（dead/dir/growNextMove），再恢复存档数据
  //    否则上一局的 dead=true 或反方向 dir 会导致读档后瞬死
  snake.reset();
  snake.setBody(state.snake);
  snake.setDirection(state.direction);
  food.setPosition(state.food);
  score = state.score;

  renderer.clear();
  renderer.drawLoadSuccess();
  renderer.flush();
  std::this_thread::sleep_for(std::chrono::milliseconds(800));
  status = GameStatus::PLAYING;
  renderer.clearScreen();
}
