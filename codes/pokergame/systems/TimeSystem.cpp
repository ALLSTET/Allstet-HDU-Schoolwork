#include "TimeSystem.h"
#include <thread>

TimeSystem::TimeSystem() : interval(200) {
    last = std::chrono::steady_clock::now();
}

bool TimeSystem::shouldUpdate() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();
    
    if (elapsed >= interval) {
        last = now;
        return true;
    }
    return false;
}

void TimeSystem::waitForNextFrame() {
    // 简单的帧率控制，等待剩余时间
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();
    
    if (elapsed < interval) {
        std::this_thread::sleep_for(std::chrono::milliseconds(interval - elapsed));
    }
}

void TimeSystem::setInterval(int ms) {
    interval = ms;
}
