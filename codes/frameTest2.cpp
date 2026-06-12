#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <chrono>

void testSleepMethod() {
    std::cout << "\n========== 测试 1: Sleep 方法 ==========" << std::endl;
    
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Sleep 方法 - 60 FPS");
    
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
    
    while (window.isOpen() && testTimer.getElapsedTime().asSeconds() < 10.0f) {
        sf::Time elapsed = frameClock.restart();
        sf::Time sleepTime = FRAME_DURATION - elapsed;
        if (sleepTime.asMilliseconds() > 0) {
            sf::sleep(sleepTime);
        }
        
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }
        
        window.clear();
        sf::CircleShape circle(50);
        circle.setFillColor(sf::Color::Blue);
        circle.setPosition({375, 275});
        window.draw(circle);
        
        frameCount++;
        if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
            currentFPS = frameCount;
            history.push_back(currentFPS);
            std::cout << "Sleep FPS: " << (int)currentFPS << std::endl;
            frameCount = 0;
            fpsClock.restart();
        }
        
        text.setString("FPS: " + std::to_string((int)currentFPS) + " (Sleep)");
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
    std::cout << "\n========== 测试 2: setFramerateLimit 方法 ==========" << std::endl;
    
    sf::RenderWindow window(sf::VideoMode({800, 600}), "setFramerateLimit - 60 FPS");
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
    
    while (window.isOpen() && testTimer.getElapsedTime().asSeconds() < 10.0f) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
        }
        
        window.clear();
        sf::CircleShape circle(50);
        circle.setFillColor(sf::Color::Red);
        circle.setPosition({375, 275});
        window.draw(circle);
        
        frameCount++;
        if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
            currentFPS = frameCount;
            history.push_back(currentFPS);
            std::cout << "SFML FPS: " << (int)currentFPS << std::endl;
            frameCount = 0;
            fpsClock.restart();
        }
        
        text.setString("FPS: " + std::to_string((int)currentFPS) + " (setFramerateLimit)");
        window.draw(text);
        window.display();
    }
    
    float avg = 0;
    for (float f : history) avg += f;
    avg /= history.size();
    std::cout << "setFramerateLimit 平均 FPS: " << (int)avg << std::endl;
    window.close();
    
    sf::sleep(sf::seconds(2));
}

int main() {
    std::cout << "=== SFML 帧率控制对比测试 ===" << std::endl;
    
    testSleepMethod();
    testFramerateLimit();
    
    std::cout << "\n=== 测试完成 ===" << std::endl;
    return 0;
}