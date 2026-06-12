#ifndef PLAYER_H
#define PLAYER_H

#include "Card.h"
#include <vector>
#include <string>

class Player {
public:
    Player(const std::string& name, bool isAI = false);
    
    void addCard(const Card& card);
    void removeCard(int index);
    const Card& getCard(int index) const;
    int getCardCount() const;
    std::vector<Card> getAllCards() const;
    
    std::string getName() const;
    bool isAIPlayer() const;
    void addScore(int points);
    void deductChips(int amount);
    int getScore() const;
    void resetHand();
    void resetForNewRound();

    void setHandQuality(int quality);
    int getHandQuality() const;
    void setBanker(bool banker);
    bool isBanker() const;
    void setOddsMultiplier(int multiplier);
    int getOddsMultiplier() const;
    
    void displayCards() const;

private:
    std::string name;
    bool isAI;
    std::vector<Card> hand;
    int score;
    int handQuality;
    bool banker;
    int oddsMultiplier;
};

#endif // PLAYER_H
