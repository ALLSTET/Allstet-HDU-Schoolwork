#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <set>
#include <algorithm>
using namespace std;

const int stageWidth = 9;
const int stageHeight = 9;
const int mMineNum = 10;

enum CellState {
    ncUNDOWN,
    ncMINE,
    ncFLAG,
    ncQ
};

struct Cell {
    CellState mState = ncUNDOWN;
    CellState mStateBackUp = ncUNDOWN;
    int number = 0;
    bool revealed = false;
};

using Board = vector<vector<Cell>>;

void CalculateNumbers(Board &mGameData) {
    for (int i = 0; i < stageHeight; ++i) {
        for (int j = 0; j < stageWidth; ++j) {
            if (mGameData[i][j].mState == ncMINE) continue;
            int count = 0;
            for (int di = -1; di <= 1; ++di) {
                for (int dj = -1; dj <= 1; ++dj) {
                    int ni = i + di, nj = j + dj;
                    if (ni >= 0 && ni < stageHeight && nj >= 0 && nj < stageWidth && mGameData[ni][nj].mState == ncMINE)
                        count++;
                }
            }
            mGameData[i][j].number = count;
        }
    }
}

void Reveal(Board &mGameData, int y, int x) {
    if (y < 0 || y >= stageHeight || x < 0 || x >= stageWidth || mGameData[y][x].revealed)
        return;
    mGameData[y][x].revealed = true;
    if (mGameData[y][x].number == 0 && mGameData[y][x].mState != ncMINE) {
        for (int dy = -1; dy <= 1; ++dy)
            for (int dx = -1; dx <= 1; ++dx)
                if (dy != 0 || dx != 0)
                    Reveal(mGameData, y + dy, x + dx);
    }
}

set<pair<int, int>> GetFrontier(const Board &mGameData) {
    set<pair<int, int>> frontier;
    for (int y = 0; y < stageHeight; ++y) {
        for (int x = 0; x < stageWidth; ++x) {
            if (mGameData[y][x].revealed && mGameData[y][x].number > 0) {
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        int ny = y + dy, nx = x + dx;
                        if (ny >= 0 && ny < stageHeight && nx >= 0 && nx < stageWidth && !mGameData[ny][nx].revealed)
                            frontier.insert({ny, nx});
                    }
                }
            }
        }
    }
    return frontier;
}

bool HasUnavoidableGuess(const Board &mGameData) {
    auto frontier = GetFrontier(mGameData);
    vector<pair<int, int>> vec(frontier.begin(), frontier.end());
    for (size_t i = 0; i < vec.size(); ++i) {
        for (size_t j = i + 1; j < vec.size(); ++j) {
            int dy = abs(vec[i].first - vec[j].first);
            int dx = abs(vec[i].second - vec[j].second);
            if (dy <= 1 && dx <= 1)
                return true;
        }
    }
    return false;
}

void MineSet(Board &mGameData, int Py, int Px) {
    srand(time(NULL));
    vector<pair<int, int>> availableCells;

    for (int i = 0; i < stageHeight; ++i) {
        for (int j = 0; j < stageWidth; ++j) {
            if (i >= Py - 1 && i <= Py + 1 && j >= Px - 1 && j <= Px + 1)
                continue;
            availableCells.push_back({i, j});
        }
    }

    bool validPlacement = false;
    while (!validPlacement) {
        random_shuffle(availableCells.begin(), availableCells.end());
        for (int i = 0; i < stageHeight; ++i)
            for (int j = 0; j < stageWidth; ++j)
                mGameData[i][j].mState = ncUNDOWN;

        for (int i = 0; i < mMineNum && i < availableCells.size(); ++i) {
            int k = availableCells[i].first;
            int l = availableCells[i].second;
            mGameData[k][l].mState = ncMINE;
            mGameData[k][l].mStateBackUp = ncMINE;
        }

        CalculateNumbers(mGameData);
        for (int i = 0; i < stageHeight; ++i)
            for (int j = 0; j < stageWidth; ++j)
                mGameData[i][j].revealed = false;

        Reveal(mGameData, Py, Px);
        validPlacement = !HasUnavoidableGuess(mGameData);
    }
}

int main() {
    Board mGameData(stageHeight, vector<Cell>(stageWidth));
    int Py = 0, Px = 0;
    MineSet(mGameData, Py, Px);
    CalculateNumbers(mGameData);
    Reveal(mGameData, Py, Px);

    cout << "当前是否存在不可避免猜测：" << (HasUnavoidableGuess(mGameData) ? "是" : "否") << endl;
    return 0;
}
