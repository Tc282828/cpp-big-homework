#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <graphics.h>

// 敌方技能类型
enum SkillType
{
    EZ_Q,       // 伊泽瑞尔 Q，直线飞行
    ASHE_R,     // 寒冰大招，直线飞行
    LUX_E       // 光辉 E，先预警再爆炸
};

// Skill 类保存一个敌方技能的位置、速度、阶段和绘制逻辑
class Skill
{
public:
    // 技能类型
    SkillType type;

    // 技能中心点坐标
    float x;
    float y;

    // 技能每帧移动的速度分量
    float vx;
    float vy;
    float speed;

    // 绘制尺寸和碰撞半径
    int width;
    int height;
    int radius;

    // timer 记录技能存在时间，Lux E 用 warningTime 和 boomTime 分阶段
    int timer;
    int warningTime;
    int boomTime;

    // active 表示是否还在场上，hasDamaged 表示是否已经造成过伤害
    bool active;
    bool hasDamaged;

    // 图片旋转角度，飞行技能出生时指向玩家
    double angle;

    Skill();

    // 创建不同类型的技能
    static Skill createEzQ(float startX, float startY, float targetX, float targetY);
    static Skill createAsheR(float startX, float startY, float targetX, float targetY);
    static Skill createLuxE(float x, float y);

    // 更新、绘制和状态控制
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
    // 根据目标点计算飞行方向和旋转角度
    void setDirection(float targetX, float targetY);
};
