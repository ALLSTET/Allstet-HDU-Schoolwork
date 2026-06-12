#include "GameScreen.h"
#include <iostream>
#include <sstream>
#include <fstream>

GameScreen::GameScreen(GameLogic& gameLogic, sf::RenderWindow& window)
    : gameLogic(gameLogic), window(window), selectedCardIndex(-1), fontLoaded(false) {
    loadFont();
    loadCardTextures();
}

void GameScreen::loadFont() {
    // 尝试多个字体位置
    if (!font.openFromFile("C:\\Windows\\Fonts\\simhei.ttf")) {
        if (!font.openFromFile("C:\\Windows\\Fonts\\msyh.ttc")) {
            if (!font.openFromFile("C:\\Windows\\Fonts\\arial.ttf")) {
                std::cerr << "Warning: Could not load any font file" << std::endl;
                fontLoaded = false;
                return;
            }
        }
    }
    fontLoaded = true;
    std::cout << "Font loaded successfully" << std::endl;
}

void GameScreen::loadCardTextures() {
    const std::vector<std::string> suits = {"Club", "Diamond", "Heart", "Spade"};
    const std::vector<std::string> ranks = {"2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A"};
    const std::string basePath = "C:\\Users\\lenovo\\Desktop\\Program\\C\\codes\\game\\assets\\PNG\\";

    std::ofstream log("card_debug.log", std::ios::app);
    log << "=== loadCardTextures() START ===" << std::endl;
    log << "basePath: " << basePath << std::endl;

    int loaded = 0;
    int failed = 0;
    for (const auto& suit : suits) {
        for (const auto& rank : ranks) {
            std::string key = suit + rank;
            std::string filePath = basePath + key + ".png";
            sf::Texture texture;
            if (texture.loadFromFile(filePath)) {
                cardTextures.emplace(key, std::move(texture));
                loaded++;
            } else {
                if (failed < 3) {
                    log << "FAILED: " << filePath << std::endl;
                }
                failed++;
            }
        }
    }

    log << "Loaded: " << loaded << " / Failed: " << failed << " / Total: " << (loaded + failed) << std::endl;
    log << "=== loadCardTextures() END ===" << std::endl;
    log.close();

    std::cout << "[CardTextures] Loaded: " << loaded << " / Failed: " << failed << " / Total: " << (loaded + failed) << std::endl;
    textureLoadStatus = "BUILD " __DATE__ " " __TIME__ " | " + std::to_string(loaded) + "/" + std::to_string(loaded + failed) + " textures loaded. Map keys:";
    for (const auto& kv : cardTextures) {
        textureLoadStatus += " " + kv.first;
        break;  // 只显示第一个 key，避免字符串过长
    }
}

std::string GameScreen::getCardTextureKey(const Card& card) const {
    return card.getSuitName() + card.getRankSymbol();
}

void GameScreen::draw() {
    drawPlayers();
    drawTableCards();
    drawBiddingButtons();

    // ── 顶部状态栏 ──
    // 左上：构建信息（暗灰色，小字）
    {
        sf::Text debugText(font, textureLoadStatus, 14);
        debugText.setFillColor(sf::Color(60, 60, 60));
        debugText.setPosition(sf::Vector2f(8.f, 4.f));
        window.draw(debugText);
    }
    // 右上：局数 + 底池
    {
        std::string infoStr = "Round " + std::to_string(gameLogic.getRoundNumber())
                            + "  |  Pot: " + std::to_string(gameLogic.getPot()) + " chips";
        sf::Text infoText(font, infoStr, 18);
        infoText.setFillColor(sf::Color::Cyan);
        sf::FloatRect bounds = infoText.getLocalBounds();
        infoText.setPosition(sf::Vector2f(window.getSize().x - bounds.size.x - 18.f, 5.f));
        window.draw(infoText);
    }

    // ── 结算公告区（仅结算后显示）──
    if (gameLogic.isSecondDealDone()) {
        if (!gameLogic.getLastSettleResult().empty()) {
            sf::Text settleText(font, gameLogic.getLastSettleResult(), 22);
            settleText.setFillColor(sf::Color::White);
            sf::FloatRect bounds = settleText.getLocalBounds();
            settleText.setPosition(sf::Vector2f(
                window.getSize().x / 2.f - bounds.size.x / 2.f, 48.f));
            window.draw(settleText);
        }
        sf::Text hintText(font, "Press  SPACE  for next round", 18);
        hintText.setFillColor(sf::Color::Yellow);
        sf::FloatRect bounds = hintText.getLocalBounds();
        hintText.setPosition(sf::Vector2f(
            window.getSize().x / 2.f - bounds.size.x / 2.f, 80.f));
        window.draw(hintText);
    }
}

void GameScreen::drawPlayers() {
    const auto& players = gameLogic.getPlayers();
    float windowWidth = static_cast<float>(window.getSize().x);
    float windowHeight = static_cast<float>(window.getSize().y);
    
    // 布局：下方(玩家0), 右侧(AI1), 上方(AI2), 左侧(AI3)
    if (players.size() >= 1) {
        float playerWidth = players[0].getCardCount() > 0 ? (players[0].getCardCount() - 1) * 70.f + 60.f : 60.f;
        float startX = (windowWidth - playerWidth) / 2.f;
        drawPlayerHand(players[0], startX, windowHeight - 240.f, 0, true);
    }
    if (players.size() >= 2) {
        float rightY = (windowHeight - (players[1].getCardCount() - 1) * 110.f - 100.f) / 2.f;
        drawPlayerHand(players[1], windowWidth - 120, rightY, 1, false);
    }
    if (players.size() >= 3) {
        float topWidth = players[2].getCardCount() > 0 ? (players[2].getCardCount() - 1) * 70.f + 60.f : 60.f;
        float startX = (windowWidth - topWidth) / 2.f;
        drawPlayerHand(players[2], startX, 20, 2, true);
    }
    if (players.size() >= 4) {
        float leftY = (windowHeight - (players[3].getCardCount() - 1) * 110.f - 100.f) / 2.f;
        drawPlayerHand(players[3], 20, leftY, 3, false);
    }
}

void GameScreen::drawPlayerHand(const Player& player, float x, float y, int playerIndex, bool horizontal) {
    // ── 玩家名字 ──
    std::string playerNameStr = player.getName();
    sf::Text playerName(font, playerNameStr, 20);
    playerName.setFillColor(sf::Color::Yellow);
    
    if (horizontal) {
        if (y < window.getSize().y / 2.f) {
            playerName.setPosition(sf::Vector2f(x, y + 108));  // 上半区：名字在牌下方
        } else {
            playerName.setPosition(sf::Vector2f(x, y - 42));    // 下半区：名字在牌上方
        }
    } else {
        playerName.setPosition(sf::Vector2f(x - 22, y - 28));
    }
    window.draw(playerName);

    // ── 筹码 ──
    {
        std::string chipStr = "Chips: " + std::to_string(player.getScore());
        sf::Text chipText(font, chipStr, 15);
        chipText.setFillColor(sf::Color(200, 180, 50));
        float chipY = 0.f;
        if (horizontal) {
            chipY = (y < window.getSize().y / 2.f) ? y + 128 : y - 22;
            chipText.setPosition(sf::Vector2f(x, chipY));
        } else {
            chipText.setPosition(sf::Vector2f(
                (playerIndex == 1) ? x - 60 : x + 80, y + 10));
        }
        window.draw(chipText);

        // 结算后显示牌型（紧贴筹码下方）
        if (gameLogic.isSecondDealDone()) {
            int quality = player.getHandQuality();
            sf::Text typeText(font, GameLogic::getBullTypeName(quality), 15);
            typeText.setFillColor(sf::Color::White);
            typeText.setPosition(sf::Vector2f(
                chipText.getPosition().x, chipText.getPosition().y + 18));
            window.draw(typeText);
        }

        // 庄家标记（筹码右侧）
        if (player.isBanker()) {
            sf::Text bankerText(font, "[BANKER]", 15);
            bankerText.setFillColor(sf::Color::Red);
            bankerText.setPosition(sf::Vector2f(
                chipText.getPosition().x + chipText.getLocalBounds().size.x + 12.f,
                chipText.getPosition().y - 1));
            window.draw(bankerText);
        }
    }
    
    // 绘制卡牌
    if (horizontal) {
        // 水平排列
        float cardX = x;
        for (int j = 0; j < player.getCardCount(); j++) {
            bool selected = (playerIndex == 0 && j == selectedCardIndex);
            drawCard(player.getCard(j), cardX, y, selected);
            cardX += 70;
        }
    } else {
        // 竖直排列
        float cardY = y;
        for (int j = 0; j < player.getCardCount(); j++) {
            bool selected = (playerIndex == 0 && j == selectedCardIndex);
            drawCard(player.getCard(j), x, cardY, selected);
            cardY += 110;
        }
    }
}

void GameScreen::drawCard(const Card& card, float x, float y, bool selected) {
    static int drawCallCount = 0;
    drawCallCount++;

    if (selected) {
        sf::RectangleShape selectionRect(sf::Vector2f(64.f, 104.f));
        selectionRect.setPosition(sf::Vector2f(x - 2.f, y - 2.f));
        selectionRect.setFillColor(sf::Color::Transparent);
        selectionRect.setOutlineColor(sf::Color::Red);
        selectionRect.setOutlineThickness(2.f);
        window.draw(selectionRect);
    }

    std::string textureKey = getCardTextureKey(card);
    auto it = cardTextures.find(textureKey);

    // 先画背景色：纹理命中=暗绿，未命中=暗红（一眼区分）
    sf::RectangleShape bgRect(sf::Vector2f(60.f, 100.f));
    bgRect.setPosition(sf::Vector2f(x, y));

    if (it != cardTextures.end()) {
        bgRect.setFillColor(sf::Color(0, 80, 0));  // 暗绿色 = 纹理命中
        window.draw(bgRect);

        if (drawCallCount <= 3) {
            std::ofstream log("card_debug.log", std::ios::app);
            log << "drawCard #" << drawCallCount << " HIT: key=" << textureKey
                << " tex=" << it->second.getSize().x << "x" << it->second.getSize().y
                << " @" << x << "," << y << std::endl;
            log.close();
        }
        const sf::Texture& texture = it->second;
        sf::Sprite sprite(texture);
        const sf::Vector2u texSize = texture.getSize();
        if (texSize.x > 0 && texSize.y > 0) {
            sprite.setScale(sf::Vector2f(
                60.f / static_cast<float>(texSize.x),
                100.f / static_cast<float>(texSize.y)));
        }
        sprite.setPosition(sf::Vector2f(x, y));
        window.draw(sprite);
        return;
    }

    // 未命中 → 暗红色背景 + 白色文字
    if (drawCallCount <= 3) {
        std::ofstream log("card_debug.log", std::ios::app);
        log << "drawCard #" << drawCallCount << " MISS: key=" << textureKey
            << " mapSize=" << cardTextures.size() << std::endl;
        log.close();
    }
    bgRect.setFillColor(sf::Color(80, 0, 0));
    bgRect.setOutlineColor(sf::Color::Black);
    bgRect.setOutlineThickness(1.f);
    window.draw(bgRect);

    std::string display = card.toString();
    sf::Text rankText(font, display, 16);
    rankText.setPosition(sf::Vector2f(x + 6, y + 6));
    rankText.setFillColor(sf::Color::White);
    window.draw(rankText);
}

void GameScreen::drawTableCards() {
    auto tableCards = gameLogic.getTableCards();
    
    // 绘制标题
    std::string tableStr = "Table";
    sf::Text tableLabel(font, tableStr, 22);
    tableLabel.setFillColor(sf::Color::Cyan);
    sf::FloatRect lb = tableLabel.getLocalBounds();
    tableLabel.setPosition(sf::Vector2f(window.getSize().x / 2.f - lb.size.x / 2.f, 185));
    window.draw(tableLabel);
    
    float cardX = window.getSize().x / 2.f - 150;
    float cardY = 215;
    
    for (const auto& card : tableCards) {
        drawCard(card, cardX, cardY, false);
        cardX += 70;
    }
}

int GameScreen::calculateHandValue(const Player& player) const {
    int value = 0;
    for (const auto& card : player.getAllCards()) {
        value += card.getValue();
    }
    return value;
}

void GameScreen::drawGameInfo() {
    // 已移除顶部右侧概览，分数显示现在靠近对应玩家牌组处
}

void GameScreen::drawInstructions() {
    // 左上提示已移除
}

void GameScreen::handleMouseClick(sf::Vector2f mousePos) {
    const auto& players = gameLogic.getPlayers();
    if (players.empty() || players[0].isAIPlayer()) return;
    
    float cardY = window.getSize().y - 240.f;
    float playerWidth = players[0].getCardCount() > 0 ? (players[0].getCardCount() - 1) * 70.f + 60.f : 60.f;
    float startX = (window.getSize().x - playerWidth) / 2.f;
    
    for (int i = 0; i < players[0].getCardCount(); i++) {
        sf::FloatRect cardRect(sf::Vector2f(startX + i * 70, cardY), sf::Vector2f(60, 100));
        if (cardRect.contains(mousePos)) {
            selectedCardIndex = i;
            break;
        }
    }
}

void GameScreen::update() {
    // 更新游戏状态
}

bool GameScreen::isPlayerTurn() const {
    return gameLogic.getCurrentPlayerIndex() == 0;
}

int GameScreen::getSelectedCardIndex() const {
    return selectedCardIndex;
}

void GameScreen::resetSelectedCard() {
    selectedCardIndex = -1;
}

void GameScreen::drawButton(const std::string&, float, float, float, float) {
    // 绘制按钮实现（预留）
}

void GameScreen::drawBiddingButtons() {
    if (!gameLogic.isBiddingActive()) {
        return;
    }
    if (!fontLoaded) {
        return;  // 字体未加载，跳过按钮绘制
    }

    float windowWidth = static_cast<float>(window.getSize().x);
    float buttonY = window.getSize().y - 108.f;  // 按钮在底部卡牌下方
    float centerX = windowWidth / 2.f;
    const float btnW = 150.f;
    const float btnH = 44.f;
    const float btnGap = 30.f;  // 两按钮间距

    bool playerIsCurrentBidder = (gameLogic.getCurrentBidderIndex() == 0);
    bool bankerCalled = gameLogic.hasSomeoneCalledBanker();

    // Call Banker button (中心偏左)
    {
        bool canCall = gameLogic.canPlayerCallBanker(0);
        bool active = canCall && playerIsCurrentBidder;
        sf::Color fillColor = active ? sf::Color(0, 180, 0) : sf::Color(80, 80, 80);
        sf::Color textColor = active ? sf::Color::White : sf::Color(160, 160, 160);

        float bx = centerX - btnW - btnGap / 2.f;

        sf::RectangleShape btnRect(sf::Vector2f(btnW, btnH));
        btnRect.setPosition(sf::Vector2f(bx, buttonY));
        btnRect.setFillColor(fillColor);
        btnRect.setOutlineColor(sf::Color::White);
        btnRect.setOutlineThickness(2.f);
        window.draw(btnRect);

        callBankerButtonRect = sf::FloatRect(sf::Vector2f(bx, buttonY), sf::Vector2f(btnW, btnH));

        sf::Text btnText(font, "Call", 20);
        btnText.setFillColor(textColor);
        auto bounds = btnText.getLocalBounds();
        btnText.setPosition(sf::Vector2f(bx + (btnW - bounds.size.x) / 2.f, buttonY + (btnH - 26.f) / 2.f));
        window.draw(btnText);

        if (!bankerCalled && playerIsCurrentBidder) {
            sf::Text hintText(font, "You are bidding", 13);
            hintText.setFillColor(sf::Color::Yellow);
            hintText.setPosition(sf::Vector2f(bx, buttonY - 18.f));
            window.draw(hintText);
        }
    }

    // Rob Banker button (中心偏右)
    {
        bool canRob = gameLogic.canPlayerRobBanker(0);
        bool active = canRob && playerIsCurrentBidder;
        sf::Color fillColor = active ? sf::Color(190, 50, 50) : sf::Color(80, 80, 80);
        sf::Color textColor = active ? sf::Color::White : sf::Color(160, 160, 160);

        float bx = centerX + btnGap / 2.f;

        sf::RectangleShape btnRect(sf::Vector2f(btnW, btnH));
        btnRect.setPosition(sf::Vector2f(bx, buttonY));
        btnRect.setFillColor(fillColor);
        btnRect.setOutlineColor(sf::Color::White);
        btnRect.setOutlineThickness(2.f);
        window.draw(btnRect);

        robBankerButtonRect = sf::FloatRect(sf::Vector2f(bx, buttonY), sf::Vector2f(btnW, btnH));

        sf::Text btnText(font, "Rob", 20);
        btnText.setFillColor(textColor);
        auto bounds = btnText.getLocalBounds();
        btnText.setPosition(sf::Vector2f(bx + (btnW - bounds.size.x) / 2.f, buttonY + (btnH - 26.f) / 2.f));
        window.draw(btnText);

        if (bankerCalled && playerIsCurrentBidder) {
            sf::Text hintText(font, "You are bidding", 13);
            hintText.setFillColor(sf::Color::Yellow);
            hintText.setPosition(sf::Vector2f(bx, buttonY - 18.f));
            window.draw(hintText);
        }
    }

    // Pass button (两主按钮中间下方)
    {
        float passW = 90.f;
        float passH = 30.f;
        float bx = centerX - passW / 2.f;
        float by = buttonY + btnH + 10.f;

        sf::Color fillColor = playerIsCurrentBidder ? sf::Color(70, 70, 140) : sf::Color(50, 50, 50);
        sf::Color textColor = playerIsCurrentBidder ? sf::Color::White : sf::Color(140, 140, 140);

        sf::RectangleShape btnRect(sf::Vector2f(passW, passH));
        btnRect.setPosition(sf::Vector2f(bx, by));
        btnRect.setFillColor(fillColor);
        btnRect.setOutlineColor(sf::Color::White);
        btnRect.setOutlineThickness(1.5f);
        window.draw(btnRect);

        passBankerButtonRect = sf::FloatRect(sf::Vector2f(bx, by), sf::Vector2f(passW, passH));

        sf::Text btnText(font, "Pass", 16);
        btnText.setFillColor(textColor);
        auto bounds = btnText.getLocalBounds();
        btnText.setPosition(sf::Vector2f(bx + (passW - bounds.size.x) / 2.f, by + (passH - 20.f) / 2.f));
        window.draw(btnText);
    }

    // Odds 赔率（按钮上方居中）
    int odds = gameLogic.getCurrentOddsMultiplier();
    sf::Text oddsText(font, "Odds: x" + std::to_string(odds), 16);
    oddsText.setFillColor(sf::Color::White);
    sf::FloatRect odBounds = oddsText.getLocalBounds();
    oddsText.setPosition(sf::Vector2f(centerX - odBounds.size.x / 2.f, buttonY - 26.f));
    window.draw(oddsText);
}

void GameScreen::handleBiddingClick(sf::Vector2f mousePos) {
    if (!gameLogic.isBiddingActive() || gameLogic.getCurrentBidderIndex() != 0) {
        return;
    }

    if (gameLogic.canPlayerCallBanker(0) && callBankerButtonRect.contains(mousePos)) {
        gameLogic.playerCallBanker(0);
        std::cout << "Player calls banker. Odds x" << gameLogic.getCurrentOddsMultiplier() << std::endl;
        return;
    }

    if (gameLogic.canPlayerRobBanker(0) && robBankerButtonRect.contains(mousePos)) {
        gameLogic.playerRobBanker(0);
        std::cout << "Player robs banker. Odds x" << gameLogic.getCurrentOddsMultiplier() << std::endl;
        return;
    }

    if (passBankerButtonRect.contains(mousePos)) {
        gameLogic.playerPassBanker(0);
        return;
    }
}
