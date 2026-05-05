#pragma once

#include "Player.h"
#include "Skill.h"

#include <graphics.h>
#include <vector>

// 游戏当前处在哪个界面
enum GameState
{
    MENU,
    PLAYING,
    WIN,
    GAME_OVER
};

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
    bool resourcesReady;

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
    void drawUI();
    void drawResourceError();
    void getRandomEdgePoint(float* startX, float* startY);
};
