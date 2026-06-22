#include "GameLogic.h"
#include "../persistence/MoneySave.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <random>
#include <sstream>

GameLogic::GameLogic()
    : currentPlayerIndex(0), currentBidPosition(0), currentBankerCandidate(-1), currentOddsMultiplier(1), biddingActive(false), bankerIndex(-1), lastCallerIndex(-1), secondDealDone(false), cardArrangementActive(false), aiDifficulty(AI_NORMAL), pot(0), roundNumber(0), anteAmount(100)
{
    // 随机外国人名表
    static const char *namePool[] = {
        "Alex", "Blake", "Casey", "Dana", "Drew", "Eddie", "Frank",
        "Glenn", "Henry", "Ivy", "Jamie", "Kyle", "Liam", "Morgan",
        "Noah", "Owen", "Pat", "Quinn", "Ryan", "Sam", "Terry",
        "Uma", "Vic", "Wes", "Xena", "Yuri", "Zoe", "Alice",
        "Ben", "Clara", "David", "Eva", "Grace", "Isaac", "Jade"};
    constexpr int poolSize = sizeof(namePool) / sizeof(namePool[0]);

    // 用 Fisher-Yates 打乱名字表以确保不重复
    int picks[3];
    for (int i = 0; i < 3; i++)
    {
        int r;
        bool dup;
        do
        {
            r = rand() % poolSize;
            dup = false;
            for (int j = 0; j < i; j++)
            {
                if (picks[j] == r)
                {
                    dup = true;
                    break;
                }
            }
        } while (dup);
        picks[i] = r;
    }

    players.push_back(Player("You", false));
    players.push_back(Player(namePool[picks[0]], true));
    players.push_back(Player(namePool[picks[1]], true));
    players.push_back(Player(namePool[picks[2]], true));
    lastRoundWinner = -1;
}

void GameLogic::initializeGame()
{
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
    for (auto &player : players)
    {
        player.resetHand();
        player.resetForNewRound();
    }
}

void GameLogic::dealCardsPhase1()
{
    // 前4张牌，用于叫庄/抢庄阶段
    for (int i = 0; i < 4; i++)
    {
        for (auto &player : players)
        {
            player.addCard(deck.drawCard());
        }
    }
}

void GameLogic::dealCardsPhase2()
{
    // 第5张牌，叫庄结束后发
    for (auto &player : players)
    {
        player.addCard(deck.drawCard());
    }
    evaluateHandQualities(); // 5张齐全后再评估牌质
    secondDealDone = true;
}

bool GameLogic::isSecondDealDone() const
{
    return secondDealDone;
}

void GameLogic::startBiddingPhase()
{
    shufflePlayerOrder();
    currentBidPosition = 0;
    currentBankerCandidate = -1;
    currentOddsMultiplier = (aiDifficulty == AI_HARD) ? 16 : 1;
    biddingActive = true;
    bankerIndex = -1;
    lastCallerIndex = -1;
    originalCallerIndex = -1;
    robBackPhase = false;
    for (auto &player : players)
    {
        player.setBanker(false);
        player.setOddsMultiplier(1);
    }
}

void GameLogic::processAIBid()
{
    if (!biddingActive)
        return;

    int bidder = getCurrentBidderIndex();
    if (bidder < 0 || bidder >= static_cast<int>(players.size()) || !players[bidder].isAIPlayer())
        return;

    // 用4张牌的手牌强度评估（叫庄阶段只有4张牌）
    int myStrength = evaluate4CardStrength(players[bidder]);
    bool acted = false;
    int bidderOrder = getOrderPosition(bidder);

    if (!hasSomeoneCalledBanker())
    {
        // ── 叫庄（无人叫过）──
        bool shouldCall = false;
        switch (aiDifficulty)
        {
        case AI_EASY:
            // 简单：手牌很强才叫（强度>=55，对应有牛或高价值牌）
            shouldCall = (myStrength >= 55);
            break;
        case AI_NORMAL:
            // 普通：手牌中上就叫（强度>=38）
            shouldCall = (myStrength >= 38);
            break;
        case AI_HARD:
            // 困难：手牌还行就叫，且位置靠后更激进（抢庄权）
            shouldCall = (myStrength >= 25) ||
                         (bidderOrder >= 2 && myStrength >= 18);
            break;
        }

        if (shouldCall)
        {
            playerCallBanker(bidder);
            acted = true;
            std::cout << players[bidder].getName() << " calls banker (strength " << myStrength
                      << ", diff=" << aiDifficulty << ")" << std::endl;
        }
    }
    else
    {
        // ── 抢庄 / 反抢（已有人叫庄）──
        int callerStrength = evaluate4CardStrength(players[lastCallerIndex]);
        int callerOrder = getOrderPosition(lastCallerIndex);
        int odds = currentOddsMultiplier;

        bool shouldRob = false;
        int robChance = 0;

        switch (aiDifficulty)
        {
        case AI_EASY:
        {
            // 简单：手牌明显强于当前庄家才抢，且赔率不太高
            int gap = myStrength - callerStrength;
            if (gap >= 12 && odds <= 4)
            {
                robChance = bidderOrder < callerOrder ? 40 : 15;
            }
            break;
        }
        case AI_NORMAL:
        {
            // 普通：手牌更强就考虑抢，位置靠前更积极
            int gap = myStrength - callerStrength;
            if (gap >= 3 && odds <= 8)
            {
                robChance = bidderOrder < callerOrder ? 75 : 35;
            }
            else if (gap >= -3 && odds <= 4 && bidderOrder < callerOrder)
            {
                // 手牌接近但位置靠前，也有低概率抢
                robChance = 20;
            }
            break;
        }
        case AI_HARD:
        {
            // 困难：非常激进，手牌接近就抢，位置优势时几乎必抢
            int gap = myStrength - callerStrength;
            if (gap >= -8 && odds <= 16)
            {
                robChance = bidderOrder < callerOrder ? 92 : 60;
                // 如果已到高赔率但手牌很强，仍可能抢
                if (odds >= 8 && gap < 5)
                    robChance = bidderOrder < callerOrder ? 50 : 20;
            }
            // 困难AI还会因为策略原因抢庄（增加赔率压力）
            if (!shouldRob && odds <= 2 && myStrength >= 30 && bidderOrder == 0)
                robChance = 25;
            break;
        }
        }

        // 反抢回合：原叫庄者更积极守庄（小幅加成）
        if (robBackPhase && robChance > 0)
            robChance = std::min(robChance + 10, 95);

        if (robChance > 0 && (rand() % 100) < robChance)
        {
            playerRobBanker(bidder);
            acted = true;
            const char *label = robBackPhase ? "counter-robs" : "robs banker";
            std::cout << players[bidder].getName() << " " << label
                      << " (my=" << myStrength << " vs=" << callerStrength
                      << " chance=" << robChance
                      << "%, odds→x" << currentOddsMultiplier << ")" << std::endl;
        }
    }

    if (!acted)
    {
        std::cout << players[bidder].getName() << " passes bidding (strength " << myStrength << ")." << std::endl;
        advanceBidder();
    }
}

void GameLogic::evaluateHandQualities()
{
    for (auto &player : players)
    {
        player.setHandQuality(evaluateHandQuality(player));
    }
}

// ==================== 4张牌手牌强度评估（叫庄/抢庄用）====================
// 返回 0-100 的强度评分，用于AI决策
int GameLogic::evaluate4CardStrength(const Player &player) const
{
    const auto &cards = player.getAllCards();
    if (cards.size() != 4)
        return 0;

    int vals[4];
    int ranks[4];
    for (int i = 0; i < 4; i++)
    {
        vals[i] = cards[i].getValue();
        ranks[i] = static_cast<int>(cards[i].getRank());
    }

    int score = 0;

    // 1. 已有3张凑成10的倍数 → 基本必抢（70-79分）
    for (int i = 0; i < 4; i++)
    {
        for (int j = i + 1; j < 4; j++)
        {
            for (int k = j + 1; k < 4; k++)
            {
                if ((vals[i] + vals[j] + vals[k]) % 10 == 0)
                {
                    // 剩余1张的斗牛值作为加分
                    int remain = 0;
                    for (int x = 0; x < 4; x++)
                        if (x != i && x != j && x != k)
                            remain = vals[x];
                    return 70 + remain; // 70~79
                }
            }
        }
    }

    // 2. 花牌数量（J/Q/K = 10点，容易凑10的倍数）
    int faceCount = 0;
    for (int i = 0; i < 4; i++)
        if (vals[i] == 10)
            faceCount++;
    score += faceCount * 13;

    // 3. 最佳2张组合的取模结果（越接近0/10越好）
    int best2Mod = 10; // 最优是0（即两数和正好10的倍数）
    for (int i = 0; i < 4; i++)
    {
        for (int j = i + 1; j < 4; j++)
        {
            int mod = (vals[i] + vals[j]) % 10;
            if (mod == 0)
            {
                best2Mod = 0;
                goto done2;
            }
            if (mod < best2Mod)
                best2Mod = mod;
            if (10 - mod < best2Mod)
                best2Mod = 10 - mod;
        }
    }
done2:
    score += (10 - best2Mod) * 3; // 0~30

    // 4. 对子奖励
    for (int i = 0; i < 4; i++)
        for (int j = i + 1; j < 4; j++)
            if (ranks[i] == ranks[j])
                score += 7;

    // 5. A奖励（灵活性高）
    int aceCount = 0;
    for (int i = 0; i < 4; i++)
        if (ranks[i] == ACE)
            aceCount++;
    score += aceCount * 3;

    return score;
}

// ==================== 斗牛牌型判断 ====================
int GameLogic::evaluateHandQuality(const Player &player) const
{
    const auto cards = player.getAllCards();
    if (cards.size() != 5)
        return BULL_NONE;

    // 收集每张牌的斗牛值（J/Q/K=10, A=1, 其他=面值）
    int vals[5];
    int ranks[5];
    int total = 0;
    for (int i = 0; i < 5; i++)
    {
        vals[i] = cards[i].getValue(); // J/Q/K→10, A→1
        ranks[i] = static_cast<int>(cards[i].getRank());
        total += vals[i];
    }

    // --- 五小牛：5张都 < 5 且总和 ≤ 10 ---
    {
        bool allSmall = true;
        for (int i = 0; i < 5; i++)
        {
            if (vals[i] >= 5)
            {
                allSmall = false;
                break;
            }
        }
        if (allSmall && total <= 10)
            return FIVE_SMALL;
    }

    // --- 炸弹：4张相同rank ---
    for (int i = 0; i < 5; i++)
    {
        int cnt = 0;
        for (int j = 0; j < 5; j++)
        {
            if (ranks[j] == ranks[i])
                cnt++;
        }
        if (cnt >= 4)
            return BOMB;
    }

    // --- 五花牛：5张全是J/Q/K ---
    {
        bool allFace = true;
        for (int i = 0; i < 5; i++)
        {
            int r = ranks[i];
            if (r != JACK && r != QUEEN && r != KING)
            {
                allFace = false;
                break;
            }
        }
        if (allFace)
            return FIVE_FLOWER;
    }

    // --- 普通有牛/没牛：找任意3张之和为10的倍数 ---
    for (int i = 0; i < 5; i++)
    {
        for (int j = i + 1; j < 5; j++)
        {
            for (int k = j + 1; k < 5; k++)
            {
                int sum3 = vals[i] + vals[j] + vals[k];
                if (sum3 % 10 == 0)
                {
                    int remain = (total - sum3) % 10;
                    return (remain == 0) ? BULL_BULL : remain; // remain=1~9 即牛1~牛9
                }
            }
        }
    }

    return BULL_NONE; // 没牛
}

void GameLogic::shufflePlayerOrder()
{
    playerOrder.clear();
    for (int i = 0; i < static_cast<int>(players.size()); ++i)
    {
        playerOrder.push_back(i);
    }
    std::shuffle(playerOrder.begin(), playerOrder.end(), std::default_random_engine(static_cast<unsigned>(std::rand())));
}

int GameLogic::getOrderPosition(int playerIndex) const
{
    for (int i = 0; i < static_cast<int>(playerOrder.size()); ++i)
    {
        if (playerOrder[i] == playerIndex)
        {
            return i;
        }
    }
    return -1;
}

void GameLogic::advanceBidder()
{
    currentBidPosition++;

    // 反抢回合中：原叫庄者做完决定后立即结束，不让已过牌的人再轮一遍
    if (robBackPhase)
    {
        finishBidding();
        return;
    }

    if (currentBidPosition >= static_cast<int>(playerOrder.size()))
    {
        // ── 反抢机制：若原叫庄者被抢，给予一次反抢回合 ──
        if (originalCallerIndex != -1 &&
            currentBankerCandidate != originalCallerIndex &&
            !robBackPhase)
        {
            robBackPhase = true;
            // 找到原叫庄者在 playerOrder 中的位置
            for (int i = 0; i < static_cast<int>(playerOrder.size()); i++)
            {
                if (playerOrder[i] == originalCallerIndex)
                {
                    currentBidPosition = i;
                    break;
                }
            }
            std::cout << "[Counter-Rob] " << players[originalCallerIndex].getName()
                      << " gets a chance to rob back! (odds x" << currentOddsMultiplier << ")"
                      << std::endl;
        }
        else
        {
            finishBidding();
        }
    }
}

void GameLogic::finishBidding()
{
    biddingActive = false;
    if (playerOrder.empty())
    {
        std::cerr << "ERROR: playerOrder is empty in finishBidding!" << std::endl;
        bankerIndex = 0;
        currentOddsMultiplier = (aiDifficulty == AI_HARD) ? 16 : 1;
    }
    else if (currentBankerCandidate == -1)
    {
        int randomIndex = rand() % static_cast<int>(playerOrder.size());
        bankerIndex = playerOrder[randomIndex];
        currentOddsMultiplier = (aiDifficulty == AI_HARD) ? 16 : 1;
        std::cout << "No one called banker. " << players[bankerIndex].getName() << " becomes banker randomly." << std::endl;
    }
    else
    {
        bankerIndex = currentBankerCandidate;
        std::cout << players[bankerIndex].getName() << " becomes banker with odds x" << currentOddsMultiplier << "." << std::endl;
    }
    for (auto &player : players)
    {
        player.setBanker(false);
        player.setOddsMultiplier(1);
    }
    if (bankerIndex >= 0 && bankerIndex < static_cast<int>(players.size()))
    {
        players[bankerIndex].setBanker(true);
        players[bankerIndex].setOddsMultiplier(currentOddsMultiplier);
    }
}

bool GameLogic::isBiddingActive() const
{
    return biddingActive;
}

int GameLogic::getCurrentBidderIndex() const
{
    if (!biddingActive || currentBidPosition < 0 || currentBidPosition >= static_cast<int>(playerOrder.size()))
    {
        return -1;
    }
    return playerOrder[currentBidPosition];
}

bool GameLogic::canPlayerCallBanker(int playerIndex) const
{
    return biddingActive && !hasSomeoneCalledBanker() && getCurrentBidderIndex() == playerIndex;
}

bool GameLogic::canPlayerRobBanker(int playerIndex) const
{
    return biddingActive && hasSomeoneCalledBanker() && getCurrentBidderIndex() == playerIndex;
}

void GameLogic::playerCallBanker(int playerIndex)
{
    if (!canPlayerCallBanker(playerIndex))
    {
        return;
    }
    currentBankerCandidate = playerIndex;
    currentOddsMultiplier = (aiDifficulty == AI_HARD) ? 16 : 2;
    lastCallerIndex = playerIndex;
    originalCallerIndex = playerIndex; // 记录首个叫庄者
    advanceBidder();
}

void GameLogic::playerRobBanker(int playerIndex)
{
    if (!canPlayerRobBanker(playerIndex))
    {
        return;
    }
    currentBankerCandidate = playerIndex;
    currentOddsMultiplier *= 2;
    lastCallerIndex = playerIndex;
    advanceBidder();
}

void GameLogic::playerPassBanker(int playerIndex)
{
    if (!biddingActive || getCurrentBidderIndex() != playerIndex)
    {
        return;
    }
    std::cout << players[playerIndex].getName() << " passes." << std::endl;
    advanceBidder();
}

bool GameLogic::hasSomeoneCalledBanker() const
{
    return currentBankerCandidate != -1;
}

int GameLogic::getCurrentOddsMultiplier() const
{
    return currentOddsMultiplier;
}

int GameLogic::getEffectiveOdds() const
{
    // 负资产时 odd 翻倍（结算规则：输家资产为负则 odd×2）
    int base = currentOddsMultiplier;
    if (!players.empty() && players[0].getScore() < 0)
        base *= 2;
    return base;
}

int GameLogic::getAnte() const
{
    return anteAmount;
}

int GameLogic::getBankerIndex() const
{
    return bankerIndex;
}

const std::vector<int> &GameLogic::getPlayerOrder() const
{
    return playerOrder;
}

int GameLogic::getLastCallerIndex() const
{
    return lastCallerIndex;
}

bool GameLogic::isRobBackPhase() const
{
    return robBackPhase;
}

std::string GameLogic::getBullTypeName(int quality)
{
    switch (quality)
    {
    case FIVE_SMALL:
        return "Five Small";
    case BOMB:
        return "Bomb";
    case FIVE_FLOWER:
        return "Five Flower";
    case BULL_BULL:
        return "Bull Bull";
    case BULL_NONE:
        return "No Bull";
    default:
        if (quality >= BULL_1 && quality <= BULL_9)
            return "Bull " + std::to_string(quality);
        return "Unknown";
    }
}

int GameLogic::getPayMultiplier(int quality)
{
    if (quality >= FIVE_SMALL)
        return 5;
    if (quality >= BOMB)
        return 4;
    if (quality >= FIVE_FLOWER)
        return 4;
    if (quality >= BULL_BULL)
        return 3;
    if (quality >= BULL_7)
        return 2;
    return 1; // 没牛~牛6 都是1倍
}

int GameLogic::getPot() const
{
    return pot;
}

int GameLogic::getRoundNumber() const
{
    return roundNumber;
}

const std::string &GameLogic::getLastSettleResult() const
{
    return lastSettleResult;
}

void GameLogic::deductAnte()
{
    for (auto &player : players)
    {
        player.deductChips(anteAmount);
        pot += anteAmount;
    }
    std::cout << "Ante deducted: $" << (anteAmount * static_cast<int>(players.size()))
              << " chips in pot." << std::endl;
}

void GameLogic::startNewRound()
{
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
    cardArrangementActive = false;
    aiSelectedIndices.clear();
    roundNumber++;

    for (auto &player : players)
    {
        player.resetHand();
        player.resetForNewRound();
    }

    deductAnte();        // 扣底注
    dealCardsPhase1();   // 发4张
    startBiddingPhase(); // 开始叫庄

    std::cout << "\n========== ROUND " << roundNumber << " START ==========" << std::endl;
    std::cout << "Pot: " << pot << " chips" << std::endl;
}

void GameLogic::settleRound()
{
    if (bankerIndex < 0 || bankerIndex >= static_cast<int>(players.size()))
    {
        std::cerr << "ERROR: Invalid banker in settleRound!" << std::endl;
        return;
    }

    std::ostringstream result;
    result << "Round " << roundNumber << " | ";

    const auto &banker = players[bankerIndex];
    int bankerQuality = banker.getHandQuality();
    int odds = currentOddsMultiplier;

    result << "Banker: " << banker.getName()
           << " [" << getBullTypeName(bankerQuality) << "] Odds x" << odds << " | ";

    // 先累计每人本局净输赢，最后统一结算（简单难度需要整局上限）
    int netChange[4] = {0, 0, 0, 0};

    for (int i = 0; i < static_cast<int>(players.size()); i++)
    {
        if (i == bankerIndex)
            continue; // 庄家不和自己比

        const auto &player = players[i];
        int playerQuality = player.getHandQuality();

        if (bankerQuality > playerQuality)
        {
            // 庄家赢
            int payMult = getPayMultiplier(bankerQuality);
            // 人类玩家(0)资产为负 → 涉及ta的对比 odd×2
            int effectiveOdds = (players[0].getScore() < 0 && (bankerIndex == 0 || i == 0)) ? odds * 2 : odds;
            int change = 100 * effectiveOdds * payMult;
            if (aiDifficulty == AI_EASY)
                change *= 4;
            netChange[bankerIndex] += change;
            netChange[i] -= change;
            std::cout << banker.getName() << "(" << getBullTypeName(bankerQuality)
                      << ") beats " << player.getName() << "(" << getBullTypeName(playerQuality)
                      << ") $" << change << std::endl;
        }
        else if (playerQuality > bankerQuality)
        {
            // 闲家赢
            int payMult = getPayMultiplier(playerQuality);
            // 人类玩家(0)资产为负 → 涉及ta的对比 odd×2
            int effectiveOdds = (players[0].getScore() < 0 && (bankerIndex == 0 || i == 0)) ? odds * 2 : odds;
            int change = 100 * effectiveOdds * payMult;
            if (aiDifficulty == AI_EASY)
                change *= 4;
            netChange[i] += change;
            netChange[bankerIndex] -= change;
            std::cout << player.getName() << "(" << getBullTypeName(playerQuality)
                      << ") beats " << banker.getName() << "(" << getBullTypeName(bankerQuality)
                      << ") $" << change << std::endl;
        }
        else
        {
            std::cout << banker.getName() << " and " << player.getName()
                      << " tie (both " << getBullTypeName(bankerQuality) << ")" << std::endl;
        }
    }

    // 简单难度：每局每人累计最多输 1000
    if (aiDifficulty == AI_EASY)
    {
        for (int i = 0; i < static_cast<int>(players.size()); i++)
        {
            if (netChange[i] < -1000)
                netChange[i] = -1000;
        }
    }

    // 统一结算
    for (int i = 0; i < static_cast<int>(players.size()); i++)
    {
        if (netChange[i] > 0)
            players[i].addScore(netChange[i]);
        else if (netChange[i] < 0)
            players[i].deductChips(-netChange[i]);
    }

    // 保存人类玩家金币到存档
    if (!players.empty())
        MoneySave::saveMoney(players[0].getScore());

    result << "You ";
    if (netChange[0] >= 0)
        result << "+" << netChange[0];
    else
        result << netChange[0];
    lastSettleResult = result.str();
    std::cout << lastSettleResult << std::endl;
    pot = 0; // 清空底池
}

void GameLogic::setPlayerScore(int index, int score)
{
    if (index >= 0 && index < static_cast<int>(players.size()))
        players[index].setScore(score);
}

void GameLogic::playRound()
{
    for (size_t i = 0; i < players.size(); i++)
    {
        currentPlayerIndex = i;
        if (players[i].isAIPlayer())
        {
            // AI逻辑：随机选择一张卡牌
            if (players[i].getCardCount() > 0)
            {
                int randomIndex = rand() % players[i].getCardCount();
                tableCards.push_back(players[i].getCard(randomIndex));
                players[i].removeCard(randomIndex);
            }
        }
    }
    determineWinner();
}

void GameLogic::displayGameState() const
{
    std::cout << "\n========== GAME STATE ==========" << std::endl;
    for (const auto &player : players)
    {
        player.displayCards();
    }
    std::cout << "Table cards: ";
    for (const auto &card : tableCards)
    {
        std::cout << card.toString() << " ";
    }
    std::cout << std::endl;
    std::cout << "============================\n"
              << std::endl;
}

void GameLogic::determineWinner()
{
    if (tableCards.empty())
        return;

    std::cout << "\nRound cards: ";
    for (const auto &card : tableCards)
    {
        std::cout << card.toString() << " ";
    }
    std::cout << std::endl;

    // Scoring rule:
    // - Check each player whether any 3-card combination sums to 10.
    // - If exactly one player has such a combination, that player wins.
    // - If multiple players have combinations, the player with the largest remaining sum (total - 10) wins.
    std::vector<int> candidates;
    std::vector<int> remainingSums(players.size(), 0);
    for (size_t i = 0; i < players.size(); ++i)
    {
        if (hasThreeSumToTen(players[i]))
        {
            candidates.push_back(static_cast<int>(i));
            remainingSums[i] = remainingAfterThreeToTen(players[i]);
        }
    }

    if (candidates.empty())
    {
        std::cout << "No player can make 10 with three cards." << std::endl;
        lastRoundWinner = -1;
    }
    else if (candidates.size() == 1)
    {
        int winnerIdx = candidates[0];
        std::cout << players[winnerIdx].getName() << " wins (only one made 10)." << std::endl;
        players[winnerIdx].addScore(1);
        lastRoundWinner = winnerIdx;
    }
    else
    {
        // multiple candidates -> compare remaining sums
        int bestIdx = candidates[0];
        int bestSum = remainingSums[bestIdx];
        for (size_t k = 1; k < candidates.size(); ++k)
        {
            int idx = candidates[k];
            if (remainingSums[idx] > bestSum)
            {
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

bool GameLogic::hasThreeSumToTen(const Player &player) const
{
    const auto hand = player.getAllCards();
    int n = static_cast<int>(hand.size());
    for (int i = 0; i < n; ++i)
    {
        for (int j = i + 1; j < n; ++j)
        {
            for (int k = j + 1; k < n; ++k)
            {
                int sum = hand[i].getValue() + hand[j].getValue() + hand[k].getValue();
                if (sum == 10)
                    return true;
            }
        }
    }
    return false;
}

int GameLogic::remainingAfterThreeToTen(const Player &player) const
{
    const auto hand = player.getAllCards();
    int total = 0;
    for (const auto &c : hand)
        total += c.getValue();
    // If player can make 10, remaining is total - 10.
    if (hasThreeSumToTen(player))
        return total - 10;
    return total;
}

int GameLogic::getLastRoundWinner() const
{
    return lastRoundWinner;
}

void GameLogic::clearLastRoundWinner()
{
    lastRoundWinner = -1;
}

const std::vector<Player> &GameLogic::getPlayers() const
{
    return players;
}

int GameLogic::getCurrentPlayerIndex() const
{
    return currentPlayerIndex;
}

void GameLogic::setCurrentPlayerIndex(int index)
{
    currentPlayerIndex = index;
}

bool GameLogic::isGameOver() const
{
    for (const auto &player : players)
    {
        if (player.getCardCount() > 0)
        {
            return false;
        }
    }
    return true;
}

std::vector<Card> GameLogic::getTableCards() const
{
    return tableCards;
}

void GameLogic::addTableCard(const Card &card)
{
    tableCards.push_back(card);
}

void GameLogic::clearTableCards()
{
    tableCards.clear();
}

int GameLogic::calculateHandValue(const std::vector<Card> &cards) const
{
    int value = 0;
    for (const auto &card : cards)
    {
        value += card.getValue();
    }
    return value;
}

// ==================== 组牌阶段 ====================

void GameLogic::startCardArrangementPhase()
{
    cardArrangementActive = true;
    aiSelectedIndices.clear();
    std::cout << "Card arrangement phase started." << std::endl;

    // AI 玩家自动选最优3张
    for (int i = 1; i < static_cast<int>(players.size()); i++)
    {
        aiAutoArrange(i);
    }
}

bool GameLogic::isCardArrangementActive() const
{
    return cardArrangementActive;
}

void GameLogic::finishCardArrangement()
{
    cardArrangementActive = false;
    std::cout << "Card arrangement phase finished." << std::endl;
}

// 根据玩家手动选择的3张牌索引计算牌型
int GameLogic::evaluatePlayerArrangement(int playerIndex, const std::vector<int> &selectedIndices) const
{
    if (selectedIndices.size() != 3)
        return BULL_NONE;
    if (playerIndex < 0 || playerIndex >= static_cast<int>(players.size()))
        return BULL_NONE;

    const auto &cards = players[playerIndex].getAllCards();
    if (cards.size() != 5)
        return BULL_NONE;

    // 验证索引合法性
    for (int idx : selectedIndices)
    {
        if (idx < 0 || idx >= 5)
            return BULL_NONE;
    }
    // 检查是否有重复
    if (selectedIndices[0] == selectedIndices[1] ||
        selectedIndices[0] == selectedIndices[2] ||
        selectedIndices[1] == selectedIndices[2])
        return BULL_NONE;

    int vals[5];
    int ranks[5];
    int total = 0;
    for (int i = 0; i < 5; i++)
    {
        vals[i] = cards[i].getValue();
        ranks[i] = static_cast<int>(cards[i].getRank());
        total += vals[i];
    }

    // --- 先检查特殊牌型（无论怎么选）---
    // 五小牛：5张都 < 5 且总和 ≤ 10
    {
        bool allSmall = true;
        for (int i = 0; i < 5; i++)
        {
            if (vals[i] >= 5)
            {
                allSmall = false;
                break;
            }
        }
        if (allSmall && total <= 10)
            return FIVE_SMALL;
    }

    // 炸弹：4张相同rank
    for (int i = 0; i < 5; i++)
    {
        int cnt = 0;
        for (int j = 0; j < 5; j++)
        {
            if (ranks[j] == ranks[i])
                cnt++;
        }
        if (cnt >= 4)
            return BOMB;
    }

    // 五花牛：5张全是J/Q/K
    {
        bool allFace = true;
        for (int i = 0; i < 5; i++)
        {
            int r = ranks[i];
            if (r != JACK && r != QUEEN && r != KING)
            {
                allFace = false;
                break;
            }
        }
        if (allFace)
            return FIVE_FLOWER;
    }

    // --- 普通斗牛：检查选中的3张是否和为10的倍数 ---
    int sum3 = vals[selectedIndices[0]] + vals[selectedIndices[1]] + vals[selectedIndices[2]];
    if (sum3 % 10 != 0)
        return BULL_NONE; // 选错了，没牛

    int remain = (total - sum3) % 10;
    return (remain == 0) ? BULL_BULL : remain;
}

// AI 自动选最优3张组合（根据难度决定智能程度）
void GameLogic::aiAutoArrange(int playerIndex)
{
    if (playerIndex < 0 || playerIndex >= static_cast<int>(players.size()))
        return;

    const auto &cards = players[playerIndex].getAllCards();
    if (cards.size() != 5)
        return;

    // 先用完整算法得出最优牌质
    int fullQuality = evaluateHandQuality(players[playerIndex]);

    // 如果是特殊牌型（五花牛/炸弹/五小牛），任意选3张都行
    if (fullQuality >= FIVE_FLOWER)
    {
        aiSelectedIndices = {0, 1, 2};
        players[playerIndex].setHandQuality(fullQuality);
        std::cout << players[playerIndex].getName() << " arranges: "
                  << GameLogic::getBullTypeName(fullQuality) << " (special)" << std::endl;
        return;
    }

    // ── 收集所有有效3张组合 ──
    int vals[5];
    int total = 0;
    for (int i = 0; i < 5; i++)
    {
        vals[i] = cards[i].getValue();
        total += vals[i];
    }

    struct Combo
    {
        int i, j, k, quality;
    };
    std::vector<Combo> validCombos;
    int bestQuality = BULL_NONE;
    int bestI = 0, bestJ = 1, bestK = 2;

    for (int i = 0; i < 5; i++)
    {
        for (int j = i + 1; j < 5; j++)
        {
            for (int k = j + 1; k < 5; k++)
            {
                int sum3 = vals[i] + vals[j] + vals[k];
                if (sum3 % 10 == 0)
                {
                    int remain = (total - sum3) % 10;
                    int q = (remain == 0) ? BULL_BULL : remain;
                    validCombos.push_back({i, j, k, q});
                    if (q > bestQuality)
                    {
                        bestQuality = q;
                        bestI = i;
                        bestJ = j;
                        bestK = k;
                    }
                }
            }
        }
    }

    // ── 根据难度选择组合 ──
    int finalI = bestI, finalJ = bestJ, finalK = bestK;
    int finalQuality = bestQuality;

    if (aiDifficulty == AI_EASY && !validCombos.empty())
    {
        // 简单AI：有30%概率不选最优，随机选一个有效组合
        if ((rand() % 100) < 30 && validCombos.size() > 1)
        {
            int idx = rand() % static_cast<int>(validCombos.size());
            finalI = validCombos[idx].i;
            finalJ = validCombos[idx].j;
            finalK = validCombos[idx].k;
            finalQuality = validCombos[idx].quality;
            std::cout << players[playerIndex].getName()
                      << " (Easy) picks suboptimal: " << GameLogic::getBullTypeName(finalQuality)
                      << " (best was " << GameLogic::getBullTypeName(bestQuality) << ")" << std::endl;
        }
        // 如果没有任何有效组合（没牛），Easy AI 随机选3张
        if (validCombos.empty())
        {
            finalI = rand() % 5;
            do
            {
                finalJ = rand() % 5;
            } while (finalJ == finalI);
            do
            {
                finalK = rand() % 5;
            } while (finalK == finalI || finalK == finalJ);
            finalQuality = BULL_NONE;
            std::cout << players[playerIndex].getName() << " (Easy) random picks: No Bull" << std::endl;
        }
    }
    else if (aiDifficulty == AI_HARD)
    {
        // 困难AI：总是最优（与普通一致，但日志标记为Hard）
        std::cout << players[playerIndex].getName()
                  << " (Hard) arranges: " << GameLogic::getBullTypeName(bestQuality) << std::endl;
    }
    else // AI_NORMAL
    {
        std::cout << players[playerIndex].getName() << " arranges: "
                  << GameLogic::getBullTypeName(bestQuality) << std::endl;
    }

    aiSelectedIndices = {finalI, finalJ, finalK};
    players[playerIndex].setHandQuality(finalQuality);
}

void GameLogic::setAIDifficulty(AIDifficulty diff)
{
    aiDifficulty = diff;
    // 根据难度设置底注
    switch (diff)
    {
    case AI_EASY:
        anteAmount = 0;
        break;
    case AI_NORMAL:
        anteAmount = 100;
        break;
    case AI_HARD:
        anteAmount = 500;
        break;
    }
    const char *names[] = {"Easy", "Normal", "Hard"};
    std::cout << "AI Difficulty set to: " << names[diff] << " (ante=" << anteAmount << ")" << std::endl;
}

AIDifficulty GameLogic::getAIDifficulty() const
{
    return aiDifficulty;
}

const std::vector<int> &GameLogic::getAiSelectedIndices() const
{
    return aiSelectedIndices;
}
void GameLogic::confirmPlayerArrangement(int playerIndex, const std::vector<int> &selectedIndices)
{
    int quality = evaluatePlayerArrangement(playerIndex, selectedIndices);
    if (playerIndex >= 0 && playerIndex < static_cast<int>(players.size()))
    {
        players[playerIndex].setHandQuality(quality);
        std::cout << players[playerIndex].getName() << " arranged: "
                  << GameLogic::getBullTypeName(quality) << std::endl;
    }
    finishCardArrangement();
    settleRound();
}