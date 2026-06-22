#ifndef GAME_SCREEN_H
#define GAME_SCREEN_H

#include "../core/GameLogic.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_map>
#include <string>
#include <optional>

class GameScreen
{
public:
    GameScreen(GameLogic &gameLogic, sf::RenderWindow &window);

    void draw();
    void handleMouseClick(sf::Vector2f mousePos);
    void handleBiddingClick(sf::Vector2f mousePos);
    int handleSettleClick(sf::Vector2f mousePos);      // 0=nothing, 1=继续, 2=退回主界面
    int handleArrangementClick(sf::Vector2f mousePos); // 0=nothing, 1=confirmed
    const std::vector<int> &getArrangementSelection() const;
    void resetArrangementSelection();
    void update();
    void onNewRound(); // 新回合时重置动画状态

    bool isPlayerTurn() const;
    int getSelectedCardIndex() const;
    void resetSelectedCard();

private:
    GameLogic &gameLogic;
    sf::RenderWindow &window;
    sf::Font font;
    int selectedCardIndex;
    std::unordered_map<std::string, sf::Texture> cardTextures;
    sf::FloatRect callBankerButtonRect;
    sf::FloatRect robBankerButtonRect;
    sf::FloatRect passBankerButtonRect;
    sf::FloatRect continueButtonRect;
    sf::FloatRect backToMenuButtonRect;
    sf::FloatRect confirmArrangementButtonRect;
    std::string textureLoadStatus;
    bool fontLoaded;

    // ── 背景 ──
    sf::Texture tableBgTexture;
    std::optional<sf::Sprite> tableBgSprite;

    // ── 发牌动画 ──
    sf::Clock dealAnimClock;
    bool dealAnimActive = false;
    float dealAnimElapsed = 0.f;
    static constexpr float DEAL_INTERVAL = 0.12f; // 每张牌间隔（秒）
    static constexpr float DEAL_DURATION = 0.25f; // 单张飞入时长（秒）

    // ── 结算动画 ──
    sf::Clock settleAnimClock;
    bool settleAnimActive = false;
    float settleAnimElapsed = 0.f;

    // ── 叫庄倒计时 ──
    sf::Clock bidTimerClock;

    // ── 卡牌阴影纹理（预生成）──
    sf::Texture cardShadowTex;
    bool cardShadowReady = false;

    // ── 卡背纹理（使用真实牌面图片）──
    sf::Texture cardBackTex;
    bool cardBackReady = false;

    // ── 按钮纹理（assets/TYPE/Button2.png）──
    sf::Texture buttonTexture;
    bool buttonTexReady = false;
    sf::IntRect buttonCropRect; // 裁剪透明区域后的有效区域

    // ── 准备！图片（assets/TYPE/GetReady.png）──
    sf::Texture readyTexture;
    bool readyTexReady = false;
    sf::IntRect readyCropRect;

    // ── 牛几图片纹理（assets/TYPE/N{0-10}.png）──
    std::unordered_map<int, sf::Texture> bullTypeTextures;
    bool bullTypeLoaded = false;

    void drawBackground();
    void drawPlayers();
    void drawTableCards();
    void drawBiddingButtons();
    void drawSettleButtons();
    void drawDealAnimation();
    void drawSettleCelebration();
    void drawGameInfo();
    void drawInstructions();
    int calculateHandValue(const Player &player) const;

    void loadFont();
    void loadCardTextures();
    void loadBackground();
    void loadBullTypeTextures();
    void loadButtonTexture();
    void loadReadyTexture();
    void createCardShadow();
    std::string getCardTextureKey(const Card &card) const;
    void drawCard(const Card &card, float x, float y, float rotation = 0.f, bool selected = false, float scale = 1.f);
    void drawCardBack(float x, float y, float rotation = 0.f, float scale = 1.f);
    void drawBullTypeImage(int quality, float x, float y, float scale = 1.2f);
    void drawBullTypeImages();
    void drawPlayerStackedHand(const Player &player, float startX, float startY, int playerIndex, bool vertical);
    void drawArrangementHand();    // 组牌阶段：5张牌摊开
    void drawArrangementButtons(); // 组牌确认按钮
    void drawRoundedButton(const std::string &text, float x, float y, float w, float h,
                           sf::Color fillColor, sf::Color textColor, sf::Color borderColor,
                           float borderWidth = 2.5f, bool glow = false);
    void drawTexturedButton(const std::string &text, float x, float y, float w, float h,
                            sf::Color textColor);

    // 组牌阶段状态
    std::vector<int> arrangementSelectedIndices; // 最多3项
};

#endif // GAME_SCREEN_H
