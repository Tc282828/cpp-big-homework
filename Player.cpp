#include "Player.h"

#include <windows.h>

const int PLAYER_WINDOW_WIDTH = 960;
const int PLAYER_WINDOW_HEIGHT = 540;
const int PLAYER_DRAW_SIZE = 80;

void drawPngAlpha(int drawX, int drawY, IMAGE* image, bool cleanRotateBack);

Player::Player()
{
    reset();
}

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

void Player::update()
{
    if (invincibleTimer > 0)
    {
        invincibleTimer--;
    }

    hurtState = invincibleTimer > 0;
}

void Player::draw(IMAGE* idleImg, IMAGE* hurtImg, bool hasIdle, bool hasHurt)
{
    if (isInvincible() && (invincibleTimer / 6) % 2 == 0)
    {
        return;
    }

    IMAGE* currentImage = idleImg;
    bool isImageLoaded = hasIdle;

    // 受伤图优先显示，没有受伤图时才用普通图。
    if (hurtState && hasHurt)
    {
        currentImage = hurtImg;
        isImageLoaded = true;
    }

    if (!isImageLoaded)
    {
        return;
    }

    int drawX = (int)x - PLAYER_DRAW_SIZE / 2;
    int drawY = (int)y - PLAYER_DRAW_SIZE / 2;
    drawPngAlpha(drawX, drawY, currentImage, false);
}

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
