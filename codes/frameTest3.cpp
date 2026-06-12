#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

void testSleepMethod() {
    std::cout << "\n========== 测试 1: Sleep 方法（平滑颜色） ==========" << std::endl;
    
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Sleep 方法 - 动态颜色");
    
    sf::Font font;
    font.openFromFile("C:/Windows/Fonts/arial.ttf");
    sf::Text text(font);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    
    const sf::Time FRAME_DURATION = sf::milliseconds(16);
    sf::Clock frameClock, fpsClock, testTimer;
    int frameCount = 0;
    float currentFPS = 0;
    std::vector<float> history;
    
    // 修正后的颜色渐变逻辑
    int r = 255, g = 0, b = 0;
    int stage = 0;  // 0:R->G, 1:G->B, 2:B->R
    
    while (window.isOpen() && testTimer.getElapsedTime().asSeconds() < 10.0f) {
        sf::Time elapsed = frameClock.restart();
        sf::Time sleepTime = FRAME_DURATION - elapsed;
        if (sleepTime.asMilliseconds() > 0) {
            sf::sleep(sleepTime);
        }
        
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }
        
        // ===== 修正后的颜色渐变 =====
        switch (stage) {
            case 0: // 红色 -> 绿色
                r--;
                g++;
                if (r == 0) stage = 1;
                break;
            case 1: // 绿色 -> 蓝色
                g--;
                b++;
                if (g == 0) stage = 2;
                break;
            case 2: // 蓝色 -> 红色
                b--;
                r++;
                if (b == 0) stage = 0;
                break;
        }
        
        window.clear(sf::Color::Black);
        
        sf::CircleShape circle(80);
        circle.setFillColor(sf::Color(r, g, b));
        circle.setPosition({400 - 80, 300 - 80});
        window.draw(circle);
        
        frameCount++;
        if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
            currentFPS = frameCount;
            history.push_back(currentFPS);
            std::cout << "Sleep FPS: " << (int)currentFPS << std::endl;
            frameCount = 0;
            fpsClock.restart();
        }
        
        text.setString("FPS: " + std::to_string((int)currentFPS) + " (Sleep)\nRGB: " + 
                      std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b));
        window.draw(text);
        window.display();
    }
    
    float avg = 0;
    for (float f : history) avg += f;
    avg /= history.size();
    std::cout << "Sleep 平均 FPS: " << (int)avg << std::endl;
    window.close();
}

void testFramerateLimit() {
    std::cout << "\n========== 测试 2: setFramerateLimit 方法（平滑颜色） ==========" << std::endl;
    
    sf::RenderWindow window(sf::VideoMode({800, 600}), "setFramerateLimit - 动态颜色");
    window.setFramerateLimit(60);
    
    sf::Font font;
    font.openFromFile("C:/Windows/Fonts/arial.ttf");
    sf::Text text(font);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    
    sf::Clock fpsClock, testTimer;
    int frameCount = 0;
    float currentFPS = 0;
    std::vector<float> history;
    
    // 修正后的颜色渐变逻辑
    int r = 255, g = 0, b = 0;
    int stage = 0;  // 0:R->G, 1:G->B, 2:B->R
    
    while (window.isOpen() && testTimer.getElapsedTime().asSeconds() < 10.0f) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }
        
        // ===== 修正后的颜色渐变 =====
        switch (stage) {
            case 0: // 红色 -> 绿色
                r--;
                g++;
                if (r == 0) stage = 1;
                break;
            case 1: // 绿色 -> 蓝色
                g--;
                b++;
                if (g == 0) stage = 2;
                break;
            case 2: // 蓝色 -> 红色
                b--;
                r++;
                if (b == 0) stage = 0;
                break;
        }
        
        window.clear(sf::Color::Black);
        
        sf::CircleShape circle(80);
        circle.setFillColor(sf::Color(r, g, b));
        circle.setPosition({400 - 80, 300 - 80});
        window.draw(circle);
        
        frameCount++;
        if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
            currentFPS = frameCount;
            history.push_back(currentFPS);
            std::cout << "SFML FPS: " << (int)currentFPS << std::endl;
            frameCount = 0;
            fpsClock.restart();
        }
        
        text.setString("FPS: " + std::to_string((int)currentFPS) + " (setFramerateLimit)\nRGB: " +
                      std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b));
        window.draw(text);
        window.display();
    }
    
    float avg = 0;
    for (float f : history) avg += f;
    avg /= history.size();
    std::cout << "setFramerateLimit 平均 FPS: " << (int)avg << std::endl;
    window.close();
}

int main() {
    std::cout << "=== SFML 帧率控制对比测试（平滑颜色渐变）===" << std::endl;
    std::cout << "圆形颜色会从红→绿→蓝→红 循环变化" << std::endl;
    std::cout << std::endl;
    
    testSleepMethod();
    testFramerateLimit();
    
    std::cout << "\n=== 测试完成 ===" << std::endl;
    return 0;
}