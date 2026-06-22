#include "GameScreen.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

GameScreen::GameScreen(GameLogic &gameLogic, sf::RenderWindow &window)
    : gameLogic(gameLogic), window(window), selectedCardIndex(-1), fontLoaded(false)
{
    loadFont();
    loadCardTextures();
    loadBackground();
    loadBullTypeTextures();
    loadButtonTexture();
    loadReadyTexture();
    createCardShadow();
}

// ==================== 资源加载 ====================

void GameScreen::loadBackground()
{
    const char *paths[] = {
        "C:\\Users\\lenovo\\Desktop\\Program\\C\\codes\\pokergame\\assets\\TYPE\\Desktop2.png",
        "assets\\TYPE\\Desktop2.png"};
    for (const auto &p : paths)
    {
        if (tableBgTexture.loadFromFile(p))
        {
            tableBgSprite.emplace(tableBgTexture);
            auto sz = tableBgTexture.getSize();
            if (sz.x > 0 && sz.y > 0)
            {
                tableBgSprite->setScale(sf::Vector2f(
                    1200.f / sz.x, 800.f / sz.y));
            }
            std::cout << "[Background] Loaded: " << p << std::endl;
            return;
        }
    }
    std::cerr << "[Background] WARNING: Could not load background texture." << std::endl;
}

void GameScreen::loadBullTypeTextures()
{
    const std::string basePath = "C:\\Users\\lenovo\\Desktop\\Program\\C\\codes\\pokergame\\assets\\TYPE\\";

    // N0=N0(没牛), N1~N9=牛1~牛9, N10=牛牛, N11=有牛通用
    // FIVE_FLOWER/BOMB/FIVE_SMALL 无对应图片，回退文字
    for (int i = 0; i <= 11; ++i)
    {
        std::string path = basePath + "N" + std::to_string(i) + ".png";
        sf::Texture tex;
        if (tex.loadFromFile(path))
        {
            bullTypeTextures.emplace(i, std::move(tex));
            std::cout << "[BullType] Loaded N" << i << std::endl;
        }
        else
        {
            std::cerr << "[BullType] WARNING: Could not load " << path << std::endl;
        }
    }
    bullTypeLoaded = !bullTypeTextures.empty();
}

void GameScreen::loadButtonTexture()
{
    std::string path = "C:\\Users\\lenovo\\Desktop\\Program\\C\\codes\\pokergame\\assets\\TYPE\\Button2.png";
    sf::Image img;
    if (buttonTexture.loadFromFile(path) && img.loadFromFile(path))
    {
        buttonTexReady = true;
        auto sz = img.getSize();
        // 扫描非透明像素的包围盒
        int minX = static_cast<int>(sz.x), minY = static_cast<int>(sz.y);
        int maxX = 0, maxY = 0;
        for (unsigned y = 0; y < sz.y; ++y)
        {
            for (unsigned x = 0; x < sz.x; ++x)
            {
                if (img.getPixel(sf::Vector2u(x, y)).a > 10)
                {
                    if (static_cast<int>(x) < minX)
                        minX = static_cast<int>(x);
                    if (static_cast<int>(y) < minY)
                        minY = static_cast<int>(y);
                    if (static_cast<int>(x) > maxX)
                        maxX = static_cast<int>(x);
                    if (static_cast<int>(y) > maxY)
                        maxY = static_cast<int>(y);
                }
            }
        }
        if (minX <= maxX && minY <= maxY)
        {
            buttonCropRect = sf::IntRect(sf::Vector2i(minX, minY), sf::Vector2i(maxX - minX + 1, maxY - minY + 1));
            std::cout << "[Button] Loaded & cropped: " << path
                      << " (" << sz.x << "x" << sz.y << " -> "
                      << buttonCropRect.size.x << "x" << buttonCropRect.size.y << ")" << std::endl;
        }
        else
        {
            buttonCropRect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(sz));
        }
    }
    else
    {
        std::cerr << "[Button] WARNING: Could not load " << path << std::endl;
    }
}

void GameScreen::loadReadyTexture()
{
    std::string path = "C:\\Users\\lenovo\\Desktop\\Program\\C\\codes\\pokergame\\assets\\TYPE\\GetReady.png";
    sf::Image img;
    if (readyTexture.loadFromFile(path) && img.loadFromFile(path))
    {
        readyTexReady = true;
        auto sz = img.getSize();
        int minX = static_cast<int>(sz.x), minY = static_cast<int>(sz.y);
        int maxX = 0, maxY = 0;
        for (unsigned y = 0; y < sz.y; ++y)
        {
            for (unsigned x = 0; x < sz.x; ++x)
            {
                if (img.getPixel(sf::Vector2u(x, y)).a > 10)
                {
                    if (static_cast<int>(x) < minX)
                        minX = static_cast<int>(x);
                    if (static_cast<int>(y) < minY)
                        minY = static_cast<int>(y);
                    if (static_cast<int>(x) > maxX)
                        maxX = static_cast<int>(x);
                    if (static_cast<int>(y) > maxY)
                        maxY = static_cast<int>(y);
                }
            }
        }
        if (minX <= maxX && minY <= maxY)
        {
            readyCropRect = sf::IntRect(sf::Vector2i(minX, minY), sf::Vector2i(maxX - minX + 1, maxY - minY + 1));
            std::cout << "[Ready] Loaded & cropped: " << path
                      << " (" << sz.x << "x" << sz.y << " -> "
                      << readyCropRect.size.x << "x" << readyCropRect.size.y << ")" << std::endl;
        }
        else
        {
            readyCropRect = sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(sz));
        }
    }
    else
    {
        std::cerr << "[Ready] WARNING: Could not load " << path << std::endl;
    }
}

void GameScreen::createCardShadow()
{
    // 用代码生成一个简单的柔光阴影纹理（32×48 半透明黑→透明）
    const unsigned SW = 40, SH = 54;
    sf::Image img;
    img.resize(sf::Vector2u(SW, SH), sf::Color::Transparent);
    for (unsigned y = 0; y < SH; ++y)
    {
        for (unsigned x = 0; x < SW; ++x)
        {
            float dx = static_cast<float>(x) - SW / 2.f;
            float dy = static_cast<float>(y) - SH / 2.f;
            float dist = std::sqrt(dx * dx + dy * dy);
            float alpha = std::max(0.f, 80.f * (1.f - dist / std::max(SW, SH) * 1.8f));
            if (alpha > 1.f)
            {
                img.setPixel(sf::Vector2u(x, y), sf::Color(0, 0, 0, static_cast<uint8_t>(alpha)));
            }
        }
    }
    cardShadowReady = cardShadowTex.loadFromImage(img);
}

void GameScreen::drawBackground()
{
    if (tableBgSprite.has_value())
        window.draw(*tableBgSprite);
    else
        window.clear(sf::Color(15, 35, 15));
}

void GameScreen::loadFont()
{
    // 尝试多个字体位置
    if (!font.openFromFile("C:\\Windows\\Fonts\\simhei.ttf"))
    {
        if (!font.openFromFile("C:\\Windows\\Fonts\\msyh.ttc"))
        {
            if (!font.openFromFile("C:\\Windows\\Fonts\\arial.ttf"))
            {
                std::cerr << "Warning: Could not load any font file" << std::endl;
                fontLoaded = false;
                return;
            }
        }
    }
    fontLoaded = true;
    std::cout << "Font loaded successfully" << std::endl;
}

void GameScreen::loadCardTextures()
{
    const std::vector<std::string> suits = {"Club", "Diamond", "Heart", "Spade"};
    const std::vector<std::string> ranks = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};
    const std::string basePath = "C:\\Users\\lenovo\\Desktop\\Program\\C\\codes\\pokergame\\assets\\PNG\\";

    std::ofstream log("card_debug.log", std::ios::app);
    log << "=== loadCardTextures() START ===" << std::endl;
    log << "basePath: " << basePath << std::endl;

    int loaded = 0;
    int failed = 0;
    for (const auto &suit : suits)
    {
        for (const auto &rank : ranks)
        {
            std::string key = suit + rank;
            std::string filePath = basePath + key + ".png";
            sf::Texture texture;
            if (texture.loadFromFile(filePath))
            {
                cardTextures.emplace(key, std::move(texture));
                loaded++;
            }
            else
            {
                if (failed < 3)
                {
                    log << "FAILED: " << filePath << std::endl;
                }
                failed++;
            }
        }
    }

    log << "Loaded: " << loaded << " / Failed: " << failed << " / Total: " << (loaded + failed) << std::endl;
    log << "=== loadCardTextures() END ===" << std::endl;
    log.close();

    // ── 加载卡背纹理（Background.png 就是卡背图）──
    std::string backPath = basePath + "Background.png";
    if (cardBackTex.loadFromFile(backPath))
    {
        cardBackReady = true;
        std::cout << "[CardBack] Loaded: " << backPath << std::endl;
    }
    else
        std::cerr << "[CardBack] WARNING: Could not load: " << backPath << std::endl;

    std::cout << "[CardTextures] Loaded: " << loaded << " / Failed: " << failed << " / Total: " << (loaded + failed) << std::endl;
    textureLoadStatus = "BUILD " __DATE__ " " __TIME__ " | " + std::to_string(loaded) + "/" + std::to_string(loaded + failed) + " textures loaded. Map keys:";
    for (const auto &kv : cardTextures)
    {
        textureLoadStatus += " " + kv.first;
        break; // 只显示第一个 key，避免字符串过长
    }
}

std::string GameScreen::getCardTextureKey(const Card &card) const
{
    return card.getSuitName() + card.getRankSymbol();
}

// ==================== 主绘制 ====================

void GameScreen::draw()
{
    drawBackground();

    // ── 顶部信息栏（半透明暗色条）──
    {
        sf::RectangleShape topBar(sf::Vector2f(1200.f, 44.f));
        topBar.setFillColor(sf::Color(10, 10, 20, 180));
        window.draw(topBar);

        sf::RectangleShape topBarBorder(sf::Vector2f(1200.f, 2.f));
        topBarBorder.setPosition(sf::Vector2f(0.f, 44.f));
        topBarBorder.setFillColor(sf::Color(218, 165, 32, 140)); // 金色底线
        window.draw(topBarBorder);
    }
    {
        // 左上：标题
        sf::Text titleText(font, "DOU NIU  |  Bull Fight", 18);
        titleText.setFillColor(sf::Color(255, 215, 0));
        titleText.setPosition(sf::Vector2f(14.f, 8.f));
        window.draw(titleText);

        // 右上：局数 + 底池
        std::string infoStr = "ROUND " + std::to_string(gameLogic.getRoundNumber()) + "    POT: " + std::to_string(gameLogic.getPot());
        sf::Text infoText(font, infoStr, 17);
        infoText.setFillColor(sf::Color(255, 215, 0));
        sf::FloatRect bounds = infoText.getLocalBounds();
        infoText.setPosition(sf::Vector2f(1200.f - bounds.size.x - 18.f, 9.f));
        window.draw(infoText);
    }

    drawDealAnimation();
    drawPlayers();
    drawSettleCelebration();

    // ── 组牌阶段：摊开手牌 + 确认按钮 ──
    if (gameLogic.isCardArrangementActive())
    {
        drawArrangementHand();
        drawArrangementButtons();
    }
    else
    {
        drawBiddingButtons();
    }

    // ── 结算阶段：展示牛几图片（利用按钮空位）──
    if (gameLogic.isSecondDealDone() && !gameLogic.isBiddingActive() && !gameLogic.isCardArrangementActive())
        drawBullTypeImages();

    // ── 结算公告（牌桌中央）──
    if (gameLogic.isSecondDealDone() && !gameLogic.isCardArrangementActive())
    {
        if (!gameLogic.getLastSettleResult().empty())
        {
            sf::Text settleText(font, gameLogic.getLastSettleResult(), 26);
            settleText.setFillColor(sf::Color(255, 255, 200));
            sf::FloatRect bounds = settleText.getLocalBounds();

            // 半透明背景
            sf::RectangleShape settleBg(sf::Vector2f(bounds.size.x + 50.f, 44.f));
            settleBg.setPosition(sf::Vector2f(600.f - bounds.size.x / 2.f - 25.f, 310.f));
            settleBg.setFillColor(sf::Color(30, 10, 10, 210));
            settleBg.setOutlineColor(sf::Color(218, 165, 32, 160));
            settleBg.setOutlineThickness(2.f);
            window.draw(settleBg);

            settleText.setPosition(sf::Vector2f(600.f - bounds.size.x / 2.f, 314.f));
            window.draw(settleText);
        }
        drawSettleButtons();
    }
}

// ==================== 玩家布局 ====================

void GameScreen::drawPlayers()
{
    const auto &players = gameLogic.getPlayers();
    float W = 1200.f, H = 800.f;

    const float cw = 86.f, ch = 127.f;
    const float overlap = 25.f;
    const float hTotalW = cw + 4.f * overlap;
    const float vTotalH = ch + 4.f * overlap;

    // ── 下方：玩家 0（水平叠放；组牌阶段跳过，由 drawArrangementHand 绘制）──
    if (players.size() >= 1)
    {
        float sx = W / 2.f - hTotalW / 2.f;

        if (gameLogic.isCardArrangementActive())
        {
            // 组牌阶段：由 drawArrangementHand 绘制（带选中交互）
        }
        else if (gameLogic.isBiddingActive())
        {
            // 叫庄/抢庄阶段：摊开平铺（与组牌阶段同Y坐标、同尺寸）
            const float cwArr = 86.f;
            const float gap = 15.f;
            float startY = H - 155.f;
            float totalWArr = cwArr * 5.f + gap * 4.f;
            float startX = W / 2.f - totalWArr / 2.f;
            for (int i = 0; i < players[0].getCardCount(); i++)
            {
                float cx = startX + i * (cwArr + gap);
                drawCard(players[0].getCard(i), cx, startY, 0.f, false, 1.15f);
            }
        }
        else
        {
            // 结算阶段：叠放（下移与摊开对齐）
            drawPlayerStackedHand(players[0], sx, H - 155.f, 0, false);
        }

        // 名字 → 左下角（庄家红，闲家金）
        {
            sf::Text nameText(font, players[0].getName(), 27);
            nameText.setFillColor(players[0].isBanker() ? sf::Color(255, 80, 80) : sf::Color(255, 220, 100));
            nameText.setPosition(sf::Vector2f(18.f, H - 55.f));
            window.draw(nameText);
        }
        // 分数 + 庄家 + Odds → 右下角
        {
            std::string infoLine = "$" + std::to_string(players[0].getScore());
            if (players[0].isBanker())
                infoLine += "  [BANKER]";
            if (gameLogic.isBiddingActive() || gameLogic.isSecondDealDone())
                infoLine += "    Odds x" + std::to_string(gameLogic.getEffectiveOdds());
            sf::Text infoText(font, infoLine, 24);
            infoText.setFillColor(players[0].isBanker() ? sf::Color(255, 80, 80) : sf::Color(255, 215, 0));
            sf::FloatRect ib = infoText.getLocalBounds();
            infoText.setPosition(sf::Vector2f(W - ib.size.x - 18.f, H - 55.f));
            window.draw(infoText);
        }
    }

    // ── 右侧：AI1（垂直叠放）──
    if (players.size() >= 2)
    {
        float sx = W - 100.f;
        float sy = H / 2.f - vTotalH / 2.f;
        drawPlayerStackedHand(players[1], sx, sy, 1, true);

        // 名字 + 金币 → 牌上方换行，右对齐防溢出
        float rightEdge = sx + cw;
        {
            sf::Text nameT(font, players[1].getName() + "(AI)", 24);
            nameT.setFillColor(sf::Color(255, 220, 100));
            sf::FloatRect nb = nameT.getLocalBounds();
            nameT.setPosition(sf::Vector2f(rightEdge - nb.size.x, sy - 52.f));
            window.draw(nameT);
        }
        {
            std::string st = "$" + std::to_string(players[1].getScore());
            if (players[1].isBanker())
                st += " [BK]";
            sf::Text stT(font, st, 21);
            stT.setFillColor(players[1].isBanker() ? sf::Color(255, 80, 80) : sf::Color(200, 180, 100));
            sf::FloatRect sb = stT.getLocalBounds();
            stT.setPosition(sf::Vector2f(rightEdge - sb.size.x, sy - 24.f));
            window.draw(stT);
        }
    }

    // ── 上方：AI2（水平叠放）──
    if (players.size() >= 3)
    {
        float sx = W / 2.f - hTotalW / 2.f;
        float sy = 70.f; // 上移利用空余空间（顶部栏 y=0..46）
        drawPlayerStackedHand(players[2], sx, sy, 2, false);

        // 名字 + 金币 → 牌上方左对齐，字号统一
        std::string label = players[2].getName() + "(AI)  $" + std::to_string(players[2].getScore());
        if (players[2].isBanker())
            label += " BK";
        sf::Text nameText(font, label, 24);
        nameText.setFillColor(players[2].isBanker() ? sf::Color(255, 80, 80) : sf::Color(255, 220, 100));
        nameText.setPosition(sf::Vector2f(sx, sy - 30.f));
        window.draw(nameText);
    }

    // ── 左侧：AI3（垂直叠放）──
    if (players.size() >= 4)
    {
        float sx = 14.f;
        float sy = H / 2.f - vTotalH / 2.f;
        drawPlayerStackedHand(players[3], sx, sy, 3, true);

        // 名字 + 金币 → 牌上方换行，左对齐防溢出
        {
            sf::Text nameT(font, players[3].getName() + "(AI)", 24);
            nameT.setFillColor(sf::Color(255, 220, 100));
            nameT.setPosition(sf::Vector2f(sx, sy - 52.f));
            window.draw(nameT);
        }
        {
            std::string st = "$" + std::to_string(players[3].getScore());
            if (players[3].isBanker())
                st += " [BK]";
            sf::Text stT(font, st, 21);
            stT.setFillColor(players[3].isBanker() ? sf::Color(255, 80, 80) : sf::Color(200, 180, 100));
            stT.setPosition(sf::Vector2f(sx, sy - 24.f));
            window.draw(stT);
        }
    }
}

// ── 统一叠放手牌（所有玩家共用）──
void GameScreen::drawPlayerStackedHand(const Player &player, float startX, float startY,
                                       int playerIndex, bool vertical)
{
    int cnt = player.getCardCount();
    if (cnt == 0)
        return;

    const float overlap = 25.f; // 每张牌露出的边宽
    bool reveal = (playerIndex == 0) || (gameLogic.isSecondDealDone() && !gameLogic.isCardArrangementActive());

    for (int i = 0; i < cnt; ++i)
    {
        float cx = vertical ? startX : startX + i * overlap;
        float cy = vertical ? startY + i * overlap : startY;

        bool sel = (playerIndex == 0 && gameLogic.isCardArrangementActive() && i == selectedCardIndex);
        float yOff = sel ? -8.f : 0.f;

        if (reveal)
            drawCard(player.getCard(i), cx, cy + yOff, 0.f, sel, 1.15f);
        else
            drawCardBack(cx, cy + yOff, 0.f, 1.15f);
    }
}

// ==================== 卡牌绘制 ====================

void GameScreen::drawCard(const Card &card, float x, float y, float rotation, bool selected, float scale)
{
    float cardW = 75.f * scale;
    float cardH = 110.f * scale;

    // ── 阴影 ──
    if (cardShadowReady)
    {
        sf::Sprite shadowSpr(cardShadowTex);
        auto sz = cardShadowTex.getSize();
        if (sz.x > 0 && sz.y > 0)
        {
            shadowSpr.setScale(sf::Vector2f(cardW / sz.x * 1.15f, cardH / sz.y * 1.15f));
        }
        shadowSpr.setPosition(sf::Vector2f(x + cardW * 0.05f, y + cardH * 0.05f));
        shadowSpr.setColor(sf::Color(0, 0, 0, 100));
        shadowSpr.setRotation(sf::degrees(rotation));
        window.draw(shadowSpr);
    }

    // ── 选中高亮 ──
    if (selected)
    {
        sf::RectangleShape glow(sf::Vector2f(cardW + 6.f, cardH + 6.f));
        glow.setPosition(sf::Vector2f(x - 3.f, y - 6.f));
        glow.setFillColor(sf::Color::Transparent);
        glow.setOutlineColor(sf::Color(255, 215, 0, 220));
        glow.setOutlineThickness(3.f);
        glow.setRotation(sf::degrees(rotation));
        window.draw(glow);
    }

    std::string textureKey = getCardTextureKey(card);
    auto it = cardTextures.find(textureKey);

    if (it != cardTextures.end())
    {
        // 白底
        sf::RectangleShape whiteBg(sf::Vector2f(cardW, cardH));
        whiteBg.setPosition(sf::Vector2f(x, y));
        whiteBg.setFillColor(sf::Color(250, 250, 250));
        whiteBg.setOutlineColor(sf::Color(60, 60, 60));
        whiteBg.setOutlineThickness(0.5f);
        whiteBg.setRotation(sf::degrees(rotation));
        window.draw(whiteBg);

        // 纹理
        sf::Sprite sprite(it->second);
        auto texSz = it->second.getSize();
        if (texSz.x > 0 && texSz.y > 0)
        {
            sprite.setScale(sf::Vector2f(cardW / texSz.x, cardH / texSz.y));
        }
        sprite.setPosition(sf::Vector2f(x, y));
        sprite.setRotation(sf::degrees(rotation));
        window.draw(sprite);
    }
    else
    {
        // fallback: 暗红色背景 + 文字
        sf::RectangleShape bg(sf::Vector2f(cardW, cardH));
        bg.setPosition(sf::Vector2f(x, y));
        bg.setFillColor(sf::Color(50, 10, 10));
        bg.setOutlineColor(sf::Color(80, 80, 80));
        bg.setOutlineThickness(1.f);
        bg.setRotation(sf::degrees(rotation));
        window.draw(bg);

        sf::Text fallback(font, card.toString(), 14);
        fallback.setPosition(sf::Vector2f(x + 4.f, y + 4.f));
        fallback.setFillColor(sf::Color::White);
        fallback.setRotation(sf::degrees(rotation));
        window.draw(fallback);
    }
}

void GameScreen::drawCardBack(float x, float y, float rotation, float scale)
{
    float w = 75.f * scale;
    float h = 110.f * scale;

    if (cardBackReady)
    {
        sf::Sprite sprite(cardBackTex);
        auto texSz = cardBackTex.getSize();
        if (texSz.x > 0 && texSz.y > 0)
            sprite.setScale(sf::Vector2f(w / texSz.x, h / texSz.y));
        sprite.setPosition(sf::Vector2f(x, y));
        sprite.setRotation(sf::degrees(rotation));
        window.draw(sprite);
    }
    else
    {
        // fallback：深蓝 + 菱形装饰
        sf::RectangleShape back(sf::Vector2f(w, h));
        back.setPosition(sf::Vector2f(x, y));
        back.setFillColor(sf::Color(20, 40, 100));
        back.setRotation(sf::degrees(rotation));
        window.draw(back);

        sf::ConvexShape diamond;
        diamond.setPointCount(4);
        float cx = x + w / 2.f, cy = y + h / 2.f;
        float dx = w * 0.28f, dy = h * 0.28f;
        diamond.setPoint(0, sf::Vector2f(cx, cy - dy));
        diamond.setPoint(1, sf::Vector2f(cx + dx, cy));
        diamond.setPoint(2, sf::Vector2f(cx, cy + dy));
        diamond.setPoint(3, sf::Vector2f(cx - dx, cy));
        diamond.setFillColor(sf::Color(60, 100, 180));
        diamond.setRotation(sf::degrees(rotation));
        window.draw(diamond);
    }

    // 金边
    sf::RectangleShape border(sf::Vector2f(w, h));
    border.setPosition(sf::Vector2f(x, y));
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(180, 160, 100));
    border.setOutlineThickness(1.f);
    border.setRotation(sf::degrees(rotation));
    window.draw(border);
}

// ==================== 牛几图片 ====================

void GameScreen::drawBullTypeImage(int quality, float x, float y, float scale)
{
    // quality: 0=N0(没牛), 1-9=N1~N9, 10=N10(牛牛)
    // 11+=FIVE_FLOWER/BOMB/FIVE_SMALL 无图片，回退文字
    int key = (quality >= 0 && quality <= 10) ? quality : -1;

    auto it = bullTypeTextures.find(key);
    if (it != bullTypeTextures.end())
    {
        sf::Sprite sprite(it->second);
        auto texSz = it->second.getSize();
        if (texSz.x > 0 && texSz.y > 0)
        {
            sprite.setScale(sf::Vector2f(scale, scale));
            sprite.setPosition(sf::Vector2f(x - texSz.x * scale / 2.f, y - texSz.y * scale / 2.f));
            window.draw(sprite);
        }
    }
    else
    {
        // 回退：金色文字
        std::string name = GameLogic::getBullTypeName(quality);
        sf::Text fallback(font, name, 18);
        fallback.setFillColor(sf::Color(255, 215, 0));
        sf::FloatRect fb = fallback.getLocalBounds();
        fallback.setPosition(sf::Vector2f(x - fb.size.x / 2.f, y - fb.size.y / 2.f));
        window.draw(fallback);
    }
}

void GameScreen::drawBullTypeImages()
{
    const auto &players = gameLogic.getPlayers();
    float W = 1200.f;

    // ── 玩家0（下方）：1.2倍放大，下移 ──
    if (players.size() >= 1)
        drawBullTypeImage(players[0].getHandQuality(), W / 2.f, 535.f, 1.44f);

    // ── AI1（右侧）：距牌底 20px，scale=1.2 ──
    if (players.size() >= 2)
        drawBullTypeImage(players[1].getHandQuality(), W - 57.5f, 534.f, 1.2f);

    // ── AI2（上方）：距牌底 30px，scale=1.2 ──
    if (players.size() >= 3)
        drawBullTypeImage(players[2].getHandQuality(), W / 2.f, 229.f, 1.2f);

    // ── AI3（左侧）：距牌底 20px，scale=1.2 ──
    if (players.size() >= 4)
        drawBullTypeImage(players[3].getHandQuality(), 57.5f, 534.f, 1.2f);
}

// ==================== 贴图按钮 ====================

void GameScreen::drawTexturedButton(const std::string &text, float x, float y, float w, float h,
                                    sf::Color textColor)
{
    if (buttonTexReady)
    {
        sf::Sprite btnSpr(buttonTexture);
        btnSpr.setTextureRect(buttonCropRect);
        auto tsz = sf::Vector2f(static_cast<float>(buttonCropRect.size.x),
                                static_cast<float>(buttonCropRect.size.y));
        if (tsz.x > 0 && tsz.y > 0)
            btnSpr.setScale(sf::Vector2f(w / tsz.x, h / tsz.y));
        btnSpr.setPosition(sf::Vector2f(x, y));
        window.draw(btnSpr);
    }
    else
    {
        // 回退：灰底圆角
        float r = h * 0.38f;
        sf::RectangleShape body(sf::Vector2f(w - 2.f * r, h));
        body.setPosition(sf::Vector2f(x + r, y));
        body.setFillColor(sf::Color(50, 50, 60));
        window.draw(body);
        sf::CircleShape cL(r), cR(r);
        cL.setPosition(sf::Vector2f(x, y));
        cL.setFillColor(sf::Color(50, 50, 60));
        window.draw(cL);
        cR.setPosition(sf::Vector2f(x + w - 2.f * r, y));
        cR.setFillColor(sf::Color(50, 50, 60));
        window.draw(cR);
    }

    // 文字（固定字号，不与按钮高度挂钩）
    sf::Text btnText(font, text, 21);
    btnText.setFillColor(textColor);
    sf::FloatRect tb = btnText.getLocalBounds();
    btnText.setPosition(sf::Vector2f(x + (w - tb.size.x) / 2.f, y + (h - tb.size.y) / 2.f - tb.position.y));
    window.draw(btnText);
}

// ==================== 中式圆角按钮（保留作为回退）====================

void GameScreen::drawRoundedButton(const std::string &text, float x, float y, float w, float h,
                                   sf::Color fillColor, sf::Color textColor, sf::Color borderColor,
                                   float borderWidth, bool glow)
{
    (void)borderWidth; // 由外部 drawRoundedRect 使用，此处保留接口一致性
    float r = h * 0.38f;

    // 主体矩形
    sf::RectangleShape body(sf::Vector2f(w - 2.f * r, h));
    body.setPosition(sf::Vector2f(x + r, y));
    body.setFillColor(fillColor);
    window.draw(body);

    // 左右半圆
    sf::CircleShape circL(r);
    circL.setPosition(sf::Vector2f(x, y));
    circL.setFillColor(fillColor);
    window.draw(circL);

    sf::CircleShape circR(r);
    circR.setPosition(sf::Vector2f(x + w - 2.f * r, y));
    circR.setFillColor(fillColor);
    window.draw(circR);

    // 发光
    if (glow)
    {
        sf::Color glowC = borderColor;
        glowC.a = 80;
        sf::RectangleShape gBody(sf::Vector2f(w - 2.f * r, h));
        gBody.setPosition(sf::Vector2f(x + r, y));
        gBody.setFillColor(glowC);
        window.draw(gBody);
        sf::CircleShape gL(r);
        gL.setPosition(sf::Vector2f(x, y));
        gL.setFillColor(glowC);
        window.draw(gL);
        sf::CircleShape gR(r);
        gR.setPosition(sf::Vector2f(x + w - 2.f * r, y));
        gR.setFillColor(glowC);
        window.draw(gR);
    }

    // 文字
    sf::Text btnText(font, text, 21);
    btnText.setFillColor(textColor);
    sf::FloatRect tb = btnText.getLocalBounds();
    btnText.setPosition(sf::Vector2f(x + (w - tb.size.x) / 2.f, y + (h - tb.size.y) / 2.f - tb.position.y));
    window.draw(btnText);
}

// ==================== 发牌动画 ====================

void GameScreen::drawDealAnimation()
{
    // 预留：发牌逐张飞入动画（后续迭代实现）
}

// ==================== 结算动画 ====================

void GameScreen::drawSettleCelebration()
{
    if (!gameLogic.isSecondDealDone() || gameLogic.isCardArrangementActive())
        return;

    int winner = gameLogic.getLastRoundWinner();
    if (winner < 0)
        return;

    // 赢家名字闪烁金色
    settleAnimElapsed += settleAnimClock.restart().asSeconds();
    float alpha = 0.5f + 0.5f * std::sin(settleAnimElapsed * 4.f);

    const auto &players = gameLogic.getPlayers();
    if (winner < static_cast<int>(players.size()))
    {
        std::string wName = players[winner].getName();
        if (players[winner].isAIPlayer())
            wName += "(AI)";
        sf::Text winText(font, "WINNER: " + wName, 33);
        winText.setFillColor(sf::Color(255, 215, 0, static_cast<uint8_t>(180 + 75 * alpha)));
        sf::FloatRect wb = winText.getLocalBounds();
        winText.setPosition(sf::Vector2f(600.f - wb.size.x / 2.f, 390.f));
        window.draw(winText);
    }
}

// ==================== 交互 ====================

void GameScreen::handleMouseClick(sf::Vector2f mousePos)
{
    const auto &players = gameLogic.getPlayers();
    if (players.empty() || players[0].isAIPlayer())
        return;

    int cnt = players[0].getCardCount();
    if (cnt == 0)
        return;

    // 水平叠放命中检测（从右往左遍历，右侧牌在上层）
    const float overlap = 25.f;
    const float cw = 86.f, ch = 127.f;
    float hTotalW = cw + 4.f * overlap;
    float startX = 600.f - hTotalW / 2.f;
    float startY = 800.f - 155.f;

    for (int i = cnt - 1; i >= 0; --i)
    {
        float cx = startX + i * overlap;
        float cy = startY;
        sf::FloatRect cardRect(sf::Vector2f(cx, cy), sf::Vector2f(cw, ch));
        if (cardRect.contains(mousePos))
        {
            selectedCardIndex = i;
            return;
        }
    }
}

void GameScreen::update()
{
    // 发牌动画计时
    if (dealAnimActive)
    {
        dealAnimElapsed += dealAnimClock.restart().asSeconds();
        if (dealAnimElapsed > DEAL_INTERVAL * 5 + DEAL_DURATION)
        {
            dealAnimActive = false;
        }
    }
}

void GameScreen::onNewRound()
{
    dealAnimActive = true;
    dealAnimElapsed = 0.f;
    dealAnimClock.restart();
    settleAnimElapsed = 0.f;
    settleAnimClock.restart();
    arrangementSelectedIndices.clear();
}

bool GameScreen::isPlayerTurn() const
{
    return gameLogic.getCurrentPlayerIndex() == 0;
}

int GameScreen::getSelectedCardIndex() const
{
    return selectedCardIndex;
}

void GameScreen::resetSelectedCard()
{
    selectedCardIndex = -1;
}

// ==================== 叫庄按钮（中式红金风格）====================

void GameScreen::drawBiddingButtons()
{
    if (!gameLogic.isBiddingActive())
        return;
    if (!fontLoaded)
        return;

    float centerX = 600.f;
    float buttonY = 574.f;
    const float btnW = 113.f;
    const float btnH = 38.f;
    const float btnGap = 25.f;

    bool playerIsCurrentBidder = (gameLogic.getCurrentBidderIndex() == 0);

    // 三按钮总宽 = 3*113 + 2*25 = 389，起始X = 600 - 194 = 406
    const float totalW = btnW * 3.f + btnGap * 2.f;
    float startX = centerX - totalW / 2.f;

    // ── CALL ──
    {
        bool canCall = gameLogic.canPlayerCallBanker(0);
        bool active = canCall && playerIsCurrentBidder;
        float bx = startX;
        drawTexturedButton(active ? "CALL" : "Call", bx, buttonY, btnW, btnH,
                           active ? sf::Color::White : sf::Color(140, 140, 140));
        callBankerButtonRect = sf::FloatRect(sf::Vector2f(bx, buttonY), sf::Vector2f(btnW, btnH));
    }

    // ── ROB ──
    {
        bool canRob = gameLogic.canPlayerRobBanker(0);
        bool active = canRob && playerIsCurrentBidder;
        float bx = startX + btnW + btnGap;
        drawTexturedButton(active ? "ROB" : "Rob", bx, buttonY, btnW, btnH,
                           active ? sf::Color::White : sf::Color(140, 140, 140));
        robBankerButtonRect = sf::FloatRect(sf::Vector2f(bx, buttonY), sf::Vector2f(btnW, btnH));
    }

    // ── PASS ──
    {
        float bx = startX + (btnW + btnGap) * 2.f;
        drawTexturedButton("PASS", bx, buttonY, btnW, btnH,
                           playerIsCurrentBidder ? sf::Color::White : sf::Color(120, 120, 120));
        passBankerButtonRect = sf::FloatRect(sf::Vector2f(bx, buttonY), sf::Vector2f(btnW, btnH));
    }

    // 当前出价者指示
    if (!playerIsCurrentBidder && gameLogic.getCurrentBidderIndex() >= 0)
    {
        const auto &players = gameLogic.getPlayers();
        int bi = gameLogic.getCurrentBidderIndex();
        if (bi < static_cast<int>(players.size()))
        {
            std::string wt = "Waiting for " + players[bi].getName();
            if (players[bi].isAIPlayer())
                wt += "(AI)";
            wt += "...";
            sf::Text waitText(font, wt, 21);
            waitText.setFillColor(sf::Color(180, 180, 180));
            sf::FloatRect wb = waitText.getLocalBounds();
            waitText.setPosition(sf::Vector2f(centerX - wb.size.x / 2.f, buttonY - 46.f));
            window.draw(waitText);
        }
    }

    // 反抢回合提示
    if (gameLogic.isRobBackPhase() && playerIsCurrentBidder)
    {
        sf::Text robBackText(font, "COUNTER-ROB! x" + std::to_string(gameLogic.getEffectiveOdds() * 2), 24);
        robBackText.setFillColor(sf::Color(255, 180, 0));
        sf::FloatRect rb = robBackText.getLocalBounds();
        robBackText.setPosition(sf::Vector2f(centerX - rb.size.x / 2.f, buttonY - 46.f));
        window.draw(robBackText);
    }
}

void GameScreen::handleBiddingClick(sf::Vector2f mousePos)
{
    if (!gameLogic.isBiddingActive() || gameLogic.getCurrentBidderIndex() != 0)
        return;

    if (gameLogic.canPlayerCallBanker(0) && callBankerButtonRect.contains(mousePos))
    {
        gameLogic.playerCallBanker(0);
        std::cout << "Player calls banker. Odds x" << gameLogic.getCurrentOddsMultiplier() << std::endl;
        return;
    }
    if (gameLogic.canPlayerRobBanker(0) && robBankerButtonRect.contains(mousePos))
    {
        gameLogic.playerRobBanker(0);
        std::cout << "Player robs banker. Odds x" << gameLogic.getCurrentOddsMultiplier() << std::endl;
        return;
    }
    if (passBankerButtonRect.contains(mousePos))
    {
        gameLogic.playerPassBanker(0);
        return;
    }
}

// ==================== 结算按钮（继续/退回主界面）====================

void GameScreen::drawSettleButtons()
{
    if (!fontLoaded)
        return;

    float centerX = 600.f;
    float buttonY = 574.f;
    const float btnW = 135.f;
    const float btnH = 38.f;
    const float btnGap = 25.f;

    // ── Continue ──
    {
        float bx = centerX - btnW - btnGap / 2.f;
        drawTexturedButton("Continue", bx, buttonY, btnW, btnH, sf::Color::White);
        continueButtonRect = sf::FloatRect(sf::Vector2f(bx, buttonY), sf::Vector2f(btnW, btnH));
    }

    // ── Main Menu ──
    {
        float bx = centerX + btnGap / 2.f;
        drawTexturedButton("Main Menu", bx, buttonY, btnW, btnH, sf::Color::White);
        backToMenuButtonRect = sf::FloatRect(sf::Vector2f(bx, buttonY), sf::Vector2f(btnW, btnH));
    }
}

int GameScreen::handleSettleClick(sf::Vector2f mousePos)
{
    if (continueButtonRect.contains(mousePos))
        return 1;
    if (backToMenuButtonRect.contains(mousePos))
        return 2;
    return 0;
}

// ==================== 组牌阶段 ====================

void GameScreen::drawArrangementHand()
{
    const auto &players = gameLogic.getPlayers();
    if (players.empty())
        return;

    int cnt = players[0].getCardCount();
    if (cnt != 5)
        return;

    float W = 1200.f, H = 800.f;
    const float cw = 86.f;
    // 5张牌平铺：间距15px
    const float gap = 15.f;
    const float totalW = cw * 5.f + gap * 4.f;
    float startX = W / 2.f - totalW / 2.f;
    float startY = H - 155.f;

    for (int i = 0; i < cnt; i++)
    {
        float cx = startX + i * (cw + gap);
        float cy = startY;

        // 是否已选中
        bool sel = false;
        for (int idx : arrangementSelectedIndices)
        {
            if (idx == i)
            {
                sel = true;
                break;
            }
        }

        float yOff = sel ? -15.f : 0.f;
        drawCard(players[0].getCard(i), cx, cy + yOff, 0.f, sel, 1.15f);

        // 选中序号标记
        if (sel)
        {
            int selNum = -1;
            for (size_t s = 0; s < arrangementSelectedIndices.size(); s++)
            {
                if (arrangementSelectedIndices[s] == i)
                {
                    selNum = static_cast<int>(s) + 1;
                    break;
                }
            }
            sf::CircleShape numBg(12.f);
            numBg.setPosition(sf::Vector2f(cx + cw - 14.f, cy + yOff - 6.f));
            numBg.setFillColor(sf::Color(255, 180, 0));
            window.draw(numBg);
            sf::Text numText(font, std::to_string(selNum), 14);
            numText.setFillColor(sf::Color::Black);
            sf::FloatRect nb = numText.getLocalBounds();
            numText.setPosition(sf::Vector2f(cx + cw - 14.f + 7.f - nb.size.x / 2.f,
                                             cy + yOff - 6.f + 7.f - nb.size.y / 2.f));
            window.draw(numText);
        }
    }
}

void GameScreen::drawArrangementButtons()
{
    if (!fontLoaded)
        return;

    float centerX = 600.f;
    float buttonY = 585.f;
    const float btnW = 150.f;
    const float btnH = 38.f;

    bool ready = (arrangementSelectedIndices.size() == 3);

    // ── Confirm 按钮（选中3张即可确认，没牛也允许）──
    {
        float btnHC = btnH * 1.2f; // 竖直拉伸1.2倍
        float by = buttonY - 11.f; // 上移11px（原 585-6=579 → 585-11=574）
        float bx = centerX - btnW / 2.f;
        if (readyTexReady)
        {
            // 绘制按钮底图 + 准备！图片
            if (buttonTexReady)
            {
                sf::Sprite btnSpr(buttonTexture);
                btnSpr.setTextureRect(buttonCropRect);
                auto tsz = sf::Vector2f(static_cast<float>(buttonCropRect.size.x),
                                        static_cast<float>(buttonCropRect.size.y));
                if (tsz.x > 0 && tsz.y > 0)
                    btnSpr.setScale(sf::Vector2f(btnW / tsz.x, btnHC / tsz.y));
                btnSpr.setPosition(sf::Vector2f(bx, by));
                if (!ready)
                    btnSpr.setColor(sf::Color(140, 140, 140));
                window.draw(btnSpr);
            }

            // GetReady 图片保持原尺寸（不随按钮拉伸）
            float imgH = btnH - 6.f;
            float scale = imgH / readyCropRect.size.y;
            float imgW = readyCropRect.size.x * scale;
            sf::Sprite readySpr(readyTexture, readyCropRect);
            readySpr.setScale(sf::Vector2f(scale, scale));
            readySpr.setPosition(sf::Vector2f(bx + (btnW - imgW) / 2.f, by + (btnHC - imgH) / 2.f - 1.f));
            if (!ready)
                readySpr.setColor(sf::Color(140, 140, 140));
            window.draw(readySpr);
        }
        else
        {
            drawTexturedButton(ready ? "CONFIRM" : "Confirm", bx, by, btnW, btnHC,
                               ready ? sf::Color::White : sf::Color(140, 140, 140));
        }
        confirmArrangementButtonRect = sf::FloatRect(sf::Vector2f(bx, by), sf::Vector2f(btnW, btnHC));
    }
}

int GameScreen::handleArrangementClick(sf::Vector2f mousePos)
{
    const auto &players = gameLogic.getPlayers();
    if (players.empty())
        return 0;

    int cnt = players[0].getCardCount();
    if (cnt != 5)
        return 0;

    float W = 1200.f, H = 800.f;
    const float cw = 86.f, ch = 127.f;
    const float gap = 15.f;
    const float totalW = cw * 5.f + gap * 4.f;
    float startX = W / 2.f - totalW / 2.f;
    float startY = H - 155.f;

    // ── 先检查是否点击了 Confirm 按钮（选满3张即可确认，含没牛）──
    if (arrangementSelectedIndices.size() == 3)
    {
        if (confirmArrangementButtonRect.contains(mousePos))
        {
            return 1; // 确认组牌，由 GameLoop 处理结算
        }
    }

    // ── 点击牌：切换选中/取消 ──
    for (int i = 0; i < cnt; i++)
    {
        float cx = startX + i * (cw + gap);
        float cy = startY;

        // 选中牌上移了，扩大点击区域
        bool alreadySel = false;
        for (int idx : arrangementSelectedIndices)
            if (idx == i)
            {
                alreadySel = true;
                break;
            }
        float topY = alreadySel ? cy - 15.f : cy;

        sf::FloatRect cardRect(sf::Vector2f(cx, topY), sf::Vector2f(cw, ch));
        if (cardRect.contains(mousePos))
        {
            // 切换选中状态
            auto it = std::find(arrangementSelectedIndices.begin(), arrangementSelectedIndices.end(), i);
            if (it != arrangementSelectedIndices.end())
            {
                arrangementSelectedIndices.erase(it);
            }
            else if (arrangementSelectedIndices.size() < 3)
            {
                arrangementSelectedIndices.push_back(i);
            }
            // 保持排序
            std::sort(arrangementSelectedIndices.begin(), arrangementSelectedIndices.end());
            return 0;
        }
    }

    return 0;
}

const std::vector<int> &GameScreen::getArrangementSelection() const
{
    return arrangementSelectedIndices;
}

void GameScreen::resetArrangementSelection()
{
    arrangementSelectedIndices.clear();
}
