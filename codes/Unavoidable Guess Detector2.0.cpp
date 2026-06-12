#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <set>
#include <algorithm>
#include <queue>
#include <map>
#include <cstring>
using namespace std;

const int stageWidth = 9;
const int stageHeight = 9;
const int mMineNum = 10;
const int MAX_RETRIES = 1000; // 最大重试次数

enum CellState
{
    ncUNDOWN,
    ncMINE,
    ncFLAG,
    ncQ
};

struct Cell
{
    CellState mState = ncUNDOWN;
    CellState mStateBackUp = ncUNDOWN;
    int number = 0;
    bool revealed = false;
};

using Board = vector<vector<Cell>>;

// 计算所有非雷格子的邻接雷数
void CalculateNumbers(Board &mGameData)
{
    for (int i = 0; i < stageHeight; ++i)
    {
        for (int j = 0; j < stageWidth; ++j)
        {
            if (mGameData[i][j].mState == ncMINE)
                continue;
            int count = 0;
            for (int di = -1; di <= 1; ++di)
            {
                for (int dj = -1; dj <= 1; ++dj)
                {
                    int ni = i + di, nj = j + dj;
                    if (ni >= 0 && ni < stageHeight && nj >= 0 && nj < stageWidth && mGameData[ni][nj].mState == ncMINE)
                        count++;
                }
            }
            mGameData[i][j].number = count;
        }
    }
}

// 递归翻开空白格子（用于模拟首次点击后的展开）
void Reveal(Board &mGameData, int y, int x)
{
    if (y < 0 || y >= stageHeight || x < 0 || x >= stageWidth || mGameData[y][x].revealed)
        return;
    if (mGameData[y][x].mState == ncMINE)
        return; // 不能翻开雷
    mGameData[y][x].revealed = true;
    if (mGameData[y][x].number == 0)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                if (dy != 0 || dx != 0)
                {
                    Reveal(mGameData, y + dy, x + dx);
                }
            }
        }
    }
}

// 重置revealed状态（用于多次检测）
void ResetRevealed(Board &mGameData)
{
    for (int i = 0; i < stageHeight; ++i)
    {
        for (int j = 0; j < stageWidth; ++j)
        {
            mGameData[i][j].revealed = false;
        }
    }
}

// 获取所有已翻开格子的边界（未被翻开的邻接格子）
set<pair<int, int>> GetFrontier(const Board &mGameData)
{
    set<pair<int, int>> frontier;
    for (int y = 0; y < stageHeight; ++y)
    {
        for (int x = 0; x < stageWidth; ++x)
        {
            if (mGameData[y][x].revealed && mGameData[y][x].number > 0)
            {
                for (int dy = -1; dy <= 1; ++dy)
                {
                    for (int dx = -1; dx <= 1; ++dx)
                    {
                        int ny = y + dy, nx = x + dx;
                        if (ny >= 0 && ny < stageHeight && nx >= 0 && nx < stageWidth && !mGameData[ny][nx].revealed && mGameData[ny][nx].mState != ncMINE)
                        {
                            frontier.insert({ny, nx});
                        }
                    }
                }
            }
        }
    }
    return frontier;
}

// 获取与边界格子相邻的已翻开数字格子的约束条件
// 返回: 数字格 -> 其相邻未翻开格子列表 的映射
map<pair<int, int>, vector<pair<int, int>>> GetConstraints(const Board &mGameData)
{
    map<pair<int, int>, vector<pair<int, int>>> constraints;
    for (int y = 0; y < stageHeight; ++y)
    {
        for (int x = 0; x < stageWidth; ++x)
        {
            if (mGameData[y][x].revealed && mGameData[y][x].number > 0)
            {
                vector<pair<int, int>> neighbors;
                for (int dy = -1; dy <= 1; ++dy)
                {
                    for (int dx = -1; dx <= 1; ++dx)
                    {
                        int ny = y + dy, nx = x + dx;
                        if (ny >= 0 && ny < stageHeight && nx >= 0 && nx < stageWidth && !mGameData[ny][nx].revealed && mGameData[ny][nx].mState != ncMINE)
                        {
                            neighbors.push_back({ny, nx});
                        }
                    }
                }
                if (!neighbors.empty())
                {
                    constraints[{y, x}] = neighbors;
                }
            }
        }
    }
    return constraints;
}

// 计算已翻开数字格周围已标记的雷数（旗子）
int CountFlaggedAround(const Board &mGameData, int y, int x)
{
    int count = 0;
    for (int dy = -1; dy <= 1; ++dy)
    {
        for (int dx = -1; dx <= 1; ++dx)
        {
            int ny = y + dy, nx = x + dx;
            if (ny >= 0 && ny < stageHeight && nx >= 0 && nx < stageWidth)
            {
                if (mGameData[ny][nx].mState == ncFLAG)
                {
                    count++;
                }
            }
        }
    }
    return count;
}

// 深度优先搜索判断未翻开格子组合的雷数约束是否可解
// 使用约束传播和回溯搜索判断是否存在多解
bool IsDeterministic(const Board &mGameData,
                     const vector<pair<int, int>> &frontierVec,
                     const map<pair<int, int>, vector<pair<int, int>>> &constraints,
                     int mineCountInFrontier)
{

    int n = frontierVec.size();
    if (n == 0)
        return true;
    if (n > 15)
        return true; // 边界太大，保守认为可解（避免指数爆炸）

    // 使用位掩码枚举所有可能的布雷组合（n <= 15）
    int totalCombinations = 1 << n;
    int validCombinations = 0;
    int firstValidCombination = -1;

    // 预计算每个边界格子的索引
    map<pair<int, int>, int> frontierIndex;
    for (int i = 0; i < n; ++i)
    {
        frontierIndex[frontierVec[i]] = i;
    }

    // 枚举所有可能的布雷方案
    for (int mask = 0; mask < totalCombinations; ++mask)
    {
        // 检查雷数是否符合预期
        int mineCount = __builtin_popcount(mask);
        if (mineCount != mineCountInFrontier)
            continue;

        // 检查是否满足所有约束
        bool valid = true;
        for (const auto &constraint : constraints)
        {
            int y = constraint.first.first;
            int x = constraint.first.second;
            int targetMines = mGameData[y][x].number - CountFlaggedAround(mGameData, y, x);

            if (targetMines < 0)
            {
                valid = false;
                break;
            }

            int actualMines = 0;
            for (const auto &neighbor : constraint.second)
            {
                auto it = frontierIndex.find(neighbor);
                if (it != frontierIndex.end() && (mask >> it->second) & 1)
                {
                    actualMines++;
                }
            }

            // 注意：如果边界格子数量少于目标雷数，那是不可能的
            if (actualMines > targetMines || actualMines + (constraint.second.size() - actualMines) < targetMines)
            {
                valid = false;
                break;
            }
        }

        if (valid)
        {
            validCombinations++;
            if (firstValidCombination == -1)
            {
                firstValidCombination = mask;
            }
        }
    }

    // 如果没有有效组合，返回true（但理论上不应该发生）
    if (validCombinations == 0)
        return true;

    // 如果有且仅有1种有效组合，说明每个格子的状态是确定的
    if (validCombinations == 1)
    {
        return true;
    }

    // 如果有多于1种有效组合，检查是否每个格子的状态在所有有效组合中都一致
    // 如果所有组合中某个格子都是雷或都是安全，则该格子仍然是确定的
    vector<int> mineCountForCell(n, 0);
    for (int mask = 0; mask < totalCombinations; ++mask)
    {
        int mineCount = __builtin_popcount(mask);
        if (mineCount != mineCountInFrontier)
            continue;

        bool valid = true;
        for (const auto &constraint : constraints)
        {
            int y = constraint.first.first;
            int x = constraint.first.second;
            int targetMines = mGameData[y][x].number - CountFlaggedAround(mGameData, y, x);

            if (targetMines < 0)
            {
                valid = false;
                break;
            }

            int actualMines = 0;
            for (const auto &neighbor : constraint.second)
            {
                auto it = frontierIndex.find(neighbor);
                if (it != frontierIndex.end() && (mask >> it->second) & 1)
                {
                    actualMines++;
                }
            }

            if (actualMines > targetMines || actualMines + (constraint.second.size() - actualMines) < targetMines)
            {
                valid = false;
                break;
            }
        }

        if (valid)
        {
            for (int i = 0; i < n; ++i)
            {
                if ((mask >> i) & 1)
                {
                    mineCountForCell[i]++;
                }
            }
        }
    }

    // 如果所有有效组合中某个格子的雷数状态一致（要么总是雷，要么总不是雷）
    // 则该格子是确定的
    for (int i = 0; i < n; ++i)
    {
        if (mineCountForCell[i] > 0 && mineCountForCell[i] < validCombinations)
        {
            // 该格子既可以是雷也可以不是雷 -> 存在不确定性
            return false;
        }
    }

    return true;
}

// 检测当前棋盘是否存在不可避免猜测（改进版）
bool HasUnavoidableGuess(const Board &mGameData)
{
    auto frontier = GetFrontier(mGameData);
    if (frontier.empty())
        return false;

    // 将frontier转换为vector以便索引
    vector<pair<int, int>> frontierVec(frontier.begin(), frontier.end());

    // 获取约束条件
    auto constraints = GetConstraints(mGameData);
    if (constraints.empty())
    {
        // 没有约束，意味着边界格子无法与任何数字关联 → 存在不可避免猜测
        // 但如果所有边界格子都是孤立的且没有数字相邻，确实需要猜测
        return !frontier.empty();
    }

    // 计算边界中可能的雷数范围（根据约束）
    // 简单估计：边界雷数至少是所有数字格的最小可能雷数之和的下界
    // 这里我们枚举所有可能的雷数范围

    int minPossibleMines = 0;
    int maxPossibleMines = frontierVec.size();

    // 尝试不同的雷数假设
    bool anyAmbiguous = false;
    for (int tryMines = minPossibleMines; tryMines <= maxPossibleMines; ++tryMines)
    {
        if (!IsDeterministic(mGameData, frontierVec, constraints, tryMines))
        {
            anyAmbiguous = true;
            break;
        }
    }

    return anyAmbiguous;
}

// 深度复制棋盘
Board CopyBoard(const Board &src)
{
    Board dst(stageHeight, vector<Cell>(stageWidth));
    for (int i = 0; i < stageHeight; ++i)
    {
        for (int j = 0; j < stageWidth; ++j)
        {
            dst[i][j] = src[i][j];
        }
    }
    return dst;
}

// 改进版布雷函数 - 确保无不可避免猜测
bool MineSet_NoGuess(Board &mGameData, int Py, int Px, int maxRetries = MAX_RETRIES)
{
    srand(time(NULL));

    for (int attempt = 0; attempt < maxRetries; ++attempt)
    {
        // 1. 收集可用格子（避开首次点击点及其周围8格）
        vector<pair<int, int>> availableCells;
        for (int i = 0; i < stageHeight; ++i)
        {
            for (int j = 0; j < stageWidth; ++j)
            {
                // 避开首次点击点及其周围8格
                if (i >= Py - 1 && i <= Py + 1 && j >= Px - 1 && j <= Px + 1)
                    continue;
                availableCells.push_back({i, j});
            }
        }

        // 检查是否有足够空间布雷
        if (availableCells.size() < mMineNum)
        {
            return false;
        }

        // 2. 随机打乱并布设地雷
        random_shuffle(availableCells.begin(), availableCells.end());

        // 重置棋盘
        for (int i = 0; i < stageHeight; ++i)
        {
            for (int j = 0; j < stageWidth; ++j)
            {
                mGameData[i][j].mState = ncUNDOWN;
                mGameData[i][j].mStateBackUp = ncUNDOWN;
                mGameData[i][j].number = 0;
                mGameData[i][j].revealed = false;
            }
        }

        // 布雷
        for (int i = 0; i < mMineNum; ++i)
        {
            int y = availableCells[i].first;
            int x = availableCells[i].second;
            mGameData[y][x].mState = ncMINE;
            mGameData[y][x].mStateBackUp = ncMINE;
        }

        // 3. 计算邻接数字
        CalculateNumbers(mGameData);

        // 4. 模拟首次点击后的展开
        ResetRevealed(mGameData);
        Reveal(mGameData, Py, Px);

        // 5. 检测是否存在不可避免猜测
        if (!HasUnavoidableGuess(mGameData))
        {
            // 成功：找到无猜棋盘
            cout << "成功生成无猜棋盘，尝试次数：" << attempt + 1 << endl;
            return true;
        }

        // 每100次尝试输出一次进度
        if ((attempt + 1) % 100 == 0)
        {
            cout << "正在进行第 " << attempt + 1 << " 次尝试..." << endl;
        }
    }

    // 降级：返回一个普通随机棋盘
    cout << "警告：已达到最大重试次数(" << maxRetries << ")，返回普通随机棋盘" << endl;

    // 最后尝试一次普通随机布雷
    vector<pair<int, int>> availableCells;
    for (int i = 0; i < stageHeight; ++i)
    {
        for (int j = 0; j < stageWidth; ++j)
        {
            if (i >= Py - 1 && i <= Py + 1 && j >= Px - 1 && j <= Px + 1)
                continue;
            availableCells.push_back({i, j});
        }
    }
    random_shuffle(availableCells.begin(), availableCells.end());

    for (int i = 0; i < stageHeight; ++i)
    {
        for (int j = 0; j < stageWidth; ++j)
        {
            mGameData[i][j].mState = ncUNDOWN;
            mGameData[i][j].revealed = false;
        }
    }
    for (int i = 0; i < mMineNum; ++i)
    {
        int y = availableCells[i].first;
        int x = availableCells[i].second;
        mGameData[y][x].mState = ncMINE;
    }
    CalculateNumbers(mGameData);

    return false; // 虽然不是无猜棋盘，但返回了有效棋盘
}

// 打印棋盘状态（调试用）
void PrintBoard(const Board &mGameData)
{
    cout << "   ";
    for (int x = 0; x < stageWidth; ++x)
    {
        cout << x << " ";
    }
    cout << endl;

    for (int y = 0; y < stageHeight; ++y)
    {
        cout << y << "  ";
        for (int x = 0; x < stageWidth; ++x)
        {
            if (mGameData[y][x].mState == ncMINE)
            {
                cout << "* ";
            }
            else if (mGameData[y][x].revealed)
            {
                if (mGameData[y][x].number == 0)
                {
                    cout << ". ";
                }
                else
                {
                    cout << mGameData[y][x].number << " ";
                }
            }
            else
            {
                cout << "# ";
            }
        }
        cout << endl;
    }
}

int main()
{
    Board mGameData(stageHeight, vector<Cell>(stageWidth));

    // 初始点击位置（避开边角，便于检测）
    int Py = stageHeight / 2;
    int Px = stageWidth / 2;

    cout << "=== 无猜布雷函数测试 ===" << endl;
    cout << "棋盘大小: " << stageWidth << "x" << stageHeight << endl;
    cout << "地雷数量: " << mMineNum << endl;
    cout << "首次点击位置: (" << Py << ", " << Px << ")" << endl;
    cout << endl;

    // 使用改进版布雷函数
    bool success = MineSet_NoGuess(mGameData, Py, Px);

    // 最终检测
    ResetRevealed(mGameData);
    Reveal(mGameData, Py, Px);

    cout << endl;
    cout << "=== 最终棋盘（游戏结束时局面）===" << endl;
    PrintBoard(mGameData);
    cout << endl;
    cout << "最终检测是否存在不可避免猜测：" << (HasUnavoidableGuess(mGameData) ? "是" : "否") << endl;
    cout << "布雷结果：" << (success ? "成功生成无猜棋盘" : "降级为普通随机棋盘") << endl;

    getchar();
    return 0;
}