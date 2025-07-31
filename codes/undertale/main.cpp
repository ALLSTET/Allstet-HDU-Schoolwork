#include <iostream>
#include <easyx.h>
#include <windows.h>
#include <conio.h>
#include <mmsystem.h>
#include <time.h>
#include <cmath>
#include <vector>
#include <memory>
#include <algorithm>
#include <fstream>
#include <string>
#define FPS 60
#define POLLEN_POOL_SIZE 20
 
// 全局变量：跟踪Pollen_Green是否被吃掉
bool greenPollenEaten = false;

class Enemy
{
protected:
  int HP;
  int HP_MAX;
  int x;
  int y;

public:
  Enemy(int a, int width, int height) : HP_MAX(a)
  {
    HP = HP_MAX;
    x = 320 - (width / 2);
    y = 220 - height;
  }
  inline void setX(int a)
  {
    x = a;
  }
  inline void setY(int a)
  {
    y = a;
  }
  inline void setHP(int a)
  {
    HP = a;
  }
  inline int getHP()
  {
    return HP;
  }
  inline int getHP_MAX()
  {
    return HP_MAX;
  }
  inline int getX()
  {
    return x;
  }
  inline int getY()
  {
    return y;
  }
};
class Flower : public Enemy
{
private:
  IMAGE flower_A;
  IMAGE flower_B;

public:
  Flower() : Enemy(100, 96, 99)
  {
    loadimage(&flower_A, _T("img\\flower_1.png"), 96, 99);
    loadimage(&flower_B, _T("img\\flower_2.png"), 96, 99);
  }
  inline IMAGE getFlowerA()
  {
    return flower_A;
  }
  inline IMAGE getFlowerB()
  {
    return flower_B;
  }
};
class Pollen;
class Heart
{
private:
  int HP_MAX;
  int HP;
  int LV;
  int x;
  int y;
  IMAGE heart;
  bool invincible = false; // 新增：无敌状态
  int invincibleTimer = 0; // 新增：无敌剩余帧
  inline void setLV(int a)
  {
    LV = a;
  }
  inline void setHP_MAX(int a)
  {
    HP_MAX = a;
  }
  inline void setX(int a)
  {
    x = a;
  }
  inline void setY(int a)
  {
    y = a;
  }
  friend class Pollen;

public:
  Heart()
  {
    LV = 1;
    HP_MAX = 10 + LV * 10;
    HP = HP_MAX;
    x = 320;
    y = 315;
    loadimage(&heart, _T("img\\heart.png"), 18, 17);
  }
  inline IMAGE getHeart()
  {
    if (invincible) {
      static IMAGE heartG;
      static bool loaded = false;
      if (!loaded) {
        loadimage(&heartG, _T("img\\HEART_G.png"), 18, 17);
        loaded = true;
      }
      return heartG;
    }
    return heart;
  }
  inline int getHP()
  {
    return HP;
  }
  inline int getHP_MAX()
  {
    return HP_MAX;
  }
  inline int getLV()
  {
    return LV;
  }
  inline int getX()
  {
    return x;
  }
  inline int getY()
  {
    return y;
  }
  // 新增：设置无敌
  void setInvincible(int duration) { invincible = true; invincibleTimer = duration; }
  // 新增：判断无敌
  bool isInvincible() const { return invincible; }
  // 新增：每帧递减无敌计时
  void tickInvincible() {
    if (invincible) {
      if (invincibleTimer > 0) --invincibleTimer;
      if (invincibleTimer <= 0) invincible = false;
    }
  }
  inline void setHP(int a)
  {
    HP = a;
  }
  void gameControl()
  {
    if (GetAsyncKeyState(VK_UP) & 0x8000)
    {
      if (getY() > 256)
      {
        setY(getY() - 4);
      }
    }
    else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
    {
      if (getY() < 358)
      {
        setY(getY() + 4);
      }
    }
    else if (GetAsyncKeyState(VK_LEFT) & 0x8000)
    {
      if (getX() > 244)
      {
        setX(getX() - 4);
      }
    }
    else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
    {
      if (getX() < 378)
      {
        setX(getX() + 4);
      }
    }
    else if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
    {
      setHP(0);
    }
    else if (GetAsyncKeyState('0') & 0x8000)
    {
      setLV(9);
    }
    else if (GetAsyncKeyState('1') & 0x8000)
    {
      mciSendString(_TEXT("stop music\\Flowey.mp3"), NULL, 0, NULL);
    }
    else if (GetAsyncKeyState('2') & 0x8000)
    {
      mciSendString(_TEXT("play music\\Flowey.mp3 repeat"), NULL, 0, NULL);
    }
    else
    {
      NULL;
    }
  }
  void heartDataShow()
  {
    const int HP_BAR_WIDTH = 50;  // 固定HP条宽度
    const int HP_BAR_X = 310;
    const int HP_BAR_Y = 405;
    
    // 边框
    setlinestyle(PS_SOLID, 5, NULL, 0);
    setlinecolor(WHITE);
    rectangle(240, 250, 400, 380);
    
    // 当前HP（黄色）
    int currentWidth = (getHP() * HP_BAR_WIDTH) / getHP_MAX();
    setfillcolor(YELLOW);
    solidrectangle(HP_BAR_X, HP_BAR_Y, HP_BAR_X + currentWidth, HP_BAR_Y + 20);
    
    // 最大HP（红色背景）
    setfillcolor(RED);
    solidrectangle(HP_BAR_X + currentWidth, HP_BAR_Y, HP_BAR_X + HP_BAR_WIDTH, HP_BAR_Y + 20);
    
    // 等级显示
    settextstyle(30, 15, _T("Determination Sans Web"));
    settextcolor(WHITE);
    char data[50] = "";
    sprintf(data, "LV %d", getLV());
    outtextxy(200, 400, data);
    
    // HP标签
    settextstyle(16, 12, _T("Determination Sans Web"));
    outtextxy(273, 406, _T("HP"));
    
    // HP数值显示
    settextstyle(30, 15, _T("Determination Sans Web"));
    sprintf(data, "%02d / %02d", getHP(), getHP_MAX());
    outtextxy(HP_BAR_X + HP_BAR_WIDTH + 10, 400, data);
  }
  void levelUp()
  {
    LV++;
    setLV(LV);
    setHP_MAX(10 + LV * 10);
    setHP(HP + 10);
  }
  int getInvincibleTimer() const { return invincibleTimer; }
};
class Pollen : public Enemy
{
protected:
    int speed;
public:
    int activationDelay = 0;
    int targetOffsetX = 0;
    int targetOffsetY = 0;
    int imageSwitchTimer = 0;  // 每个pollen独立的图像切换计时器
    bool wasHit = false;  // 标记是否击中玩家
    int switchCooldown = 0; // 新增：切换冷却
    bool usePollenAState = true; // 新增：当前使用A图像
    int lastSwitchState = 0; // 新增：上次切换状态
    virtual void setSpeed(int a) { speed = a; }
    bool active = false;
    Pollen() : Enemy(2, 14, 14), speed(1)
    {
        loadimage(&pollenA, _T("img\\pollen_1.png"), 14, 8);
        loadimage(&pollenB, _T("img\\pollen_2.png"), 8, 14);
        generateNewXY();
    }
    inline virtual IMAGE getPollenA()
    {
        return pollenA;
    }
    inline virtual IMAGE getPollenB()
    {
        return pollenB;
    }
    inline int getSpeed()
    {
        return speed;
    }
    inline int getNewX()
    {
        return newX;
    }
    inline int getNewY()
    {
        return newY;
    }
    virtual void draw() {
        IMAGE bulletImage = usePollenAState ? getPollenA() : getPollenB();
        putimage(getX(), getY(), &bulletImage);
    }
    virtual void bulletMove(Heart &name)
    {
        if (activationDelay > 0) { activationDelay--; return; }
        if (getSpeed() == 2 || getSpeed() == 3)  // 增加对速度3的支持
        {
            setNewX(name.getX() + targetOffsetX);
            setNewY(name.getY() + targetOffsetY);
            chaseTimer++;
            
            // 检查是否与玩家碰撞
            // 定义heart的碰撞箱范围（以heart中心为基准）
            int heartLeft = name.getX() - 4;    // heart中心X - 宽度的一半（原来9，现在4）
            int heartRight = name.getX() + 4;   // heart中心X + 宽度的一半（原来9，现在4）
            int heartTop = name.getY() - 4;     // heart中心Y - 高度的一半（原来8，现在4）
            int heartBottom = name.getY() + 4;  // heart中心Y + 高度的一半（原来8，现在4）
            
            // 定义pollen的碰撞箱范围（以pollen中心为基准）
            int pollenLeft = getX() - 3;        // pollen中心X - 宽度的一半（原来7，现在3）
            int pollenRight = getX() + 3;       // pollen中心X + 宽度的一半（原来7，现在3）
            int pollenTop = getY() - 3;         // pollen中心Y - 高度的一半（原来7，现在3）
            int pollenBottom = getY() + 3;      // pollen中心Y + 高度的一半（原来7，现在3）
            
            // 检查两个碰撞箱是否重叠
            if (pollenRight >= heartLeft && pollenLeft <= heartRight &&
                pollenBottom >= heartTop && pollenTop <= heartBottom)
            {
                if (!name.isInvincible()) {
                    mciSendString(_T("close music\\Hit.mp3"), NULL, 0, NULL);
                    mciSendString(_T("open music\\Hit.mp3"), NULL, 0, NULL);
                    mciSendString(_T("setaudio music\\Hit.mp3 volume to 1000"), NULL, 0, NULL);
                    mciSendString(_T("play music\\Hit.mp3"), NULL, 0, NULL);
                    Sleep(100);
                    name.setHP(name.getHP() - getHP());
                }
                // 无论是否无敌都要重置pollen状态，避免卡住
                setSpeed(1);
                setX(320 - (14 / 2));
                setY(220 - 14);
                chaseTimer = 0;
                active = false;  // 击中玩家的pollen被击败
                wasHit = true;   // 标记为击中玩家
                return;
            }
            
            if (chaseTimer >= 2 * FPS)
            {
                setSpeed(1);
                setX(320 - (14 / 2));
                setY(220 - 14);
                chaseTimer = 0;
                active = false;  // 躲避成功的pollen也被击败，避免单个pollen无限追击
                return;
            }
        }
        else
        {
            chaseTimer = 0;
        }
        if (getX() > getNewX()) setX(getX() - getSpeed());
        else if (getX() < getNewX()) setX(getX() + getSpeed());
        if (getY() > getNewY()) setY(getY() - getSpeed());
        else if (getY() < getNewY()) setY(getY() + getSpeed());
        if (getX() == getNewX() && getY() == getNewY())
        {
            if (getSpeed() == 2 || getSpeed() == 3)  // 增加对速度3的支持
            {
                setSpeed(1);
                setX(320 - (14 / 2));
                setY(220 - 14);
                if (!name.isInvincible()) {
                    mciSendString(_T("close music\\Hit.mp3"), NULL, 0, NULL);
                    mciSendString(_T("open music\\Hit.mp3"), NULL, 0, NULL);
                    mciSendString(_T("setaudio music\\Hit.mp3 volume to 1000"), NULL, 0, NULL);
                    mciSendString(_T("play music\\Hit.mp3"), NULL, 0, NULL);
                    Sleep(100);
                    name.setHP(name.getHP() - getHP());
                }
                // 无论是否无敌都要重置pollen状态，避免卡住
                chaseTimer = 0;
                active = false;  // 击中玩家的pollen被击败
                wasHit = true;   // 标记为击中玩家
            }
            else
            {
                setSpeed(2);
                setNewX(name.getX() + targetOffsetX);
                setNewY(name.getY() + targetOffsetY);
                return;
            }
        }
    }
protected:
    IMAGE pollenA;
    IMAGE pollenB;
    int newX;
    int newY;
    int chaseTimer = 0;
    inline void setNewX(int a)
    {
        newX = a;
    }
    inline void setNewY(int a)
    {
        newY = a;
    }
private:
    inline void generateNewXY()
    {
        int radius = 10;                                 // Define the radius of the circular area
        double angle = (rand() % 360) * 3.14159 / 180.0; // Random angle in radians
        int offsetX = static_cast<int>(radius * cos(angle));
        int offsetY = static_cast<int>(radius * sin(angle));
        newX = x + offsetX;
        newY = y + offsetY;
        if (newX < 0)
            newX = 0;
        if (newX > 640)
            newX = 640;
        if (newY < 0)
            newY = 0;
        if (newY > 480)
            newY = 480;
    }
};

class Pollen_Red : public Pollen
{
private:
    IMAGE pollenRedA;
    IMAGE pollenRedB;

public:
    Pollen_Red() : Pollen()
    {
        loadimage(&pollenRedA, _T("img\\pollen_red1.png"), 14, 8);
        loadimage(&pollenRedB, _T("img\\pollen_red2.png"), 8, 14);
        setHP(3);  // 红色pollen血量设3
    }
    inline IMAGE getPollenA()
    {
        return pollenRedA;
    }
    inline IMAGE getPollenB()
    {
        return pollenRedB;
    }
    void setSpeed(int a) override {
        if (a == 2) speed = 3;
        else speed = a;
    }
    void bulletMove(Heart &name) override {
        // 检查绿色pollen是否被吃掉，如果是则立即回收红色pollen
        if (greenPollenEaten) {
            active = false;
            return;
        }
        
        // 调用基类的bulletMove逻辑
        Pollen::bulletMove(name);
    }
};

class Pollen_Green : public Pollen
{
private:
    IMAGE pollenGreenA;
    IMAGE pollenGreenB;
    bool isEscaping = false;  // 是否处于逃离状态
    int escapeTimer = 0;      // 逃离计时器

public:
    Pollen_Green() : Pollen()
    {
        loadimage(&pollenGreenA, _T("img\\pollen_green1.png"), 14, 8);  // 使用专门的绿色pollen图像
        loadimage(&pollenGreenB, _T("img\\pollen_green2.png"), 8, 14);  // 使用专门的绿色pollen图像
        setHP(3);
        setSpeed(3);
    }
    
    inline IMAGE getPollenA()
    {
        return pollenGreenA;
    }
    
    inline IMAGE getPollenB()
    {
        return pollenGreenB;
    }
    
    void bulletMove(Heart& heart) override {
        if (!active) return;
        
        if (activationDelay > 0) { 
            activationDelay--; 
            return; 
        }
        
        if (isEscaping) {
            // 逃离逻辑：远离玩家
            escapeTimer--;
            
            // 移动远离玩家
            if (getX() > getNewX()) setX(getX() - getSpeed());
            else if (getX() < getNewX()) setX(getX() + getSpeed());
            if (getY() > getNewY()) setY(getY() - getSpeed());
            else if (getY() < getNewY()) setY(getY() + getSpeed());
            
            // 逃离时间结束或出界
            if (escapeTimer <= 0 || getX() <= 240 || getX() >= 400 - 14 || 
                getY() <= 250 || getY() >= 380 - 14) {
                active = false;  // 逃离完成，消失
                return;
            }
            
            // 逃离时与玩家碰撞检测
            int heartLeft = heart.getX() - 4;
            int heartRight = heart.getX() + 4;
            int heartTop = heart.getY() - 4;
            int heartBottom = heart.getY() + 4;
            
            int pollenLeft = getX() - 3;
            int pollenRight = getX() + 3;
            int pollenTop = getY() - 3;
            int pollenBottom = getY() + 3;
            
            if (pollenRight >= heartLeft && pollenLeft <= heartRight &&
                pollenBottom >= heartTop && pollenTop <= heartBottom) {
                // 绿色pollen被吃掉
                if (!heart.isInvincible()) {
                    // 非无敌状态：增加玩家血量
                    int newHP = heart.getHP() + 3;  // 增加3点血量
                    if (newHP > heart.getHP_MAX()) newHP = heart.getHP_MAX();  // 不超过最大血量
                    heart.setHP(newHP);
                    
                    // 播放治疗音效
                    mciSendString(_T("close music\\heal.mp3"), NULL, 0, NULL);
                    mciSendString(_T("open music\\heal.mp3"), NULL, 0, NULL);
                    mciSendString(_T("setaudio music\\heal.mp3 volume to 1000"), NULL, 0, NULL);
                    mciSendString(_T("play music\\heal.mp3"), NULL, 0, NULL);
                    Sleep(100);
                }
                // 无论是否无敌都要消失
                wasHit = true;
                active = false;
                greenPollenEaten = true;  // 设置全局标志：绿色pollen被吃掉
                return;
            }
        } else {
            // 使用基类的追击逻辑，但添加距离检查
            Pollen::bulletMove(heart);
            
            // 检查是否追上（通过距离判断）
            int distanceToPlayer = abs(getX() - heart.getX()) + abs(getY() - heart.getY());
            if (distanceToPlayer < 20) {  // 距离小于20像素时认为追上
                // 追上后，进入逃离模式
                isEscaping = true;
                setSpeed(1);
                escapeTimer = 4 * FPS;  // 逃离4秒
                
                // 设置逃离目标（远离玩家当前位置）
                int escapeDirX = (getX() > heart.getX()) ? 1 : -1;
                int escapeDirY = (getY() > heart.getY()) ? 1 : -1;
                setNewX(getX() + escapeDirX * 120);  // 逃离120像素距离
                setNewY(getY() + escapeDirY * 120);
                
                // 边界检查（限制在白色游戏框框内）
                if (getNewX() < 240) setNewX(240);
                if (getNewX() > 400 - 14) setNewX(400 - 14);
                if (getNewY() < 250) setNewY(250);
                if (getNewY() > 380 - 14) setNewY(380 - 14);
            }
        }
    }
};

// 道具基类
class Item {
protected:
    int x, y; // 左上角坐标
    int width, height;
    IMAGE img;
    bool visible;
    int lifeTime; // 新增：存活帧数
public:
    Item(int w, int h) : width(w), height(h), visible(true), lifeTime(5 * FPS) {} // 默认5秒
    virtual ~Item() {}
    virtual void loadImage(const TCHAR* path) { loadimage(&img, path, width, height); }
    virtual void draw() {
        if (visible) {
            // 使用透明模式绘制图像
            putimage(x, y, &img, SRCINVERT);
        }
    }
    virtual bool isVisible() const { return visible; }
    virtual void setVisible(bool v) { visible = v; }
    virtual int getX() const { return x; }
    virtual int getY() const { return y; }
    virtual int getWidth() const { return width; }
    virtual int getHeight() const { return height; }
    // 碰撞箱检测
    virtual bool checkCollision(int px, int py, int pwidth, int pheight) {
        return visible && (x < px + pwidth && x + width > px && y < py + pheight && y + height > py);
    }
    // 存活帧递减
    virtual void tickLife() { if (visible && lifeTime > 0) --lifeTime; if (lifeTime <= 0) visible = false; }
    // 派生类可重写
    virtual void onPick(Heart& heart) = 0;
};

// 衍生瓶子类
class Bottle : public Item {
public:
    enum BottleType { BLOOD, SHIELD } type;
    Bottle(BottleType t) : Item(12, 16), type(t) {
        if (type == BLOOD) loadImage(_T("img\\bottle_blood.png"));
        else loadImage(_T("img\\bottle_shield.png"));
    }
    void setPos(int nx, int ny) { x = nx; y = ny; }
    void onPick(Heart& heart) override {
        if (type == BLOOD) {
            int newHP = heart.getHP() + 5;
            if (newHP > heart.getHP_MAX()) newHP = heart.getHP_MAX();
            heart.setHP(newHP);
            
            // 播放治疗音效
            mciSendString(_T("close music\\heal.mp3"), NULL, 0, NULL);
            mciSendString(_T("open music\\heal.mp3"), NULL, 0, NULL);
            mciSendString(_T("setaudio music\\heal.mp3 volume to 1000"), NULL, 0, NULL);
            mciSendString(_T("play music\\heal.mp3"), NULL, 0, NULL);
            Sleep(100);
        } else if (type == SHIELD) {
            heart.setInvincible(5 * FPS); // 5秒无敌
        }
        visible = false;
    }
};

void pressShow();
void initPollenPool(std::vector<std::shared_ptr<Pollen>> &pool, int &activeCount);
void increasePollen(std::vector<std::shared_ptr<Pollen>> &pool, int &activeCount, int addNum, int currentLevel);

// 根据等级计算红色pollen生成概率
int getRedPollenProbability(int level) {
    if (level < 4) return 0;           // LV1-3: 0%
    if (level == 4) return 10;         // LV4: 10%
    if (level >= 8) return 50;         // LV8: 50%
    return 10 + (level - 4) * 10;      // LV5-7: 20%, 30%, 40%
}

// 根据等级计算绿色pollen生成概率
int getGreenPollenProbability(int level) {
    if (level < 4) return 0;          
    if (level == 4) return 15;         
    if (level == 5) return 20;        
    if (level == 6) return 25;         
    if (level == 7) return 30;         
    if (level >= 8) return 25;         
    return 0;
}

// 游戏数据结构
struct GameData {
    int highestLevel = 0;
    int totalGameTime = 0;  // 总游戏时间（秒）
    int gamesPlayed = 0;    // 游戏次数
    int totalWins = 0;      // 胜利次数
    int totalDeaths = 0;    // 死亡次数
};

// 游戏数据文件操作函数（文本格式）
void saveGameData(const GameData& data) {
    std::ofstream file("gamedata.txt");
    if (file.is_open()) {
        file << "highestLevel=" << data.highestLevel << std::endl;
        file << "totalGameTime=" << data.totalGameTime << std::endl;
        file << "gamesPlayed=" << data.gamesPlayed << std::endl;
        file << "totalWins=" << data.totalWins << std::endl;
        file << "totalDeaths=" << data.totalDeaths << std::endl;
        file.close();
    }
}

void loadGameData(GameData& data) {
    std::ifstream file("gamedata.txt");
    if (!file.is_open()) {
        // 文件不存在则写入初始数据
        saveGameData(data);
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        int value = atoi(line.substr(eq + 1).c_str());
        if (key == "highestLevel") data.highestLevel = value;
        else if (key == "totalGameTime") data.totalGameTime = value;
        else if (key == "gamesPlayed") data.gamesPlayed = value;
        else if (key == "totalWins") data.totalWins = value;
        else if (key == "totalDeaths") data.totalDeaths = value;
    }
    file.close();
}

// 大厅界面显示函数
void showLobby(const GameData& data) {
    BeginBatchDraw();
    cleardevice();
    setbkcolor(BLACK);
    
    // 加载并显示上方图片 (column.png) - 放在上方，紧贴窗口上边
    IMAGE columnImg;
    loadimage(&columnImg, _T("img\\column.png"), 213, 86);  // 原始尺寸
    putimage(214, 0, &columnImg);  // 居中显示 (640-213)/2 = 214，y=0紧贴上边
    
    // 标题 - 放在上方图片下方
    settextstyle(40, 20, _T("Determination Sans Web"));
    settextcolor(WHITE);
    outtextxy(210, 120, _T("GAME LOBBY"));  // 右移10个像素
    
    // 统计数据 - 重新排版，使用更平衡的布局
    settextstyle(22, 11, _T("Determination Sans Web"));
    settextcolor(WHITE);
    
    // 左栏统计
    char levelText[100];
    sprintf(levelText, "Highest Level: %d", data.highestLevel);
    outtextxy(130, 180, levelText);
    
    char gamesText[100];
    sprintf(gamesText, "Games Played: %d", data.gamesPlayed);
    outtextxy(130, 210, gamesText);
    
    char winsText[100];
    sprintf(winsText, "Total Wins: %d", data.totalWins);
    outtextxy(130, 240, winsText);
    
    // 右栏统计 - 左移以避免重叠
    int hours = data.totalGameTime / 3600;
    int minutes = (data.totalGameTime % 3600) / 60;
    int seconds = data.totalGameTime % 60;
    char timeText[100];
    sprintf(timeText, "Total Time: %02d:%02d:%02d", hours, minutes, seconds);
    outtextxy(320, 180, timeText);
    
    char deathsText[100];
    sprintf(deathsText, "Total Deaths: %d", data.totalDeaths);
    outtextxy(320, 210, deathsText);
    
    // 胜率计算
    float winRate = (data.gamesPlayed > 0) ? (float)data.totalWins / data.gamesPlayed * 100 : 0;
    char winRateText[100];
    sprintf(winRateText, "Win Rate: %.1f%%", winRate);
    outtextxy(320, 240, winRateText);
    
    // 操作提示 - 合并为一行，减少间距
    settextstyle(20, 10, _T("Determination Sans Web"));
    settextcolor(YELLOW);
    outtextxy(130, 290, _T("[ENTER] Start Game"));
    outtextxy(130, 320, _T("[ESC] Exit Game"));    
    // 为所有文本添加白色边框 - 调整边框大小以适应新布局
    setlinecolor(WHITE);
    setlinestyle(PS_SOLID, 5, NULL, 0);  // 设置线条粗细为5像素
    rectangle(110, 115, 530, 350);  // 两边各加长10像素：左边界从120减到110，右边界从520加到530
    
    // 加载并显示下方图片 (you.png) - 放在下方，紧贴窗口下边
    IMAGE youImg;
    loadimage(&youImg, _T("img\\you.png"), 336, 113);  // 原始尺寸
    putimage(152, 367, &youImg);  // 居中显示 (640-336)/2 = 152，y=480-113=367紧贴下边
    
    EndBatchDraw();
}

// 全局音乐状态变量
bool g_musicInitialized = false;
bool g_lobbyMusicPlaying = false;

// 重置音乐状态函数
void resetMusicState() {
    g_musicInitialized = false;
    g_lobbyMusicPlaying = false;
}

// 播放大厅音乐函数
void playLobbyMusic() {
    if (!g_lobbyMusicPlaying) {
        mciSendString(_TEXT("open music\\Fallen_Down.mp3"), NULL, 0, NULL);
        mciSendString(_TEXT("setaudio music\\Fallen_Down.mp3 volume to 600"), NULL, 0, NULL);
        mciSendString(_TEXT("play music\\Fallen_Down.mp3 repeat"), NULL, 0, NULL);
        g_lobbyMusicPlaying = true;
    }
}

// 关闭大厅音乐函数
void stopLobbyMusic() {
    if (g_lobbyMusicPlaying) {
        mciSendString(_TEXT("close music\\Fallen_Down.mp3"), NULL, 0, NULL);
        g_lobbyMusicPlaying = false;
    }
}

// 入场动画函数
void entranceAnimation() {
    // 加载flower_1图像
    IMAGE flowerImg;
    loadimage(&flowerImg, _T("img\\flower_1.png"), 96, 99);
    
    // 动画参数
    const int targetX = 320 - 48;  // 目标X位置 (320 - width/2)
    const int targetY = 220 - 99;  // 目标Y位置 (220 - height)
    const int startY = -99;        // 起始Y位置 (窗口上方外)
    const int animationFrames = 60; // 动画帧数 (1秒)
    
    // 播放入场音效（可选）
    // mciSendString(_TEXT("play music\\entrance.mp3"), NULL, 0, NULL);
    
    // 执行动画
    for (int frame = 0; frame <= animationFrames; frame++) {
        BeginBatchDraw();
        cleardevice();
        setbkcolor(BLACK);
        
        // 计算当前Y位置（缓动效果）
        float progress = (float)frame / animationFrames;
        float easeProgress = 1.0f - (1.0f - progress) * (1.0f - progress); // 缓出效果
        int currentY = startY + (int)((targetY - startY) * easeProgress);
        
        // 绘制flower_1
        putimage(targetX, currentY, &flowerImg);
        
        EndBatchDraw();
        Sleep(1000 / FPS); // 60FPS
    }
    
    // 短暂停留
    Sleep(300);
    
    // 绘制游戏边框
    BeginBatchDraw();
    cleardevice();
    setbkcolor(BLACK);
    
    // 绘制flower_1在最终位置
    putimage(targetX, targetY, &flowerImg);
    
    // 绘制游戏边框 - 使用与实际游戏一致的坐标
    setlinecolor(WHITE);
    setlinestyle(PS_SOLID, 5, NULL, 0);  // 设置线条粗细为5像素
    rectangle(240, 250, 400, 380);
    
    EndBatchDraw();
    
    // 短暂停留
    Sleep(200);
    
    // 显示心形
    IMAGE heartImg;
    loadimage(&heartImg, _T("img\\heart.png"), 18, 17);
    
    BeginBatchDraw();
    putimage(320, 315, &heartImg); // 心形显示，与实际游戏初始位置一致
    EndBatchDraw();
    
    // 最终停留
    Sleep(500);
    
    // 播放游戏对局音乐 - 只在第一次初始化
    if (!g_musicInitialized) {
        mciSendString(_TEXT("open music\\Flowey.mp3"), NULL, 0, NULL);
        mciSendString(_T("open music\\Hit.mp3"), NULL, 0, NULL);
        mciSendString(_TEXT("setaudio music\\Flowey.mp3 volume to 800"), NULL, 0, NULL);
        mciSendString(_TEXT("setaudio music\\Hit.mp3 volume to 1000"), NULL, 0, NULL);
        g_musicInitialized = true;
    } else {
        // 如果已经初始化过，确保文件是打开的
        mciSendString(_TEXT("open music\\Flowey.mp3"), NULL, 0, NULL);
        mciSendString(_T("open music\\Hit.mp3"), NULL, 0, NULL);
        mciSendString(_TEXT("setaudio music\\Flowey.mp3 volume to 800"), NULL, 0, NULL);
        mciSendString(_TEXT("setaudio music\\Hit.mp3 volume to 1000"), NULL, 0, NULL);
    }
    mciSendString(_TEXT("play music\\Flowey.mp3 repeat"), NULL, 0, NULL);
    
    // 确保平滑过渡到游戏
    Sleep(200);
}

int main()
{
  FreeConsole();
  
  // 加载游戏数据
  GameData gameData;
  loadGameData(gameData);
  
#ifndef UI
  initgraph(640, 480, 2); //'2'(EX_NOCLOSE) stands for "Unable the close button".
  setbkcolor(BLACK);      // LIGHTGRAY is needed if build the "[PRESS 2 OR ENTER]".
  cleardevice();
  Sleep(1000);
  mciSendString(_TEXT("open music\\Splash.mp3"), NULL, 0, NULL);
  mciSendString(_TEXT("setaudio music\\Splash.mp3 volume to 600"), NULL, 0, NULL);
  mciSendString(_TEXT("play music\\Splash.mp3 repeat"), NULL, 0, NULL);
  Sleep(400);
  IMAGE SPLASH;
  loadimage(&SPLASH, _T("img\\splash.png"));
  putimage(0, 0, &SPLASH);
  Sleep(3100);
  mciSendString(_TEXT("close music\\Splash.mp3"), NULL, 0, NULL);
  pressShow();
  bool Flag = true;
  while (Flag)
  {
    if (GetAsyncKeyState(VK_RETURN) && 0x8000)
    {
      Flag = false;
      cleardevice();
    }
    else if (GetAsyncKeyState('2') && 0x8000)
    {
      Flag = false;
      cleardevice();
    }
    else if (GetAsyncKeyState(VK_ESCAPE) && 0x8000)
    {
      abort();
    }
  }
  
  // 大厅界面
  bool inLobby = true;
  bool enterPressed = false;
  
  // 清除按键状态残留 - 简化处理
  Sleep(300);
  
  while (inLobby) {
    // 播放大厅音乐
    playLobbyMusic();
    
    showLobby(gameData);
    
    // 检测Enter键
    if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
      if (!enterPressed) {
        enterPressed = true;
        inLobby = false;  // 进入游戏
        
        // 关闭大厅音乐
        stopLobbyMusic();
        
        // 播放入场动画
        entranceAnimation();
      }
    } else {
      enterPressed = false;
    }
    
    // 检测ESC键 - 简化处理
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
      // 关闭大厅音乐
      stopLobbyMusic();
      saveGameData(gameData);  // 保存数据后退出
      return 0;
    }
    
    Sleep(100);
  }
#endif
#ifndef Game_kernel
game_start:
  // 初始化随机数种子
  srand((unsigned int)time(NULL));
  
  // 概率平衡系统
  int consecutiveRareEvents = 0;  // 连续小概率事件计数
  bool forceMinPollen = false;    // 是否强制生成最少数量
  
  // 全局躲避计数系统
  int globalDodgeCount = 0;
  int globalDodgeThreshold = 3;
  
  // 游戏时间跟踪
  clock_t gameStartTime = clock();
  int gameTimeSeconds = 0;
  
  Heart name;
  Flower Flower;
  std::vector<std::shared_ptr<Pollen>> pollenPool; // Object Pool
  int activePollenCount = 2;
  initPollenPool(pollenPool, activePollenCount);
  Sleep(500);
  static int startTime = 0;
  static int frameTime = 0;
  // 音乐已在入场动画中初始化
  static int flowerSwitchTime = 0; // Time counter for switching flower images
  static bool useFlowerA = true;

  // 道具生成相关变量
  std::vector<std::shared_ptr<Item>> items; // 当前场上道具
  int itemGenTimer = 0; // 帧计数
  int itemGenInterval = (5 + rand() % 6) * FPS; // 5~10秒

  while (name.getHP() <= 0 && name.getLV() != 9)
  {
    startTime = clock();
    name.gameControl();
    
    // 道具生成逻辑
    itemGenTimer++;
    if (itemGenTimer >= itemGenInterval) {
      int randVal = rand() % 4;
      if (randVal < 2) {
        // 1/2概率生成血瓶
        std::shared_ptr<Bottle> newBottle = std::make_shared<Bottle>(Bottle::BLOOD);
        int minX = 240, maxX = 400 - 12;
        int minY = 250, maxY = 380 - 16;
        int rx = minX + rand() % (maxX - minX + 1);
        int ry = minY + rand() % (maxY - minY + 1);
        newBottle->setPos(rx, ry);
        items.push_back(newBottle);
      } else if (randVal == 2) {
        // 1/4概率生成护盾瓶
        std::shared_ptr<Bottle> newBottle = std::make_shared<Bottle>(Bottle::SHIELD);
        int minX = 240, maxX = 400 - 12;
        int minY = 250, maxY = 380 - 16;
        int rx = minX + rand() % (maxX - minX + 1);
        int ry = minY + rand() % (maxY - minY + 1);
        newBottle->setPos(rx, ry);
        items.push_back(newBottle);
      }
      // 1/4概率什么都不生成（randVal==3）
      // 重置计时器和下次间隔
      itemGenTimer = 0;
      itemGenInterval = (5 + rand() % 6) * FPS;
    }
    
    // 修复：使用帧数而不是毫秒来计算切换时间
    flowerSwitchTime++;
    
    if (flowerSwitchTime >= 30)  // 每30帧切换一次 (约500ms at 60FPS)
    {
      useFlowerA = !useFlowerA;
      flowerSwitchTime = 0;
    }
    BeginBatchDraw();
    cleardevice();
    
    // 检查是否所有pollen都被击败
    bool allPollenDefeated = true;
    for(auto &pollen : pollenPool) {
        if(pollen->active) {
            allPollenDefeated = false;
            break;
        }
    }
    
    // 如果所有pollen都被击败，根据等级生成新的pollen
    if(allPollenDefeated) {
        activePollenCount = 0;
        greenPollenEaten = false;  // 重置全局标志：新的一轮pollen生成
        int minPollen = std::max(1, name.getLV() / 2);
        int maxPollen = std::min(7, name.getLV());
        int newCount;
        if (forceMinPollen) {
            newCount = minPollen;
            forceMinPollen = false;
            consecutiveRareEvents = 0;
        } else {
            if (name.getLV() == 1) {
                int randVal = rand() % 3;
                if (randVal < 2) {
                    newCount = 1;
                } else {
                    newCount = 2;
                    consecutiveRareEvents++;
                }
            } else if (name.getLV() == 2) {
                int randVal = rand() % 10;
                if (randVal < 6) {
                    newCount = 1;
                } else if (randVal < 9) {
                    newCount = 2;
                } else {
                    newCount = 3;
                    consecutiveRareEvents++;
                }
            } else if (name.getLV() == 3) {
                int randVal = rand() % 20;
                if (randVal < 10) {
                    newCount = 2;
                } else {
                    newCount = 3;
                    consecutiveRareEvents++;
                }
            } else {
                newCount = rand() % (maxPollen - minPollen + 1) + minPollen;
            }
            if (consecutiveRareEvents >= 2) {
                forceMinPollen = true;
            }
        }
        increasePollen(pollenPool, activePollenCount, newCount, name.getLV());
    }
    
    for (auto &pollen : pollenPool)
    {
      if (pollen->active)
      {
        pollen->bulletMove(name);
        if (!pollen->active && !pollen->wasHit)
        {
          globalDodgeCount++;
          if (globalDodgeCount >= globalDodgeThreshold)
          {
            name.levelUp();
            globalDodgeCount = 0;
            globalDodgeThreshold *= 2;
          }
        }
        pollen->imageSwitchTimer++;
        int switchPeriod = 120 + (pollen->getX() % 61);
        int randomOffset = (pollen->getY() % 7) - 3;
        int adjustedTimer = pollen->imageSwitchTimer + randomOffset;
        if (pollen->switchCooldown > 0) pollen->switchCooldown--;
        bool usePollenA = pollen->usePollenAState;
        if (pollen->switchCooldown == 0 && (adjustedTimer / switchPeriod) % 2 != pollen->lastSwitchState) {
          pollen->usePollenAState = !pollen->usePollenAState;
          pollen->lastSwitchState = (adjustedTimer / switchPeriod) % 2;
          pollen->switchCooldown = switchPeriod;
          usePollenA = pollen->usePollenAState;
        }
        // 统一绘制逻辑
        pollen->draw();
      }
    }

    // 绘制道具
    for (auto& item : items) {
      if (item->isVisible()) item->draw();
      item->tickLife();
    }
    // 移除已消失的道具
    items.erase(std::remove_if(items.begin(), items.end(), [](const std::shared_ptr<Item>& item){ return !item->isVisible(); }), items.end());

    // 检查heart与道具碰撞
    for (auto& item : items) {
      if (item->isVisible() && item->checkCollision(name.getX(), name.getY(), 18, 17)) {
        item->onPick(name);
      }
    }

    name.heartDataShow();
    IMAGE heartImage = name.getHeart();
    putimage(name.getX(), name.getY(), &heartImage); // Display the appropriate heart image
    // 在绿色心上叠加显示无敌剩余秒数
    if (name.isInvincible()) {
      setbkmode(TRANSPARENT);
      settextcolor(WHITE);
      settextstyle(15, 8, _T("Determination Sans Web"));
      int seconds = (name.getInvincibleTimer() + FPS - 1) / FPS; // 向上取整
      char secStr[8];
      sprintf(secStr, "%d", seconds);
      int textW = textwidth(secStr);
      int textH = textheight(secStr);
      int heartCenterX = name.getX() + 18 / 2;
      int heartCenterY = name.getY() + 17 / 2;
      int textX = heartCenterX - textW / 2 + 1;
      int textY = heartCenterY - textH / 2;
      outtextxy(textX, textY, secStr);
    }
    IMAGE enemyImage = useFlowerA ? Flower.getFlowerA() : Flower.getFlowerB();
    putimage(Flower.getX(), Flower.getY(), &enemyImage);
    name.tickInvincible();
    EndBatchDraw();
    frameTime = clock() - startTime;
    int sleepTime = (1000 / FPS) - frameTime;
    if (sleepTime > 0)
    {
      Sleep(sleepTime);
    }
  }
  if (name.getHP() <= 0)
  {
    // 统计游戏时间
    gameTimeSeconds = (int)((clock() - gameStartTime) / CLOCKS_PER_SEC);
    gameData.totalGameTime += gameTimeSeconds;
    gameData.gamesPlayed++;
    if (name.getLV() > gameData.highestLevel) gameData.highestLevel = name.getLV();
    gameData.totalDeaths++;
    saveGameData(gameData);
    
    // 播放失败音效
    mciSendString(_TEXT("close music\\Flowey.mp3"), NULL, 0, NULL);
    mciSendString(_TEXT("close music\\Hit.mp3"), NULL, 0, NULL);
    mciSendString(_TEXT("close music\\youlose.mp3"), NULL, 0, NULL);  // 先关闭，确保状态正确
    mciSendString(_TEXT("open music\\youlose.mp3"), NULL, 0, NULL);
    mciSendString(_TEXT("setaudio music\\youlose.mp3 volume to 1000"), NULL, 0, NULL);
    mciSendString(_TEXT("play music\\youlose.mp3"), NULL, 0, NULL);
    
    clearrectangle(245, 255, 397, 377);
    settextstyle(36, 18, _T("Determination Sans Web"));
    settextcolor(RED);
    outtextxy(245, 300, _T("YOU DIED!"));
    
    // 显示失败专用的flowey_4图像
    IMAGE flowey4Img;
    loadimage(&flowey4Img, _T("img\\flowey_4.png"), 96, 99);
    putimage(Flower.getX(), Flower.getY(), &flowey4Img);
    
    Sleep(3000);
    
    // 返回大厅
    goto lobby_return;
  }
  else if (name.getLV() == 9)
  {
    // 统计游戏时间
    gameTimeSeconds = (int)((clock() - gameStartTime) / CLOCKS_PER_SEC);
    gameData.totalGameTime += gameTimeSeconds;
    gameData.gamesPlayed++;
    if (name.getLV() > gameData.highestLevel) gameData.highestLevel = name.getLV();
    gameData.totalWins++;
    saveGameData(gameData);
    
    // 播放胜利音效
    mciSendString(_TEXT("close music\\Flowey.mp3"), NULL, 0, NULL);
    mciSendString(_TEXT("close music\\Hit.mp3"), NULL, 0, NULL);
    mciSendString(_TEXT("close music\\youwin.mp3"), NULL, 0, NULL);  // 先关闭，确保状态正确
    mciSendString(_TEXT("open music\\youwin.mp3"), NULL, 0, NULL);
    mciSendString(_TEXT("setaudio music\\youwin.mp3 volume to 1000"), NULL, 0, NULL);
    mciSendString(_TEXT("play music\\youwin.mp3"), NULL, 0, NULL);
    
    clearrectangle(245, 255, 397, 377);
    settextstyle(36, 18, _T("Determination Sans Web"));
    settextcolor(YELLOW);
    outtextxy(255, 300, _T("YOU WON!"));
    
    // 显示胜利专用的flower_3图像
    IMAGE flower3Img;
    loadimage(&flower3Img, _T("img\\flower_3.png"), 96, 99);
    putimage(Flower.getX(), Flower.getY(), &flower3Img);
    
    // 动态等待胜利音效播放时长
    char lengthStr[128] = {0};
    mciSendString(_TEXT("status music\\youwin.mp3 length"), lengthStr, sizeof(lengthStr), NULL);
    int winMusicLength = atoi(lengthStr);
    if (winMusicLength <= 0) winMusicLength = 2000; // 默认2秒
    Sleep(winMusicLength);
    
    // 返回大厅
    goto lobby_return;
  }
  
lobby_return:
  // 重置音乐状态，确保下次进入游戏时重新初始化
  resetMusicState();
  
  // 重新进入大厅循环
  inLobby = true;
  
  // 清除按键状态残留 - 简化处理
  Sleep(300);
  enterPressed = false;
  
  // 重新播放大厅音乐
  playLobbyMusic();
  
  while (inLobby) {
    showLobby(gameData);
    
    // 检测Enter键
    if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
      if (!enterPressed) {
        enterPressed = true;
        inLobby = false;  // 进入游戏
        
        // 关闭大厅音乐
        stopLobbyMusic();
        
        // 播放入场动画
        entranceAnimation();
      }
    } else {
      enterPressed = false;
    }
    
    // 检测ESC键 - 简化处理
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
      // 关闭大厅音乐
      stopLobbyMusic();
      saveGameData(gameData);  // 保存数据后退出
      return 0;
    }
    
    Sleep(100);
  }
  goto game_start;  // 重新开始游戏
#endif
  getchar();
  return 0;
}

void pressShow()
{
  settextstyle(20, 10, _T("Determination Sans Web"));
  settextcolor(LIGHTGRAY);
  setbkmode(TRANSPARENT);
  outtextxy(230, 360, _T("[PRESS 2 OR ENTER]"));
}

void initPollenPool(std::vector<std::shared_ptr<Pollen>> &pool, int &activeCount)
{
  pool.clear();
  for (int i = 0; i < POLLEN_POOL_SIZE; ++i)
  {
    std::shared_ptr<Pollen> p;
    // 根据等级动态选择创建不同类型的pollen
    int redProb = getRedPollenProbability(1);  // 初始等级为1
    int greenProb = getGreenPollenProbability(1);
    int randVal = rand() % 100;
    
    if (randVal < greenProb) {
      // 生成绿色pollen
      p = std::make_shared<Pollen_Green>();
    } else if (randVal < greenProb + redProb) {
      // 生成红色pollen
      p = std::make_shared<Pollen_Red>();
    } else {
      // 生成普通pollen
      p = std::make_shared<Pollen>();
    }
    
    p->active = (i < activeCount);
    p->wasHit = false;  // 重置击中标记
    // 为前两个活跃的pollen设置不同的初始位置
    if (i < activeCount)
    {
      if (i == 0)
      {
        p->setX(280 - 7);
        p->setY(180 - 14);
      }
      else if (i == 1)
      {
        p->setX(360 - 7);
        p->setY(180 - 14);
      }
    }
    // 分配不同的激活延迟和追击偏移
    p->activationDelay = 20 * i; // 每个pollen延迟20帧
    p->targetOffsetX = (rand() % 21) - 10; // -10~10
    p->targetOffsetY = (rand() % 21) - 10;
    
    // 为每个pollen设置不同的起始图像切换计时器，让它们从一开始就不同步
    p->imageSwitchTimer = rand() % 50;  // 0-49的随机起始值
    
    pool.push_back(p);
  }
}

void increasePollen(std::vector<std::shared_ptr<Pollen>> &pool, int &activeCount, int addNum, int currentLevel)
{
  int newCount = std::min((int)pool.size(), activeCount + addNum);
  for (int i = activeCount; i < newCount; ++i)
  {
    // 根据等级动态选择创建不同类型的pollen
    int redProb = getRedPollenProbability(currentLevel);
    int greenProb = getGreenPollenProbability(currentLevel);
    int randVal = rand() % 100;
    
    if (randVal < greenProb) {
      // 生成绿色pollen
      pool[i] = std::make_shared<Pollen_Green>();
    } else if (randVal < greenProb + redProb) {
      // 生成红色pollen
      pool[i] = std::make_shared<Pollen_Red>();
    } else {
      // 生成普通pollen
      pool[i] = std::make_shared<Pollen>();
    }
    
    pool[i]->active = true;
    pool[i]->wasHit = false;  // 重置击中标记
    
    // 修复：使用相对索引计算位置，避免重复
    int relativeIndex = i - activeCount;
    int positionIndex = relativeIndex % 4;
    switch (positionIndex)
    {
      case 0: pool[i]->setX(280 - 7); pool[i]->setY(180 - 14); break;
      case 1: pool[i]->setX(360 - 7); pool[i]->setY(180 - 14); break;
      case 2: pool[i]->setX(280 - 7); pool[i]->setY(260 - 14); break;
      case 3: pool[i]->setX(360 - 7); pool[i]->setY(260 - 14); break;
    }
    
    pool[i]->setSpeed(1);
    
    // 修复：使用相对索引计算延迟，确保每个pollen有不同的延迟
    pool[i]->activationDelay = 20 * relativeIndex;
    
    // 重新生成随机偏移
    pool[i]->targetOffsetX = (rand() % 21) - 10;
    pool[i]->targetOffsetY = (rand() % 21) - 10;
    
    // 为新pollen设置不同的起始图像切换计时器
    pool[i]->imageSwitchTimer = rand() % 50;  // 0-49的随机起始值
  }
  activeCount = newCount;
}
