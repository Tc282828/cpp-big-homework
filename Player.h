#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <graphics.h>

// Player 类保存玩家的位置、血量、防御和绘制逻辑
class Player
{
public:
    // 玩家中心点坐标
    float x;
    float y;

    // 移动速度、碰撞半径和血量
    float speed;
    int radius;
    int hp;

    // 受伤后的无敌时间，用来做闪烁和防止连续扣血
    int invincibleTimer;

    // J 防御的持续时间、冷却时间和当前状态
    int defenseTimer;
    int defenseCooldown;

    // hurtState 控制受击图片，usingDefense 控制防御迷雾
    bool hurtState;
    bool usingDefense;

    // 右键移动的目标点和当前是否正在自动移动
    bool movingByMouse;
    float targetX;
    float targetY;

    Player();

    // 重置、输入、更新和绘制
    void reset();
    void handleInput();
    void update();
    void draw(IMAGE* idleImg, IMAGE* hurtImg, bool hasIdle, bool hasHurt);

    // 受伤扣血和开始 J 防御
    void hurt();
    void startDefense();

    // 右键点击后设置移动目标，每帧自动向目标点移动
    void setMoveTarget(float newTargetX, float newTargetY);
    void updateMouseMove();

    // 给 Game 类读取玩家状态
    float getX() const;
    float getY() const;
    int getRadius() const;
    int getHp() const;
    bool isInvincible() const;
    bool isUsingDefense() const;
    int getDefenseTimer() const;
    int getDefenseCooldown() const;
};
