#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "Player.h"
#include "Deck.h"
#include <vector>
#include <string>

// 斗牛牌型等级
enum BullType : int
{
    BULL_NONE = 0,    // 没牛
    BULL_1 = 1,       // 牛1
    BULL_2 = 2,       // 牛2
    BULL_3 = 3,       // 牛3
    BULL_4 = 4,       // 牛4
    BULL_5 = 5,       // 牛5
    BULL_6 = 6,       // 牛6
    BULL_7 = 7,       // 牛7
    BULL_8 = 8,       // 牛8
    BULL_9 = 9,       // 牛9
    BULL_BULL = 10,   // 牛牛
    FIVE_FLOWER = 11, // 五花牛（全J/Q/K）
    BOMB = 12,        // 炸弹（4张相同）
    FIVE_SMALL = 13   // 五小牛（5张都<5且总和≤10）
};

// ANTE 已改为成员变量 anteAmount，根据难度动态设置

// AI 难度（与 StartScreen::Difficulty 对应）
enum AIDifficulty : int
{
    AI_EASY = 0,
    AI_NORMAL = 1,
    AI_HARD = 2
};

class GameLogic
{
public:
    GameLogic();

    void initializeGame();
    void dealCardsPhase1(); // 前4张
    void dealCardsPhase2(); // 第5张 + 评估牌质
    bool isSecondDealDone() const;
    void startBiddingPhase();
    void processAIBid();
    void playRound();
    void displayGameState() const;
    void determineWinner();

    const std::vector<Player> &getPlayers() const;
    int getCurrentPlayerIndex() const;
    void setCurrentPlayerIndex(int index);

    // ── 难度 ──
    void setAIDifficulty(AIDifficulty diff);
    AIDifficulty getAIDifficulty() const;

    bool isBiddingActive() const;
    int getCurrentBidderIndex() const;
    bool canPlayerCallBanker(int playerIndex) const;
    bool canPlayerRobBanker(int playerIndex) const;
    void playerCallBanker(int playerIndex);
    void playerRobBanker(int playerIndex);
    void playerPassBanker(int playerIndex);
    bool hasSomeoneCalledBanker() const;
    int getCurrentOddsMultiplier() const;
    int getEffectiveOdds() const;
    int getAnte() const;
    int getBankerIndex() const;
    const std::vector<int> &getPlayerOrder() const;
    int getLastCallerIndex() const;
    bool isRobBackPhase() const;

    // 斗牛结算
    void settleRound();
    void setPlayerScore(int index, int score);
    void startNewRound();
    int getPot() const;
    int getRoundNumber() const;
    static std::string getBullTypeName(int quality);
    static int getPayMultiplier(int quality);

    bool isGameOver() const;
    std::vector<Card> getTableCards() const;
    void addTableCard(const Card &card);
    void clearTableCards();

    // ── 组牌阶段 ──
    void startCardArrangementPhase();
    bool isCardArrangementActive() const;
    void finishCardArrangement();
    int evaluatePlayerArrangement(int playerIndex, const std::vector<int> &selectedIndices) const;
    void aiAutoArrange(int playerIndex);
    void confirmPlayerArrangement(int playerIndex, const std::vector<int> &selectedIndices);
    const std::vector<int> &getAiSelectedIndices() const;

private:
    std::vector<Player> players;
    Deck deck;
    int currentPlayerIndex;
    std::vector<Card> tableCards;

    std::vector<int> playerOrder;
    int currentBidPosition;
    int currentBankerCandidate;
    int currentOddsMultiplier;
    bool biddingActive;
    int bankerIndex;
    int lastCallerIndex;
    bool secondDealDone;
    bool cardArrangementActive;
    int originalCallerIndex; // 首个叫庄者（被抢后可反抢）
    bool robBackPhase;       // 反抢回合进行中
    std::vector<int> aiSelectedIndices;
    AIDifficulty aiDifficulty;
    int pot;         // 底池
    int roundNumber; // 当前局数
    int anteAmount;  // 底注（按难度动态变化）

    int calculateHandValue(const std::vector<Card> &cards) const;
    int evaluateHandQuality(const Player &player) const;
    int evaluate4CardStrength(const Player &player) const; // 4张牌时的手牌强度评估（叫庄用）
    bool hasThreeSumToTen(const Player &player) const;
    int remainingAfterThreeToTen(const Player &player) const;
    int lastRoundWinner;
    std::string lastSettleResult; // 结算结果描述

    void shufflePlayerOrder();
    void evaluateHandQualities();
    void deductAnte();
    void advanceBidder();
    void finishBidding();
    int getOrderPosition(int playerIndex) const;

public:
    int getLastRoundWinner() const;
    const std::string &getLastSettleResult() const;
    void clearLastRoundWinner();
};

#endif // GAME_LOGIC_H
