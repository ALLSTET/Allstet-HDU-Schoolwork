#include <SFML/Graphics.hpp>
#include <iostream>

// 明确使用sf命名空间
using namespace sf;

int main()
{
    // 方法1：使用VideoMode的静态方法创建（最可靠）
    VideoMode mode = VideoMode::getDesktopMode();
    // 或者使用指定的分辨率
    // VideoMode mode(800, 600);

    // 创建窗口 - 使用最基础的构造函数
    RenderWindow window(mode, "SFML 3.0.2 Demo", Style::Default);

    // 设置窗口位置（可选）
    window.setPosition(Vector2i(100, 100));

    // 创建一些简单的图形
    CircleShape circle(50.f);
    circle.setFillColor(Color::Green);
    circle.setPosition(Vector2f(100.f, 100.f));

    RectangleShape rectangle(Vector2f(120.f, 60.f));
    rectangle.setFillColor(Color::Blue);
    rectangle.setPosition(Vector2f(300.f, 150.f));

    // 创建一个移动的小方块
    RectangleShape player(Vector2f(40.f, 40.f));
    player.setFillColor(Color::Red);
    player.setPosition(Vector2f(400.f, 500.f));

    // 控制变量
    float speed = 5.f;

    std::cout << "程序启动成功！" << std::endl;
    std::cout << "使用方向键移动红色方块" << std::endl;
    std::cout << "按ESC退出程序" << std::endl;

    // 主循环
    while (window.isOpen())
    {
        // 正确的SFML 3.0.2事件处理方式
        while (auto optionalEvent = window.pollEvent())
        {
            // 检查是否有事件
            if (optionalEvent.has_value())
            {
                Event event = optionalEvent.value();

                // 使用getIf检查事件类型
                if (auto *closed = event.getIf<Event::Closed>())
                {
                    window.close();
                }
                else if (auto *keyPressed = event.getIf<Event::KeyPressed>())
                {
                    if (keyPressed->code == Keyboard::Key::Escape)
                    {
                        window.close();
                    }
                }
                else if (auto *resized = event.getIf<Event::Resized>())
                {
                    // 窗口大小改变时调整视口
                    FloatRect visibleArea(Vector2f(0, 0), Vector2f(resized->size.x, resized->size.y));
                    window.setView(View(visibleArea));
                }
            }
        }

        // 键盘控制
        if (Keyboard::isKeyPressed(Keyboard::Key::Left))
        {
            player.move(Vector2f(-speed, 0.f));
        }
        if (Keyboard::isKeyPressed(Keyboard::Key::Right))
        {
            player.move(Vector2f(speed, 0.f));
        }
        if (Keyboard::isKeyPressed(Keyboard::Key::Up))
        {
            player.move(Vector2f(0.f, -speed));
        }
        if (Keyboard::isKeyPressed(Keyboard::Key::Down))
        {
            player.move(Vector2f(0.f, speed));
        }

        // 边界检查
        Vector2f pos = player.getPosition();
        Vector2f size = player.getSize();

        // 获取窗口大小
        Vector2u windowSize = window.getSize();

        if (pos.x < 0)
            pos.x = 0;
        if (pos.y < 0)
            pos.y = 0;
        if (pos.x + size.x > windowSize.x)
            pos.x = windowSize.x - size.x;
        if (pos.y + size.y > windowSize.y)
            pos.y = windowSize.y - size.y;

        player.setPosition(pos);

        // 清空窗口
        window.clear(Color(50, 50, 50));

        // 绘制图形
        window.draw(circle);
        window.draw(rectangle);
        window.draw(player);

        // 显示内容
        window.display();
    }

    std::cout << "程序正常退出" << std::endl;
    return 0;
}