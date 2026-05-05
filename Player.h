#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <graphics.h>

class Player
{
public:
    float x;
    float y;
    float speed;
    int radius;
    int hp;
    int invincibleTimer;
    int defenseTimer;
    int defenseCooldown;
    bool hurtState;
    bool usingDefense;

    Player();

    void reset();
    void handleInput();
    void update();
    void draw(IMAGE* idleImg, IMAGE* hurtImg, bool hasIdle, bool hasHurt);
    void hurt();
    void startDefense();

    float getX() const;
    float getY() const;
    int getRadius() const;
    int getHp() const;
    bool isInvincible() const;
    bool isUsingDefense() const;
    int getDefenseTimer() const;
    int getDefenseCooldown() const;
};
