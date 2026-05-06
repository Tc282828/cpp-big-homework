#pragma once

#include "Player.h"
#include "Skill.h"

#include <graphics.h>
#include <vector>

// 游戏当前处在哪个界面
enum GameState
{
    MENU,       // 开始菜单
    PLAYING,    // 游戏进行中
    WIN,        // 存活 60 秒后胜利
    GAME_OVER   // 血量为 0 后失败
};

// Game 类负责整个游戏流程：加载资源、处理输入、更新逻辑、绘制画面
class Game
{
public:
    // 窗口大小
    int width;
    int height;

    // 当前游戏状态
    GameState state;

    // 玩家对象和敌方技能列表
    Player player;
    std::vector<Skill> skills;

    // 游戏计时和分数
    int frameCount;
    int surviveFrame;
    int score;

    // 技能生成计时，spawnInterval 越小技能越密集
    int spawnTimer;
    int spawnInterval;

    // running 控制主循环，resourcesReady 表示图片是否加载成功
    bool running;
    bool resourcesReady;

    // 防止失败音效在 GAME_OVER 界面重复播放
    bool hasPlayedGameOverSound;

    // 游戏中用到的图片资源
    IMAGE imgBg;
    IMAGE imgCover;
    IMAGE imgEnd;
    IMAGE imgGwenIdle;
    IMAGE imgGwenHurt;
    IMAGE imgEzQ;
    IMAGE imgAsheR;
    IMAGE imgLuxWarning;
    IMAGE imgLuxBoom;
    IMAGE imgHeart;
    IMAGE imgWMist;

    // 每张图片是否加载成功
    bool hasBg;
    bool hasCover;
    bool hasEnd;
    bool hasGwenIdle;
    bool hasGwenHurt;
    bool hasEzQ;
    bool hasAsheR;
    bool hasLuxWarning;
    bool hasLuxBoom;
    bool hasHeart;
    bool hasWMist;

    Game();

    // 游戏主流程函数
    void run();
    void init();
    void loadResources();
    void resetGame();

    // 每一帧的输入、更新和绘制
    void handleInput();
    void update();
    void draw();

    // 技能生成和不同界面绘制
    void spawnSkill();
    void drawMenu();
    void drawPlaying();
    void drawWin();
    void drawGameOver();
    void drawUI();
    void drawResourceError();

    // 在窗口边缘随机取一个技能出生点
    void getRandomEdgePoint(float* startX, float* startY);

    // 给飞行技能生成一条穿过玩家附近的长弹道
    void getLineSkillPoint(float* startX, float* startY, float* targetX, float* targetY);
};
