#include "Skill.h"

#include <cstdlib>
#include <cmath>

const int SKILL_WINDOW_WIDTH = 960;
const int SKILL_WINDOW_HEIGHT = 540;

// 画带透明效果的 PNG。技能图片旋转后也用它画，尽量避免黑底。
void skillDrawPng(int drawX, int drawY, IMAGE* image)
{
    DWORD* screenBuffer = GetImageBuffer();
    DWORD* imageBuffer = GetImageBuffer(image);

    int screenWidth = getwidth();
    int screenHeight = getheight();
    int imageWidth = image->getwidth();
    int imageHeight = image->getheight();

    DWORD cornerColor[4];
    cornerColor[0] = imageBuffer[0];
    cornerColor[1] = imageBuffer[imageWidth - 1];
    cornerColor[2] = imageBuffer[(imageHeight - 1) * imageWidth];
    cornerColor[3] = imageBuffer[(imageHeight - 1) * imageWidth + imageWidth - 1];

    bool hasAlpha = false;
    for (int i = 0; i < imageWidth * imageHeight; i++)
    {
        int alpha = (imageBuffer[i] >> 24) & 0xff;
        if (alpha > 0)
        {
            hasAlpha = true;
            break;
        }
    }

    for (int y = 0; y < imageHeight; y++)
    {
        int screenY = drawY + y;
        if (screenY < 0 || screenY >= screenHeight)
        {
            continue;
        }

        for (int x = 0; x < imageWidth; x++)
        {
            int screenX = drawX + x;
            if (screenX < 0 || screenX >= screenWidth)
            {
                continue;
            }

            DWORD sourceColor = imageBuffer[y * imageWidth + x];
            int alpha = (sourceColor >> 24) & 0xff;
            int sourceRed = sourceColor & 0xff;
            int sourceGreen = (sourceColor >> 8) & 0xff;
            int sourceBlue = (sourceColor >> 16) & 0xff;

            bool isNearWhite = sourceRed > 225 && sourceGreen > 225 && sourceBlue > 225;
            bool isNearBlack = sourceRed < 35 && sourceGreen < 35 && sourceBlue < 35;
            bool isNearCornerColor = false;

            for (int i = 0; i < 4; i++)
            {
                int cornerRed = cornerColor[i] & 0xff;
                int cornerGreen = (cornerColor[i] >> 8) & 0xff;
                int cornerBlue = (cornerColor[i] >> 16) & 0xff;

                int colorDistance =
                    abs(sourceRed - cornerRed) +
                    abs(sourceGreen - cornerGreen) +
                    abs(sourceBlue - cornerBlue);

                if (colorDistance < 80)
                {
                    isNearCornerColor = true;
                    break;
                }
            }

            // 白底、黑边、以及和四个角接近的背景色都不画。
            if (isNearWhite || isNearBlack || isNearCornerColor)
            {
                continue;
            }

            if (hasAlpha && alpha == 0)
            {
                continue;
            }
            if (!hasAlpha)
            {
                alpha = 255;
            }

            DWORD oldColor = screenBuffer[screenY * screenWidth + screenX];

            int oldRed = oldColor & 0xff;
            int oldGreen = (oldColor >> 8) & 0xff;
            int oldBlue = (oldColor >> 16) & 0xff;

            int newRed = (sourceRed * alpha + oldRed * (255 - alpha)) / 255;
            int newGreen = (sourceGreen * alpha + oldGreen * (255 - alpha)) / 255;
            int newBlue = (sourceBlue * alpha + oldBlue * (255 - alpha)) / 255;

            screenBuffer[screenY * screenWidth + screenX] = RGB(newRed, newGreen, newBlue);
        }
    }
}

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

// 计算技能朝哪个方向飞。
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

// 每帧更新技能位置。
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

// 画技能。有图片就画图片，没有图片就画原来的图形占位技能。
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
                skillDrawPng(drawX, drawY, warningImg);
            }
            else
            {
                setlinecolor(RGB(255, 70, 70));
                setlinestyle(PS_SOLID, 3);
                circle((int)x, (int)y, radius);
                setlinestyle(PS_DOT, 1);
                circle((int)x, (int)y, radius - 12);
                setlinestyle(PS_SOLID, 1);
            }
        }
        else
        {
            if (hasBoom)
            {
                skillDrawPng(drawX, drawY, boomImg);
            }
            else
            {
                setfillcolor(RGB(255, 118, 45));
                solidcircle((int)x, (int)y, radius);
                setfillcolor(RGB(255, 220, 80));
                solidcircle((int)x, (int)y, radius / 2);
                setlinecolor(RGB(255, 55, 35));
                setlinestyle(PS_SOLID, 4);
                circle((int)x, (int)y, radius);
                setlinestyle(PS_SOLID, 1);
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

    if (isImageLoaded)
    {
        IMAGE rotatedImage;
        rotateimage(&rotatedImage, skillImage, angle, BLACK, true, true);

        int drawWidth = rotatedImage.getwidth();
        int drawHeight = rotatedImage.getheight();
        int drawX = (int)x - drawWidth / 2;
        int drawY = (int)y - drawHeight / 2;

        skillDrawPng(drawX, drawY, &rotatedImage);
        return;
    }

    float cosValue = cos(angle);
    float sinValue = sin(angle);
    int x1 = (int)(x - cosValue * width * 0.5f);
    int y1 = (int)(y - sinValue * width * 0.5f);
    int x2 = (int)(x + cosValue * width * 0.5f);
    int y2 = (int)(y + sinValue * width * 0.5f);

    if (type == EZ_Q)
    {
        setlinecolor(RGB(80, 180, 255));
        setlinestyle(PS_SOLID, 10);
        line(x1, y1, x2, y2);
        setlinecolor(RGB(210, 245, 255));
        setlinestyle(PS_SOLID, 4);
        line(x1, y1, x2, y2);
        setfillcolor(RGB(120, 210, 255));
        solidcircle(x2, y2, 8);
    }
    else
    {
        setlinecolor(RGB(150, 235, 255));
        setlinestyle(PS_SOLID, 14);
        line(x1, y1, x2, y2);
        setlinecolor(RGB(235, 255, 255));
        setlinestyle(PS_SOLID, 5);
        line(x1, y1, x2, y2);

        POINT arrow[3] = {
            {x2, y2},
            {(LONG)(x2 - cosValue * 24 - sinValue * 14), (LONG)(y2 - sinValue * 24 + cosValue * 14)},
            {(LONG)(x2 - cosValue * 24 + sinValue * 14), (LONG)(y2 - sinValue * 24 - cosValue * 14)}
        };
        setfillcolor(RGB(180, 245, 255));
        solidpolygon(arrow, 3);
    }

    setlinestyle(PS_SOLID, 1);
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
