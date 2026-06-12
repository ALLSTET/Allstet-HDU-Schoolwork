#include "RenderSystem.h"
#include "InputSystem.h"
#include "../core/StateMachine.h"
#include "../gameplay/Snake.h"
#include <windows.h>

static int lastState = -1;
static bool stateChanged = false;
static bool ansiEnabled = []() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(hOut, &mode)) {
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, mode);
        return true;
    }
    return false;
}();

void RenderSystem::render(const StateMachine& fsm, const InputSystem& input) {
    buffer.str("");
    buffer.clear();
    
    GameState state = fsm.getState();
    
    // 检测状态改变
    stateChanged = ((int)lastState != (int)state);
    if (stateChanged) {
        lastState = (int)state;
    }
    
    if (state == GameState::MENU) {
        int choice = input.getMenuChoice();
        buffer << "\n\n";
        buffer << "    ========================================\n";
        buffer << "              SNAKE GAME\n";
        buffer << "    ========================================\n\n";
        buffer << "              " << (choice == 1 ? "[1]" : " 1 ") << " - 开新档\n";
        buffer << "              " << (choice == 2 ? "[2]" : " 2 ") << " - 读档\n";
        buffer << "              Q - 退出\n\n";
        if (choice > 0) {
            buffer << "              按Enter确认...\n";
        }
        buffer << "    ========================================\n";
    } 
    else if (state == GameState::RUNNING || state == GameState::PAUSED) {
        buffer << "\n";
        buffer << "    SNAKE GAME  [" << (state == GameState::PAUSED ? "PAUSED" : "RUNNING") << "]\n";
        buffer << "    ";
        for (int i = 0; i < GAME_WIDTH * 2; i++) buffer << "=";
        buffer << "\n";
        
        const Snake& snake = fsm.getSnake();
        const auto& body = snake.getBody();
        const Point foodPos = fsm.getFood().getPosition();
        const char* headColor = "\x1b[38;5;196m";   // 红色（蛇头）
        const char* bodyColor = "\x1b[38;5;46m";    // 绿色（蛇身）
        const char* foodColor  = "\x1b[38;5;39m";   // 蓝色（食物）
        const char* resetColor = "\x1b[0m";
        
        // 绘制游戏地图
        for (int y = 0; y < GAME_HEIGHT; y++) {
            buffer << "    |";
            for (int x = 0; x < GAME_WIDTH; x++) {
                bool isSnake = false;
                // 检查该位置是否有蛇
                for (size_t idx = 0; idx < body.size(); ++idx) {
                    if (body[idx].x == x && body[idx].y == y) {
                        if (idx == 0) {
                            buffer << headColor << "■ " << resetColor; // 蛇头红色实心
                        } else {
                            buffer << bodyColor << "□ " << resetColor; // 蛇身绿色空心
                        }
                        isSnake = true;
                        break;
                    }
                }
                if (!isSnake) {
                    if (foodPos.x == x && foodPos.y == y) {
                        buffer << foodColor << "■ " << resetColor;
                    } else {
                        buffer << "□ ";
                    }
                }
            }
            buffer << "|\n";
            buffer << "    |";
            for (int x = 0; x < GAME_WIDTH; x++) {
                buffer << "  ";
            }
            buffer << "|\n";
        }
        
        buffer << "    ";
        for (int i = 0; i < GAME_WIDTH * 2; i++) buffer << "=";
        buffer << "\n";
        
        buffer << "\n    蛇长: " << body.size() << "  速度等级: " << fsm.getSpeedLevel() << "\n";
        buffer << "    W/↑:上  S/↓:下  A/←:左  D/→:右  P:暂停  Q:退出  F5:保存/读档\n";
        
        if (state == GameState::PAUSED) {
            buffer << "\n    [PAUSED] P-Resume  Q-Quit\n";
        }
    } 
    else if (state == GameState::GAME_OVER) {
        buffer << "\n\n";
        buffer << "    ========================================\n";
        buffer << "             GAME OVER\n";
        buffer << "    ========================================\n\n";
        buffer << "         Press Q to return to menu\n\n";
        buffer << "    ========================================\n";
    }
}

void RenderSystem::flush() {
    // 状态改变时立即清屏
    if (firstFrame || stateChanged) {
        system("cls");
        firstFrame = false;
        stateChanged = false;
    } else {
        // 其他时间只设置光标位置
        HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD coord = {0, 0};
        SetConsoleCursorPosition(stdout_handle, coord);
    }
    
    std::cout << buffer.str();
    std::cout.flush();
}
