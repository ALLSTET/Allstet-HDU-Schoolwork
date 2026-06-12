#include "SaveSystem.h"
#include <fstream>
#include <deque>
#include <iostream>

bool SaveSystem::saveGame(const std::string& filename, const StateMachine& fsm) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "无法打开存档文件进行写入: " << filename << std::endl;
        return false;
    }

    try {
        // 保存游戏状态
        GameState state = fsm.getState();
        file.write(reinterpret_cast<const char*>(&state), sizeof(GameState));

        // 保存蛇的状态
        const Snake& snake = fsm.getSnake();
        Direction dir = snake.getDirection();
        file.write(reinterpret_cast<const char*>(&dir), sizeof(Direction));

        // 保存蛇的身体长度
        const auto& body = snake.getBody();
        size_t bodySize = body.size();
        file.write(reinterpret_cast<const char*>(&bodySize), sizeof(size_t));

        // 保存蛇的身体坐标
        for (const auto& segment : body) {
            file.write(reinterpret_cast<const char*>(&segment.x), sizeof(int));
            file.write(reinterpret_cast<const char*>(&segment.y), sizeof(int));
        }

        file.close();
        std::cout << "游戏已保存到: " << filename << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "保存游戏时出错: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

bool SaveSystem::loadGame(const std::string& filename, StateMachine& fsm) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "无法打开存档文件进行读取: " << filename << std::endl;
        return false;
    }

    try {
        // 读取游戏状态
        GameState state;
        file.read(reinterpret_cast<char*>(&state), sizeof(GameState));

        // 读取蛇的方向
        Direction dir;
        file.read(reinterpret_cast<char*>(&dir), sizeof(Direction));

        // 读取蛇的身体长度
        size_t bodySize;
        file.read(reinterpret_cast<char*>(&bodySize), sizeof(size_t));

        // 读取蛇的身体坐标
        std::deque<Point> body;
        for (size_t i = 0; i < bodySize; ++i) {
            int x, y;
            file.read(reinterpret_cast<char*>(&x), sizeof(int));
            file.read(reinterpret_cast<char*>(&y), sizeof(int));
            body.push_back({x, y});
        }

        file.close();

        // 恢复游戏状态
        fsm.changeState(state);
        Snake& snake = fsm.getSnake();
        snake.setDirection(dir);
        snake.setBody(body);

        std::cout << "游戏已从 " << filename << " 加载" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "加载游戏时出错: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

bool SaveSystem::saveExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}
