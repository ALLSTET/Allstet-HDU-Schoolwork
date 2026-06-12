#include "GameLogic.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <random>
#include <sstream>

GameLogic::GameLogic()
    : currentPlayerIndex(0), currentBidPosition(0), currentBankerCandidate(-1), currentOddsMultiplier(1), biddingActive(false), bankerIndex(-1), lastCallerIndex(-1), secondDealDone(false), pot(0), roundNumber(0) {
    players.push_back(Player("Player1(You)", false));
    players.push_back(Player("player2(AI)", true));
    players.push_back(Player("player3(AI)", true));
    players.push_back(Player("player4(AI)", true));
    lastRoundWinner = -1;
}

void GameLogic::initializeGame() {
    deck.reset();
    currentPlayerIndex = 0;
    tableCards.clear();
    lastRoundWinner = -1;
    lastSettleResult.clear();
    playerOrder.clear();
    currentBidPosition = 0;
    currentBankerCandidate = -1;
    currentOddsMultiplier = 1;
    biddingActive = false;
    bankerIndex = -1;
    lastCallerIndex = -1;
    secondDealDone = false;
    pot = 0;
    roundNumber = 0;
    for (auto& player : players) {
        player.resetHand();
        player.resetForNewRound();
    }
}

void GameLogic::dealCardsPhase1() {
    // 前4张牌，用于叫庄/抢庄阶段
    for (int i = 0; i < 4; i++) {
        for (auto& player : players) {
            player.addCard(deck.drawCard());
        }
    }
}

void GameLogic::dealCardsPhase2() {
    // 第5张牌，叫庄结束后发
    for (auto& player : players) {
        player.addCard(deck.drawCard());
    }
    evaluateHandQualities();  // 5张齐全后再评估牌质
    secondDealDone = true;
}

bool GameLogic::isSecondDealDone() const {
    return secondDealDone;
}

void GameLogic::startBiddingPhase() {
    shufflePlayerOrder();
    currentBidPosition = 0;
    currentBankerCandidate = -1;
    currentOddsMultiplier = 1;
    biddingActive = true;
    bankerIndex = -1;
    lastCallerIndex = -1;
    for (auto& player : players) {
        player.setBanker(false);
        player.setOddsMultiplier(1);
    }
}

void GameLogic::processAIBid() {
    if (!biddingActive) {
        return;
    }

    int bidder = getCurrentBidderIndex();
    if (bidder < 0 || bidder >= static_cast<int>(players.size()) || !players[bidder].isAIPlayer()) {
        return;
    }

    int quality = players[bidder].getHandQuality();
    bool acted = false;

    if (!hasSomeoneCalledBanker()) {
        if (quality > 5) {
            playerCallBanker(bidder);
            acted = true;
            std::cout << players[bidder].getName() << " calls banker with quality " << quality << "." << std::endl;
        }
    } else {
        int callerQuality = players[lastCallerIndex].getHandQuality();
        if (quality > callerQuality) {
            int bidderOrder = getOrderPosition(bidder);
            int callerOrder = getOrderPosition(lastCallerIndex);
            double chance = bidderOrder < callerOrder ? 0.7 : 0.2;
            if ((rand() % 100) < static_cast<int>(chance * 100)) {
                playerRobBanker(bidder);
                acted = true;
                std::cout << players[bidder].getName() << " robs banker with quality " << quality << "." << std::endl;
            }
        }
    }

    if (!acted) {
        std::cout << players[bidder].getName() << " passes bidding." << std::endl;
        advanceBidder();
    }
}

void GameLogic::evaluateHandQualities() {
    for (auto& player : players) {
        player.setHandQuality(evaluateHandQuality(player));
    }
}

// ==================== 斗牛牌型判断 ====================
int GameLogic::evaluateHandQuality(const Player& player) const {
    const auto cards = player.getAllCards();
    if (cards.size() != 5) return BULL_NONE;

    // 收集每张牌的斗牛值（J/Q/K=10, A=1, 其他=面值）
    int vals[5];
    int ranks[5];
    int total = 0;
    for (int i = 0; i < 5; i++) {
        vals[i] = cards[i].getValue();   // J/Q/K→10, A→1
        ranks[i] = static_cast<int>(cards[i].getRank());
        total += vals[i];
    }

    // --- 五小牛：5张都 < 5 且总和 ≤ 10 ---
    {
        bool allSmall = true;
        for (int i = 0; i < 5; i++) {
            if (vals[i] >= 5) { allSmall = false; break; }
        }
        if (allSmall && total <= 10) return FIVE_SMALL;
    }

    // --- 炸弹：4张相同rank ---
    for (int i = 0; i < 5; i++) {
        int cnt = 0;
        for (int j = 0; j < 5; j++) {
            if (ranks[j] == ranks[i]) cnt++;
        }
        if (cnt >= 4) return BOMB;
    }

    // --- 五花牛：5张全是J/Q/K ---
    {
        bool allFace = true;
        for (int i = 0; i < 5; i++) {
            int r = ranks[i];
            if (r != JACK && r != QUEEN && r != KING) { allFace = false; break; }
        }
        if (allFace) return FIVE_FLOWER;
    }

    // --- 普通有牛/没牛：找任意3张之和为10的倍数 ---
    for (int i = 0; i < 5; i++) {
        for (int j = i + 1; j < 5; j++) {
            for (int k = j + 1; k < 5; k++) {
                int sum3 = vals[i] + vals[j] + vals[k];
                if (sum3 % 10 == 0) {
                    int remain = (total - sum3) % 10;
                    return (remain == 0) ? BULL_BULL : remain;  // remain=1~9 即牛1~牛9
                }
            }
        }
    }

    return BULL_NONE;  // 没牛
}

void GameLogic::shufflePlayerOrder() {
    playerOrder.clear();
    for (int i = 0; i < static_cast<int>(players.size()); ++i) {
        playerOrder.push_back(i);
    }
    std::shuffle(playerOrder.begin(), playerOrder.end(), std::default_random_engine(static_cast<unsigned>(std::rand())));
}

int GameLogic::getOrderPosition(int playerIndex) const {
    for (int i = 0; i < static_cast<int>(playerOrder.size()); ++i) {
        if (playerOrder[i] == playerIndex) {
            return i;
        }
    }
    return -1;
}

void GameLogic::advanceBidder() {
    currentBidPosition++;
    if (currentBidPosition >= static_cast<int>(playerOrder.size())) {
        finishBidding();
    }
}

void GameLogic::finishBidding() {
    biddingActive = false;
    if (playerOrder.empty()) {
        std::cerr << "ERROR: playerOrder is empty in finishBidding!" << std::endl;
        bankerIndex = 0;
        currentOddsMultiplier = 1;
    } else if (currentBankerCandidate == -1) {
        int randomIndex = rand() % static_cast<int>(playerOrder.size());
        bankerIndex = playerOrder[randomIndex];
        currentOddsMultiplier = 1;
        std::cout << "No one called banker. " << players[bankerIndex].getName() << " becomes banker randomly." << std::endl;
    } else {
        bankerIndex = currentBankerCandidate;
        std::cout << players[bankerIndex].getName() << " becomes banker with odds x" << currentOddsMultiplier << "." << std::endl;
    }
    for (auto& player : players) {
        player.setBanker(false);
        player.setOddsMultiplier(1);
    }
    if (bankerIndex >= 0 && bankerIndex < static_cast<int>(players.size())) {
        players[bankerIndex].setBanker(true);
        players[bankerIndex].setOddsMultiplier(currentOddsMultiplier);
    }
}

bool GameLogic::isBiddingActive() const {
    return biddingActive;
}

int GameLogic::getCurrentBidderIndex() const {
    if (!biddingActive || currentBidPosition < 0 || currentBidPosition >= static_cast<int>(playerOrder.size())) {
        return -1;
    }
    return playerOrder[currentBidPosition];
}

bool GameLogic::canPlayerCallBanker(int playerIndex) const {
    return biddingActive && !hasSomeoneCalledBanker() && getCurrentBidderIndex() == playerIndex;
}

bool GameLogic::canPlayerRobBanker(int playerIndex) const {
    return biddingActive && hasSomeoneCalledBanker() && getCurrentBidderIndex() == playerIndex;
}

void GameLogic::playerCallBanker(int playerIndex) {
    if (!canPlayerCallBanker(playerIndex)) {
        return;
    }
    currentBankerCandidate = playerIndex;
    currentOddsMultiplier = 2;
    lastCallerIndex = playerIndex;
    advanceBidder();
}

void GameLogic::playerRobBanker(int playerIndex) {
    if (!canPlayerRobBanker(playerIndex)) {
        return;
    }
    currentBankerCandidate = playerIndex;
    currentOddsMultiplier *= 2;
    lastCallerIndex = playerIndex;
    advanceBidder();
}

void GameLogic::playerPassBanker(int playerIndex) {
    if (!biddingActive || getCurrentBidderIndex() != playerIndex) {
        return;
    }
    std::cout << players[playerIndex].getName() << " passes." << std::endl;
    advanceBidder();
}

bool GameLogic::hasSomeoneCalledBanker() const {
    return currentBankerCandidate != -1;
}

int GameLogic::getCurrentOddsMultiplier() const {
    return currentOddsMultiplier;
}

int GameLogic::getBankerIndex() const {
    return bankerIndex;
}

const std::vector<int>& GameLogic::getPlayerOrder() const {
    return playerOrder;
}

int GameLogic::getLastCallerIndex() const {
    return lastCallerIndex;
}

std::string GameLogic::getBullTypeName(int quality) {
    switch (quality) {
        case FIVE_SMALL:  return "Five Small";
        case BOMB:        return "Bomb";
        case FIVE_FLOWER: return "Five Flower";
        case BULL_BULL:   return "Bull Bull";
        case BULL_NONE:   return "No Bull";
        default:
            if (quality >= BULL_1 && quality <= BULL_9)
                return "Bull " + std::to_string(quality);
            return "Unknown";
    }
}

int GameLogic::getPayMultiplier(int quality) {
    if (quality >= FIVE_SMALL)  return 5;
    if (quality >= BOMB)       return 4;
    if (quality >= FIVE_FLOWER)return 4;
    if (quality >= BULL_BULL)  return 3;
    if (quality >= BULL_7)     return 2;
    return 1;  // 没牛~牛6 都是1倍
}

int GameLogic::getPot() const {
    return pot;
}

int GameLogic::getRoundNumber() const {
    return roundNumber;
}

const std::string& GameLogic::getLastSettleResult() const {
    return lastSettleResult;
}

void GameLogic::deductAnte() {
    for (auto& player : players) {
        player.deductChips(ANTE);
        pot += ANTE;
    }
    std::cout << "Ante deducted: " << ANTE << " x " << players.size()
              << " = " << (ANTE * static_cast<int>(players.size()))
              << " chips in pot." << std::endl;
}

void GameLogic::startNewRound() {
    deck.reset();
    tableCards.clear();
    lastRoundWinner = -1;
    lastSettleResult.clear();
    playerOrder.clear();
    currentBidPosition = 0;
    currentBankerCandidate = -1;
    currentOddsMultiplier = 1;
    biddingActive = false;
    bankerIndex = -1;
    lastCallerIndex = -1;
    secondDealDone = false;
    roundNumber++;

    for (auto& player : players) {
        player.resetHand();
        player.resetForNewRound();
    }

    deductAnte();         // 扣底注
    dealCardsPhase1();    // 发4张
    startBiddingPhase();  // 开始叫庄

    std::cout << "\n========== ROUND " << roundNumber << " START ==========" << std::endl;
    std::cout << "Pot: " << pot << " chips" << std::endl;
}

void GameLogic::settleRound() {
    if (bankerIndex < 0 || bankerIndex >= static_cast<int>(players.size())) {
        std::cerr << "ERROR: Invalid banker in settleRound!" << std::endl;
        return;
    }

    std::ostringstream result;
    result << "Round " << roundNumber << " | ";

    const auto& banker = players[bankerIndex];
    int bankerQuality = banker.getHandQuality();
    int odds = currentOddsMultiplier;

    result << "Banker: " << banker.getName()
           << " [" << getBullTypeName(bankerQuality) << "] Odds x" << odds << " | ";

    for (int i = 0; i < static_cast<int>(players.size()); i++) {
        if (i == bankerIndex) continue;  // 庄家不和自己比

        const auto& player = players[i];
        int playerQuality = player.getHandQuality();

        if (bankerQuality > playerQuality) {
            // 庄家赢
            int payMult = getPayMultiplier(bankerQuality);
            int winAmount = ANTE * odds * payMult;
            players[bankerIndex].addScore(winAmount);
            players[i].deductChips(winAmount);
            std::cout << banker.getName() << "(" << getBullTypeName(bankerQuality)
                      << ") beats " << player.getName() << "(" << getBullTypeName(playerQuality)
                      << ") +" << winAmount << std::endl;
        } else if (playerQuality > bankerQuality) {
            // 闲家赢
            int payMult = getPayMultiplier(playerQuality);
            int winAmount = ANTE * odds * payMult;
            players[i].addScore(winAmount);
            players[bankerIndex].deductChips(winAmount);
            std::cout << player.getName() << "(" << getBullTypeName(playerQuality)
                      << ") beats " << banker.getName() << "(" << getBullTypeName(bankerQuality)
                      << ") +" << winAmount << std::endl;
        } else {
            // 平局：比较最大牌
            std::cout << banker.getName() << " and " << player.getName()
                      << " tie (both " << getBullTypeName(bankerQuality) << ")" << std::endl;
        }
    }

    result << "Pot: " << pot;
    lastSettleResult = result.str();
    std::cout << lastSettleResult << std::endl;
    pot = 0;  // 清空底池
}

void GameLogic::playRound() {
    for (size_t i = 0; i < players.size(); i++) {
        currentPlayerIndex = i;
        if (players[i].isAIPlayer()) {
            // AI逻辑：随机选择一张卡牌
            if (players[i].getCardCount() > 0) {
                int randomIndex = rand() % players[i].getCardCount();
                tableCards.push_back(players[i].getCard(randomIndex));
                players[i].removeCard(randomIndex);
            }
        }
    }
    determineWinner();
}

void GameLogic::displayGameState() const {
    std::cout << "\n========== GAME STATE ==========" << std::endl;
    for (const auto& player : players) {
        player.displayCards();
    }
    std::cout << "Table cards: ";
    for (const auto& card : tableCards) {
        std::cout << card.toString() << " ";
    }
    std::cout << std::endl;
    std::cout << "============================\n" << std::endl;
}

void GameLogic::determineWinner() {
    if (tableCards.empty()) return;

    std::cout << "\nRound cards: ";
    for (const auto& card : tableCards) {
        std::cout << card.toString() << " ";
    }
    std::cout << std::endl;

    // Scoring rule:
    // - Check each player whether any 3-card combination sums to 10.
    // - If exactly one player has such a combination, that player wins.
    // - If multiple players have combinations, the player with the largest remaining sum (total - 10) wins.
    std::vector<int> candidates;
    std::vector<int> remainingSums(players.size(), 0);
    for (size_t i = 0; i < players.size(); ++i) {
        if (hasThreeSumToTen(players[i])) {
            candidates.push_back(static_cast<int>(i));
            remainingSums[i] = remainingAfterThreeToTen(players[i]);
        }
    }

    if (candidates.empty()) {
        std::cout << "No player can make 10 with three cards." << std::endl;
        lastRoundWinner = -1;
    } else if (candidates.size() == 1) {
        int winnerIdx = candidates[0];
        std::cout << players[winnerIdx].getName() << " wins (only one made 10)." << std::endl;
        players[winnerIdx].addScore(1);
        lastRoundWinner = winnerIdx;
    } else {
        // multiple candidates -> compare remaining sums
        int bestIdx = candidates[0];
        int bestSum = remainingSums[bestIdx];
        for (size_t k = 1; k < candidates.size(); ++k) {
            int idx = candidates[k];
            if (remainingSums[idx] > bestSum) {
                bestSum = remainingSums[idx];
                bestIdx = idx;
            }
        }
        std::cout << players[bestIdx].getName() << " wins (largest remaining sum: " << bestSum << ")." << std::endl;
        players[bestIdx].addScore(1);
        lastRoundWinner = bestIdx;
    }

    tableCards.clear();
}

bool GameLogic::hasThreeSumToTen(const Player& player) const {
    const auto hand = player.getAllCards();
    int n = static_cast<int>(hand.size());
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            for (int k = j + 1; k < n; ++k) {
                int sum = hand[i].getValue() + hand[j].getValue() + hand[k].getValue();
                if (sum == 10) return true;
            }
        }
    }
    return false;
}

int GameLogic::remainingAfterThreeToTen(const Player& player) const {
    const auto hand = player.getAllCards();
    int total = 0;
    for (const auto& c : hand) total += c.getValue();
    // If player can make 10, remaining is total - 10.
    if (hasThreeSumToTen(player)) return total - 10;
    return total;
}

int GameLogic::getLastRoundWinner() const {
    return lastRoundWinner;
}

void GameLogic::clearLastRoundWinner() {
    lastRoundWinner = -1;
}

const std::vector<Player>& GameLogic::getPlayers() const {
    return players;
}

int GameLogic::getCurrentPlayerIndex() const {
    return currentPlayerIndex;
}

void GameLogic::setCurrentPlayerIndex(int index) {
    currentPlayerIndex = index;
}

bool GameLogic::isGameOver() const {
    for (const auto& player : players) {
        if (player.getCardCount() > 0) {
            return false;
        }
    }
    return true;
}

std::vector<Card> GameLogic::getTableCards() const {
    return tableCards;
}

void GameLogic::addTableCard(const Card& card) {
    tableCards.push_back(card);
}

void GameLogic::clearTableCards() {
    tableCards.clear();
}

int GameLogic::calculateHandValue(const std::vector<Card>& cards) const {
    int value = 0;
    for (const auto& card : cards) {
        value += card.getValue();
    }
    return value;
}
