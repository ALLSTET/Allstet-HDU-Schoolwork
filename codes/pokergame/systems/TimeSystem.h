#pragma once
#include <chrono>

class TimeSystem {
private:
    std::chrono::steady_clock::time_point last;
    int interval = 200; // ms

public:
    TimeSystem();
    bool shouldUpdate();
    void waitForNextFrame();
    void setInterval(int ms);
};
