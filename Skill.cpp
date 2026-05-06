#include "Skill.h"

#include <cmath>

const int SKILL_WINDOW_WIDTH = 960;
const int SKILL_WINDOW_HEIGHT = 540;
const double PI = 3.14159265358979323846;

// 透明贴图函数在 Game.cpp 里实现，技能这里只负责传图片和位置
void drawPngAlpha(int drawX, int drawY, IMAGE* image, bool cleanRotateBack);

Skill::Skill()
{
    type = EZ_Q;
    x = 0.0f;
    y = 0.0f;
    vx = 0.0f;
    vy = 0.0f;
    speed = 0.0f;
    width = 80;
    height = 32;
    radius = 16;
    timer = 0;
    warningTime = 0;
    boomTime = 0;
    active = false;
    hasDamaged = false;
    angle = 0.0;
}

void Skill::setDirection(float targetX, float targetY)
{
    // 用目标点减去出生点，得到技能应该飞向的方向
    float distanceX = targetX - x;
    float distanceY = targetY - y;
    float length = sqrt(distanceX * distanceX + distanceY * distanceY);

    if (length < 0.001f)
    {
        // 防止距离太小导致除以 0
        length = 1.0f;
    }

    // 把方向变成单位向量，再乘速度，得到每帧移动量
    vx = distanceX / length * speed;
    vy = distanceY / length * speed;

    // 角度只在创建时计算一次，所以飞行中不会一直追踪玩家
    angle = atan2(vy, vx);
}

Skill Skill::createEzQ(float startX, float startY, float targetX, float targetY)
{
    // 创建一个 EZ Q，从边缘出生，朝玩家出生时的位置飞
    Skill skill;
    skill.type = EZ_Q;
    skill.x = startX;
    skill.y = startY;
    skill.speed = 8.0f;
    skill.width = 80;
    skill.height = 32;
    skill.radius = 20;
    skill.active = true;
    skill.setDirection(targetX, targetY);
    return skill;
}

Skill Skill::createAsheR(float startX, float startY, float targetX, float targetY)
{
    // 创建一个寒冰大招，速度比 EZ Q 慢，但碰撞范围更大
    Skill skill;
    skill.type = ASHE_R;
    skill.x = startX;
    skill.y = startY;
    skill.speed = 5.0f;
    skill.width = 140;
    skill.height = 52;
    skill.radius = 30;
    skill.active = true;
    skill.setDirection(targetX, targetY);
    return skill;
}

Skill Skill::createLuxE(float targetX, float targetY)
{
    // 创建一个光辉 E，不移动，先预警 60 帧，再爆炸 20 帧
    Skill skill;
    skill.type = LUX_E;
    skill.x = targetX;
    skill.y = targetY;
    skill.vx = 0.0f;
    skill.vy = 0.0f;
    skill.speed = 0.0f;
    skill.width = 120;
    skill.height = 120;
    skill.radius = 60;
    skill.warningTime = 60;
    skill.boomTime = 20;
    skill.active = true;
    return skill;
}

void Skill::update()
{
    // timer 记录这个技能已经存在了多少帧
    timer++;

    if (type == LUX_E)
    {
        // Lux E 只按时间切换阶段，不需要移动
        if (timer > warningTime + boomTime)
        {
            active = false;
        }
        return;
    }

    x += vx;
    y += vy;

    // 飞出屏幕一段距离后删除，避免技能一直留在数组里
    if (x < -180 || x > SKILL_WINDOW_WIDTH + 180 || y < -180 || y > SKILL_WINDOW_HEIGHT + 180)
    {
        active = false;
    }
}

void Skill::draw(IMAGE* ezImg, IMAGE* asheImg, IMAGE* warningImg, IMAGE* boomImg,
    bool hasEz, bool hasAshe, bool hasWarning, bool hasBoom)
{
    if (!active)
    {
        return;
    }

    if (type == LUX_E)
    {
        // Lux E 前半段画预警圈，后半段画爆炸图
        int drawX = (int)x - width / 2;
        int drawY = (int)y - height / 2;

        if (timer <= warningTime)
        {
            if (hasWarning)
            {
                drawPngAlpha(drawX, drawY, warningImg, false);
            }
        }
        else
        {
            if (hasBoom)
            {
                drawPngAlpha(drawX, drawY, boomImg, false);
            }
        }
        return;
    }

    IMAGE* skillImage = ezImg;
    bool isImageLoaded = hasEz;
    if (type == ASHE_R)
    {
        // 寒冰大招使用自己的图片
        skillImage = asheImg;
        isImageLoaded = hasAshe;
    }

    if (!isImageLoaded)
    {
        // 正式素材没加载成功时不画临时技能
        return;
    }

    double drawAngle = angle;
    if (type == EZ_Q)
    {
        // ezq.png 原图方向和角度方向相反，所以这里补 180 度
        drawAngle += PI;
    }

    // 先把技能图旋转，再按透明方式画到屏幕上
    IMAGE rotatedImage;
    rotateimage(&rotatedImage, skillImage, drawAngle, BLACK, true, true);

    int drawWidth = rotatedImage.getwidth();
    int drawHeight = rotatedImage.getheight();
    int drawX = (int)x - drawWidth / 2;
    int drawY = (int)y - drawHeight / 2;

    drawPngAlpha(drawX, drawY, &rotatedImage, true);
}

bool Skill::isActive() const { return active; }
void Skill::deactivate() { active = false; }

RECT Skill::getRect() const
{
    // 飞行技能用矩形做碰撞，矩形中心就是技能中心点
    RECT rect;
    rect.left = (LONG)(x - width / 2);
    rect.top = (LONG)(y - height / 2);
    rect.right = (LONG)(x + width / 2);
    rect.bottom = (LONG)(y + height / 2);
    return rect;
}

float Skill::getX() const { return x; }
float Skill::getY() const { return y; }
int Skill::getRadius() const { return radius; }
SkillType Skill::getType() const { return type; }

bool Skill::canCauseDamage() const
{
    // 已经消失或已经造成过伤害的技能不能再次伤害玩家
    if (!active || hasDamaged)
    {
        return false;
    }

    if (type == LUX_E)
    {
        // Lux E 只有爆炸阶段能造成伤害，预警阶段不扣血
        return timer > warningTime && timer <= warningTime + boomTime;
    }

    return true;
}

bool Skill::alreadyDamaged() const { return hasDamaged; }
void Skill::markDamaged() { hasDamaged = true; }
