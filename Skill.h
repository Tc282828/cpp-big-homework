#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <graphics.h>

// 敌方技能类型。
enum SkillType
{
    EZ_Q,
    ASHE_R,
    LUX_E
};

// 技能类：表示飞行弹道或 Lux E 范围技能。
class Skill
{
public:
    SkillType type;
    float x;
    float y;
    float vx;
    float vy;
    float speed;
    int width;
    int height;
    int radius;
    int timer;
    int warningTime;
    int boomTime;
    bool active;
    bool hasDamaged;
    double angle;

    Skill();

    static Skill createEzQ(float startX, float startY, float targetX, float targetY);
    static Skill createAsheR(float startX, float startY, float targetX, float targetY);
    static Skill createLuxE(float x, float y);

    void update();
    void draw(IMAGE* ezImg, IMAGE* asheImg, IMAGE* warningImg, IMAGE* boomImg,
        bool hasEz, bool hasAshe, bool hasWarning, bool hasBoom);
    bool isActive() const;
    void deactivate();
    RECT getRect() const;
    float getX() const;
    float getY() const;
    int getRadius() const;
    SkillType getType() const;
    bool canCauseDamage() const;
    bool alreadyDamaged() const;
    void markDamaged();

private:
    void setDirection(float targetX, float targetY);
};
