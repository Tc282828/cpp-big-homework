#include "Player.h"

#include <windows.h>

const int PLAYER_WINDOW_WIDTH = 960;
const int PLAYER_WINDOW_HEIGHT = 540;
const int PLAYER_DRAW_SIZE = 80;

// 画带透明效果的 PNG，防止透明背景变成黑色。
void playerDrawPng(int drawX, int drawY, IMAGE* image)
{
    DWORD* screenBuffer = GetImageBuffer();
    DWORD* imageBuffer = GetImageBuffer(image);

    int screenWidth = getwidth();
    int screenHeight = getheight();
    int imageWidth = image->getwidth();
    int imageHeight = image->getheight();

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

            // 这几张角色素材是白底图，把接近白色的地方跳过。
            if (sourceRed > 240 && sourceGreen > 240 && sourceBlue > 240)
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

Player::Player()
{
    reset();
}

// 重置玩家数据，开局和重开都会用到。
void Player::reset()
{
    x = 180.0f;
    y = 300.0f;
    speed = 5.0f;
    radius = 32;
    hp = 3;
    invincibleTimer = 0;
    hurtState = false;
}

// 读取键盘，WASD 和方向键都可以移动。
void Player::handleInput()
{
    float moveX = 0.0f;
    float moveY = 0.0f;

    if ((GetAsyncKeyState('W') & 0x8000) || (GetAsyncKeyState(VK_UP) & 0x8000))
    {
        moveY -= speed;
    }
    if ((GetAsyncKeyState('S') & 0x8000) || (GetAsyncKeyState(VK_DOWN) & 0x8000))
    {
        moveY += speed;
    }
    if ((GetAsyncKeyState('A') & 0x8000) || (GetAsyncKeyState(VK_LEFT) & 0x8000))
    {
        moveX -= speed;
    }
    if ((GetAsyncKeyState('D') & 0x8000) || (GetAsyncKeyState(VK_RIGHT) & 0x8000))
    {
        moveX += speed;
    }

    x += moveX;
    y += moveY;

    if (x < radius) x = (float)radius;
    if (x > PLAYER_WINDOW_WIDTH - radius) x = (float)(PLAYER_WINDOW_WIDTH - radius);
    if (y < radius) y = (float)radius;
    if (y > PLAYER_WINDOW_HEIGHT - radius) y = (float)(PLAYER_WINDOW_HEIGHT - radius);
}

// 每帧更新受伤后的无敌时间。
void Player::update()
{
    if (invincibleTimer > 0)
    {
        invincibleTimer--;
    }

    hurtState = invincibleTimer > 0;
}

// 画玩家。优先显示图片，没有图片就画原来的占位小人。
void Player::draw(IMAGE* idleImg, IMAGE* hurtImg, bool hasIdle, bool hasHurt)
{
    if (isInvincible() && (invincibleTimer / 6) % 2 == 0)
    {
        return;
    }

    IMAGE* currentImage = idleImg;
    bool isImageLoaded = hasIdle;

    if (hurtState && hasHurt)
    {
        currentImage = hurtImg;
        isImageLoaded = true;
    }

    int drawX = (int)x - PLAYER_DRAW_SIZE / 2;
    int drawY = (int)y - PLAYER_DRAW_SIZE / 2;

    if (isImageLoaded)
    {
        playerDrawPng(drawX, drawY, currentImage);
        return;
    }

    setlinecolor(RGB(30, 55, 80));
    setfillcolor(RGB(145, 220, 255));
    solidcircle((int)x, (int)y, 28);

    setfillcolor(RGB(80, 165, 235));
    solidellipse((int)x - 30, (int)y - 36, (int)x + 30, (int)y - 4);

    setfillcolor(RGB(255, 228, 210));
    solidcircle((int)x, (int)y - 8, 18);

    setfillcolor(RGB(30, 60, 85));
    solidcircle((int)x - 6, (int)y - 10, 2);
    solidcircle((int)x + 7, (int)y - 10, 2);

    setlinecolor(RGB(210, 235, 255));
    setlinestyle(PS_SOLID, 4);
    line((int)x - 35, (int)y + 22, (int)x + 30, (int)y - 2);
    line((int)x - 22, (int)y + 18, (int)x + 38, (int)y + 24);
    setlinestyle(PS_SOLID, 1);

}

// 玩家受伤时扣 1 点血，然后短暂无敌。
void Player::hurt()
{
    if (hp <= 0 || isInvincible())
    {
        return;
    }

    hp--;
    invincibleTimer = 60;
    hurtState = true;
}

float Player::getX() const { return x; }
float Player::getY() const { return y; }
int Player::getRadius() const { return radius; }
int Player::getHp() const { return hp; }
bool Player::isInvincible() const { return invincibleTimer > 0; }
