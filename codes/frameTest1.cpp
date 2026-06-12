#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

int main()
{
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML 3.0 - 极限帧率测试");

    // SFML 3.0: 使用系统字体
    sf::Font font;
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf"))
    {
        std::cerr << "Warning: Cannot load font" << std::endl;
    }

    sf::Text text(font);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});

    // FPS 计算
    sf::Clock fpsClock;
    int frameCount = 0;
    float currentFPS = 0;

    std::vector<float> fpsHistory;
    sf::Clock testTimer;

    std::cout << "=== SFML 3.0 极限帧率测试 ===" << std::endl;
    std::cout << "模式：无限制（已禁用 setFramerateLimit）" << std::endl;
    std::cout << "测试时长：15 秒" << std::endl;

    while (window.isOpen() && testTimer.getElapsedTime().asSeconds() < 15.0f)
    {
        // 事件处理
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // 简单渲染负载
        window.clear(sf::Color::Black);

        // 绘制一些图形增加负载
        sf::CircleShape circle(50);
        circle.setFillColor(sf::Color::Green);
        circle.setPosition({375, 275});
        window.draw(circle);

        // 显示 FPS
        text.setString("FPS: " + std::to_string((int)currentFPS) + " (极限模式)");
        window.draw(text);

        window.display();

        // FPS 统计
        frameCount++;
        if (fpsClock.getElapsedTime().asSeconds() >= 1.0f)
        {
            currentFPS = frameCount;
            fpsHistory.push_back(currentFPS);
            std::cout << "当前 FPS: " << (int)currentFPS << std::endl;
            frameCount = 0;
            fpsClock.restart();
        }
    }

    // 统计结果
    if (!fpsHistory.empty())
    {
        float avg = 0, maxFPS = 0, minFPS = 9999;
        for (float f : fpsHistory)
        {
            avg += f;
            if (f > maxFPS)
                maxFPS = f;
            if (f < minFPS)
                minFPS = f;
        }
        avg /= fpsHistory.size();

        std::cout << "\n=== 极限测试结果 ===" << std::endl;
        std::cout << "平均 FPS: " << (int)avg << std::endl;
        std::cout << "最大 FPS: " << (int)maxFPS << std::endl;
        std::cout << "最小 FPS: " << (int)minFPS << std::endl;

        getchar();
    }

    window.close();
    return 0;
}