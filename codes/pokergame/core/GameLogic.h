#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "Player.h"
#include "Deck.h"
#include <vector>
#include <string>

// 斗牛牌型等级
enum BullType : int {
    BULL_NONE   = 0,   // 没牛
    BULL_1      = 1,   // 牛1
    BULL_2      = 2,   // 牛2
    BULL_3      = 3,   // 牛3
    BULL_4      = 4,   // 牛4
    BULL_5      = 5,   // 牛5
    BULL_6      = 6,   // 牛6
    BULL_7      = 7,   // 牛7
    BULL_8      = 8,   // 牛8
    BULL_9      = 9,   // 牛9
    BULL_BULL   = 10,  // 牛牛
    FIVE_FLOWER = 11,  // 五花牛（全J/Q/K）
    BOMB        = 12,  // 炸弹（4张相同）
    FIVE_SMALL  = 13   // 五小牛（5张都<5且总和≤10）
};

constexpr int ANTE = 10;  // 底注

class GameLogic {
public:
    GameLogic();
    
    void initializeGame();
    void dealCardsPhase1();   // 前4张
    void dealCardsPhase2();   // 第5张 + 评估牌质
    bool isSecondDealDone() const;
    void startBiddingPhase();
    void processAIBid();
    void playRound();
    void displayGameState() const;
    void determineWinner();
    
    const std::vector<Player>& getPlayers() const;
    int getCurrentPlayerIndex() const;
    void setCurrentPlayerIndex(int index);
    
    bool isBiddingActive() const;
    int getCurrentBidderIndex() const;
    bool canPlayerCallBanker(int playerIndex) const;
    bool canPlayerRobBanker(int playerIndex) const;
    void playerCallBanker(int playerIndex);
    void playerRobBanker(int playerIndex);
    void playerPassBanker(int playerIndex);
    bool hasSomeoneCalledBanker() const;
    int getCurrentOddsMultiplier() const;
    int getBankerIndex() const;
    const std::vector<int>& getPlayerOrder() const;
    int getLastCallerIndex() const;
    
    // 斗牛结算
    void settleRound();
    void startNewRound();
    int getPot() const;
    int getRoundNumber() const;
    static std::string getBullTypeName(int quality);
    static int getPayMultiplier(int quality);
    
    bool isGameOver() const;
    std::vector<Card> getTableCards() const;
    void addTableCard(const Card& card);
    void clearTableCards();
    
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
    int pot;            // 底池
    int roundNumber;    // 当前局数
    
    int calculateHandValue(const std::vector<Card>& cards) const;
    int evaluateHandQuality(const Player& player) const;
    bool hasThreeSumToTen(const Player& player) const;
    int remainingAfterThreeToTen(const Player& player) const;
    int lastRoundWinner;
    std::string lastSettleResult;  // 结算结果描述
    
    void shufflePlayerOrder();
    void evaluateHandQualities();
    void deductAnte();
    void advanceBidder();
    void finishBidding();
    int getOrderPosition(int playerIndex) const;

public:
    int getLastRoundWinner() const;
    const std::string& getLastSettleResult() const;
    void clearLastRoundWinner();
};

#endif // GAME_LOGIC_H
