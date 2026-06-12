#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>

void testSleepMethod()
{
  std::cout << "\n========== 测试 1: Sleep 方法 ==========" << std::endl;

  sf::RenderWindow window(sf::VideoMode({800, 600}), "Sleep 方法 - 60 FPS");

  sf::Font font;
  font.openFromFile("C:/Windows/Fonts/arial.ttf");
  sf::Text text(font);
  text.setCharacterSize(24);
  text.setFillColor(sf::Color::White);
  text.setPosition({10, 10});

  const sf::Time FRAME_DURATION = sf::milliseconds(16);
  sf::Clock frameClock, fpsClock, testTimer, colorClock;
  int frameCount = 0;
  float currentFPS = 0;
  std::vector<float> history;

  while (window.isOpen() && testTimer.getElapsedTime().asSeconds() < 10.0f)
  {
    sf::Time elapsed = frameClock.restart();
    sf::Time sleepTime = FRAME_DURATION - elapsed;
    if (sleepTime.asMilliseconds() > 0)
    {
      sf::sleep(sleepTime);
    }

    while (const std::optional event = window.pollEvent())
    {
      if (event->is<sf::Event::Closed>())
        window.close();
    }

    // 使用正弦波生成平滑变化的颜色
    float time = colorClock.getElapsedTime().asSeconds() * 2.0f; // 速度系数

    // RGB 各自用正弦波，相位相差 120 度
    int r = static_cast<int>((std::sin(time) + 1) * 127.5);
    int g = static_cast<int>((std::sin(time + 2.094) + 1) * 127.5);
    int b = static_cast<int>((std::sin(time + 4.188) + 1) * 127.5);

    window.clear(sf::Color::Black);

    sf::CircleShape circle(80);
    circle.setFillColor(sf::Color(r, g, b));
    circle.setPosition({400 - 80, 300 - 80});
    window.draw(circle);

    frameCount++;
    if (fpsClock.getElapsedTime().asSeconds() >= 1.0f)
    {
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
  for (float f : history)
    avg += f;
  avg /= history.size();
  std::cout << "Sleep 平均 FPS: " << (int)avg << std::endl;
  window.close();
}

void testFramerateLimit()
{
  std::cout << "\n========== 测试 2: setFramerateLimit 方法 ==========" << std::endl;

  sf::RenderWindow window(sf::VideoMode({800, 600}), "setFramerateLimit - 60 FPS");
  window.setFramerateLimit(60);

  sf::Font font;
  font.openFromFile("C:/Windows/Fonts/arial.ttf");
  sf::Text text(font);
  text.setCharacterSize(24);
  text.setFillColor(sf::Color::White);
  text.setPosition({10, 10});

  sf::Clock fpsClock, testTimer, colorClock;
  int frameCount = 0;
  float currentFPS = 0;
  std::vector<float> history;

  while (window.isOpen() && testTimer.getElapsedTime().asSeconds() < 10.0f)
  {
    while (const std::optional event = window.pollEvent())
    {
      if (event->is<sf::Event::Closed>())
        window.close();
    }

    // 使用正弦波生成平滑变化的颜色
    float time = colorClock.getElapsedTime().asSeconds() * 2.0f;

    int r = static_cast<int>((std::sin(time) + 1) * 127.5);
    int g = static_cast<int>((std::sin(time + 2.094) + 1) * 127.5);
    int b = static_cast<int>((std::sin(time + 4.188) + 1) * 127.5);

    window.clear(sf::Color::Black);

    sf::CircleShape circle(80);
    circle.setFillColor(sf::Color(r, g, b));
    circle.setPosition({400 - 80, 300 - 80});
    window.draw(circle);

    frameCount++;
    if (fpsClock.getElapsedTime().asSeconds() >= 1.0f)
    {
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
  for (float f : history)
    avg += f;
  avg /= history.size();
  std::cout << "setFramerateLimit 平均 FPS: " << (int)avg << std::endl;
  window.close();
}

int main()
{
  std::cout << "=== SFML 帧率控制对比测试 ===" << std::endl;
  std::cout << "圆形颜色会连续平滑变化（红→绿→蓝→红）" << std::endl;
  std::cout << std::endl;

  testSleepMethod();
  testFramerateLimit();

  std::cout << "\n=== 测试完成 ===" << std::endl;
  getchar();
  return 0;
}