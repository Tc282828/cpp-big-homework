#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <graphics.h>

// 玩家类：保存格温的位置、血量和受伤无敌状态。
class Player
{
public:
    float x;
    float y;
    float speed;
    int radius;
    int hp;
    int invincibleTimer;
    bool hurtState;

    Player();

    void reset();
    void handleInput();
    void update();
    void draw(IMAGE* idleImg, IMAGE* hurtImg, bool hasIdle, bool hasHurt);
    void hurt();

    float getX() const;
    float getY() const;
    int getRadius() const;
    int getHp() const;
    bool isInvincible() const;
};
