#include "StartScreen.h"
#include <iostream>
#include <cmath>

StartScreen::StartScreen(sf::RenderWindow &window)
    : window(window), selectedDifficulty(NORMAL)
{
    loadFont();
    // 尝试加载背景图
    const char *bgPaths[] = {
        "C:\\Users\\lenovo\\Desktop\\Program\\C\\codes\\pokergame\\assets\\TYPE\\Desktop1.png"};
    for (const auto &p : bgPaths)
    {
        if (bgTexture.loadFromFile(p))
        {
            bgSprite.emplace(bgTexture);
            auto sz = bgTexture.getSize();
            if (sz.x > 0 && sz.y > 0)
                bgSprite->setScale(sf::Vector2f(1200.f / sz.x, 800.f / sz.y));
            break;
        }
    }
}

void StartScreen::loadFont()
{
    if (!font.openFromFile("C:\\Windows\\Fonts\\simhei.ttf"))
    {
        if (!font.openFromFile("C:\\Windows\\Fonts\\msyh.ttc"))
        {
            if (!font.openFromFile("C:\\Windows\\Fonts\\arial.ttf"))
            {
                std::cerr << "Warning: Could not load font" << std::endl;
            }
        }
    }
}

static void drawRoundedRect(sf::RenderWindow &win, float x, float y, float w, float h,
                            sf::Color fill)
{
    float r = h * 0.35f;
    sf::RectangleShape body(sf::Vector2f(w - 2.f * r, h));
    body.setPosition(sf::Vector2f(x + r, y));
    body.setFillColor(fill);
    win.draw(body);
    sf::CircleShape cl(r), cr(r);
    cl.setPosition(sf::Vector2f(x, y));
    cr.setPosition(sf::Vector2f(x + w - 2.f * r, y));
    cl.setFillColor(fill);
    cr.setFillColor(fill);
    win.draw(cl);
    win.draw(cr);
}

void StartScreen::draw()
{
    // 背景
    if (bgSprite.has_value())
    {
        window.draw(*bgSprite);
        // 暗色遮罩
        sf::RectangleShape overlay(sf::Vector2f(1200.f, 800.f));
        overlay.setFillColor(sf::Color(0, 0, 0, 120));
        window.draw(overlay);
    }
    else
    {
        window.clear(sf::Color(15, 30, 15));
    }

    float cx = 600.f;

    // ── 标题 ──
    {
        sf::Text title(font, "DOU NIU", 72);
        title.setFillColor(sf::Color(255, 215, 0));
        sf::FloatRect tb = title.getLocalBounds();
        title.setPosition(sf::Vector2f(cx - tb.size.x / 2.f, 90.f));
        window.draw(title);

        sf::Text sub(font, "Bull Fight", 28);
        sub.setFillColor(sf::Color(255, 200, 100));
        sf::FloatRect sb = sub.getLocalBounds();
        sub.setPosition(sf::Vector2f(cx - sb.size.x / 2.f, 170.f));
        window.draw(sub);
    }

    // ── 装饰线 ──
    {
        sf::RectangleShape line(sf::Vector2f(300.f, 2.f));
        line.setPosition(sf::Vector2f(cx - 150.f, 215.f));
        line.setFillColor(sf::Color(218, 165, 32, 150));
        window.draw(line);
    }

    // ── 描述 ──
    {
        sf::Text desc(font, "1 Player  vs  3 AI  |  5 Cards Each  |  Call & Rob Banker", 18);
        desc.setFillColor(sf::Color(200, 200, 200));
        sf::FloatRect db = desc.getLocalBounds();
        desc.setPosition(sf::Vector2f(cx - db.size.x / 2.f, 240.f));
        window.draw(desc);
    }

    // ── 难度按钮 ──
    drawDifficultyButtons();

    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    // ── START 按钮 ──
    {
        bool hover = startButtonRect.contains(mousePos);
        sf::Color fill = hover ? sf::Color(200, 45, 45) : sf::Color(160, 35, 35);
        float bx = cx - 120.f, by = 500.f, bw = 240.f, bh = 56.f;

        if (hover)
        {
            sf::Color glow = sf::Color(255, 200, 50, 60);
            drawRoundedRect(window, bx - 4.f, by - 4.f, bw + 8.f, bh + 8.f, glow);
        }
        drawRoundedRect(window, bx, by, bw, bh, fill);

        sf::Text btnText(font, "START GAME", 24);
        btnText.setFillColor(sf::Color(255, 215, 0));
        sf::FloatRect bb = btnText.getLocalBounds();
        btnText.setPosition(sf::Vector2f(bx + (bw - bb.size.x) / 2.f, by + (bh - bb.size.y) / 2.f - bb.position.y));
        window.draw(btnText);

        startButtonRect = sf::FloatRect(sf::Vector2f(bx, by), sf::Vector2f(bw, bh));
    }

    // ── QUIT 按钮 ──
    {
        bool hover = quitButtonRect.contains(mousePos);
        sf::Color fill = hover ? sf::Color(80, 80, 90) : sf::Color(50, 50, 55);
        float bx = cx - 90.f, by = 580.f, bw = 180.f, bh = 44.f;

        drawRoundedRect(window, bx, by, bw, bh, fill);

        sf::Text btnText(font, "QUIT", 20);
        btnText.setFillColor(sf::Color(200, 200, 200));
        sf::FloatRect bb = btnText.getLocalBounds();
        btnText.setPosition(sf::Vector2f(bx + (bw - bb.size.x) / 2.f, by + (bh - bb.size.y) / 2.f - bb.position.y));
        window.draw(btnText);

        quitButtonRect = sf::FloatRect(sf::Vector2f(bx, by), sf::Vector2f(bw, bh));
    }

    // ── 底部版本信息 ──
    {
        sf::Text ver(font, "v1.0  |  C++ / SFML 3  |  2025", 12);
        ver.setFillColor(sf::Color(150, 150, 150));
        ver.setPosition(sf::Vector2f(cx - 80.f, 760.f));
        window.draw(ver);
    }
}

void StartScreen::drawDifficultyButtons()
{
    float cx = 600.f;
    float baseY = 320.f;

    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    sf::Text label(font, "Difficulty:", 16);
    label.setFillColor(sf::Color(200, 200, 200));
    label.setPosition(sf::Vector2f(cx - 210.f, baseY - 8.f));
    window.draw(label);

    struct
    {
        Difficulty d;
        std::string name;
        float x;
    } opts[] = {
        {EASY, "EASY", cx - 210.f},
        {NORMAL, "NORMAL", cx - 55.f},
        {HARD, "HARD", cx + 100.f}};

    for (auto &opt : opts)
    {
        bool sel = (selectedDifficulty == opt.d);
        bool hover = false;
        sf::FloatRect *rect = nullptr;
        if (opt.d == EASY)
        {
            hover = easyButtonRect.contains(mousePos);
            rect = &easyButtonRect;
        }
        else if (opt.d == NORMAL)
        {
            hover = normalButtonRect.contains(mousePos);
            rect = &normalButtonRect;
        }
        else
        {
            hover = hardButtonRect.contains(mousePos);
            rect = &hardButtonRect;
        }

        sf::Color fill = sel ? sf::Color(180, 45, 45) : (hover ? sf::Color(120, 40, 40) : sf::Color(50, 30, 30));
        float bw = 120.f, bh = 40.f;
        float bx = opt.x, by = baseY + 24.f;

        drawRoundedRect(window, bx, by, bw, bh, fill);

        sf::Text t(font, opt.name, 15);
        t.setFillColor(sel ? sf::Color(255, 215, 0) : sf::Color(200, 200, 200));
        sf::FloatRect tb = t.getLocalBounds();
        t.setPosition(sf::Vector2f(bx + (bw - tb.size.x) / 2.f, by + (bh - tb.size.y) / 2.f - tb.position.y));
        window.draw(t);

        *rect = sf::FloatRect(sf::Vector2f(bx, by), sf::Vector2f(bw, bh));
    }
}

void StartScreen::handleMouseClick(sf::Vector2f mousePos)
{
    if (easyButtonRect.contains(mousePos))
        selectedDifficulty = EASY;
    else if (normalButtonRect.contains(mousePos))
        selectedDifficulty = NORMAL;
    else if (hardButtonRect.contains(mousePos))
        selectedDifficulty = HARD;
}

bool StartScreen::isStartButtonClicked(sf::Vector2f mousePos) const
{
    return startButtonRect.contains(mousePos);
}

bool StartScreen::isQuitButtonClicked(sf::Vector2f mousePos) const
{
    return quitButtonRect.contains(mousePos);
}

StartScreen::Difficulty StartScreen::getSelectedDifficulty() const
{
    return selectedDifficulty;
}
