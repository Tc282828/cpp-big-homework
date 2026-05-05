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
