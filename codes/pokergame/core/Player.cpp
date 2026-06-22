#include "Player.h"
#include <iostream>

Player::Player(const std::string &name, bool isAI)
    : name(name), isAI(isAI), score(1000), handQuality(0), banker(false), oddsMultiplier(1) {}

void Player::addCard(const Card &card)
{
    hand.push_back(card);
}

void Player::removeCard(int index)
{
    if (index >= 0 && index < static_cast<int>(hand.size()))
    {
        hand.erase(hand.begin() + index);
    }
}

const Card &Player::getCard(int index) const
{
    return hand[index];
}

int Player::getCardCount() const
{
    return hand.size();
}

std::vector<Card> Player::getAllCards() const
{
    return hand;
}

std::string Player::getName() const
{
    return name;
}

bool Player::isAIPlayer() const
{
    return isAI;
}

void Player::addScore(int points)
{
    score += points;
}

void Player::deductChips(int amount)
{
    score -= amount;
}

int Player::getScore() const
{
    return score;
}

void Player::setScore(int s)
{
    score = s;
}

void Player::resetHand()
{
    hand.clear();
}

void Player::resetForNewRound()
{
    handQuality = 0;
    banker = false;
    oddsMultiplier = 1;
}

void Player::setHandQuality(int quality)
{
    handQuality = quality;
}

int Player::getHandQuality() const
{
    return handQuality;
}

void Player::setBanker(bool bankerFlag)
{
    banker = bankerFlag;
}

bool Player::isBanker() const
{
    return banker;
}

void Player::setOddsMultiplier(int multiplier)
{
    oddsMultiplier = multiplier;
}

int Player::getOddsMultiplier() const
{
    return oddsMultiplier;
}

void Player::displayCards() const
{
    std::cout << name << " cards: ";
    for (const auto &card : hand)
    {
        std::cout << card.toString() << " ";
    }
    std::cout << std::endl;
}
