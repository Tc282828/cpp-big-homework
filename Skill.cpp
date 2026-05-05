#include "Skill.h"

#include <cmath>

const int SKILL_WINDOW_WIDTH = 960;
const int SKILL_WINDOW_HEIGHT = 540;
const double PI = 3.14159265358979323846;

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
    float distanceX = targetX - x;
    float distanceY = targetY - y;
    float length = sqrt(distanceX * distanceX + distanceY * distanceY);

    if (length < 0.001f)
    {
        length = 1.0f;
    }

    vx = distanceX / length * speed;
    vy = distanceY / length * speed;
    angle = atan2(vy, vx);
}

Skill Skill::createEzQ(float startX, float startY, float targetX, float targetY)
{
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
    timer++;

    if (type == LUX_E)
    {
        if (timer > warningTime + boomTime)
        {
            active = false;
        }
        return;
    }

    x += vx;
    y += vy;

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
        skillImage = asheImg;
        isImageLoaded = hasAshe;
    }

    if (!isImageLoaded)
    {
        return;
    }

    double drawAngle = angle;
    if (type == EZ_Q)
    {
        drawAngle += PI;
    }

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
    if (!active || hasDamaged)
    {
        return false;
    }

    if (type == LUX_E)
    {
        return timer > warningTime && timer <= warningTime + boomTime;
    }

    return true;
}

bool Skill::alreadyDamaged() const { return hasDamaged; }
void Skill::markDamaged() { hasDamaged = true; }
