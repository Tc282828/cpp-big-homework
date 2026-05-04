#pragma once

#include "Player.h"
#include "Skill.h"

#include <graphics.h>
#include <vector>

// 游戏状态。
enum GameState
{
    MENU,
    PLAYING,
    WIN,
    GAME_OVER
};

// Game 类：负责资源、输入、更新、绘制和主循环。
class Game
{
public:
    int width;
    int height;
    GameState state;
    Player player;
    std::vector<Skill> skills;
    int frameCount;
    int surviveFrame;
    int score;
    int spawnTimer;
    int spawnInterval;
    bool running;

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

    Game();

    void run();
    void init();
    void loadResources();
    void resetGame();
    void handleInput();
    void update();
    void draw();
    void spawnSkill();
    void drawMenu();
    void drawPlaying();
    void drawWin();
    void drawGameOver();
    void drawFallbackMap();
    void drawUI();
};
