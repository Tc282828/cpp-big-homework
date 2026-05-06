#include "Player.h"

#include <cmath>
#include <tchar.h>
#include <windows.h>

const int PLAYER_WINDOW_WIDTH = 960;
const int PLAYER_WINDOW_HEIGHT = 540;
const int PLAYER_DRAW_SIZE = 80;
const int DEFENSE_KEEP_FRAME = 60;
const int DEFENSE_COOLDOWN_FRAME = 300;

// 这两个函数在 Game.cpp 里实现，Player 这里只负责调用
void drawPngAlpha(int drawX, int drawY, IMAGE* image, bool cleanRotateBack);
void playSoundEffect(const TCHAR* soundPath);

Player::Player()
{
    reset();
}

void Player::reset()
{
    // 玩家初始位置放在地图左侧，血量和计时器全部重置
    x = 180.0f;
    y = 300.0f;
    speed = 5.0f;
    radius = 32;
    hp = 3;
    invincibleTimer = 0;
    defenseTimer = 0;
    defenseCooldown = 0;
    hurtState = false;
    usingDefense = false;
    movingByMouse = false;
    targetX = x;
    targetY = y;
}

void Player::handleInput()
{
    // moveX 和 moveY 先记录本帧移动量，最后统一加到坐标上
    float moveX = 0.0f;
    float moveY = 0.0f;

    // WASD 和方向键都可以控制移动
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
    if (GetAsyncKeyState('J') & 0x8000)
    {
        // J 键释放防御，冷却中 startDefense 会自动不生效
        startDefense();
    }

    if (moveX != 0.0f || moveY != 0.0f)
    {
        // 键盘移动优先，只要按了移动键，就取消右键自动移动
        movingByMouse = false;

        // 根据键盘输入结果移动玩家
        x += moveX;
        y += moveY;
    }
    else
    {
        // 没有按键盘移动时，才执行右键自动移动
        updateMouseMove();
    }

    // 防止玩家走出窗口边界
    if (x < radius) x = (float)radius;
    if (x > PLAYER_WINDOW_WIDTH - radius) x = (float)(PLAYER_WINDOW_WIDTH - radius);
    if (y < radius) y = (float)radius;
    if (y > PLAYER_WINDOW_HEIGHT - radius) y = (float)(PLAYER_WINDOW_HEIGHT - radius);
}

void Player::update()
{
    // 受伤后的无敌时间每帧减少
    if (invincibleTimer > 0)
    {
        invincibleTimer--;
    }

    hurtState = invincibleTimer > 0;

    // 防御持续时间每帧减少，减到 0 就关闭防御状态
    if (defenseTimer > 0)
    {
        defenseTimer--;
    }
    if (defenseTimer <= 0)
    {
        usingDefense = false;
    }
    if (defenseCooldown > 0)
    {
        // 防御冷却时间每帧减少
        defenseCooldown--;
    }
}

void Player::draw(IMAGE* idleImg, IMAGE* hurtImg, bool hasIdle, bool hasHurt)
{
    // 无敌时做闪烁效果，有些帧直接不画玩家
    if (isInvincible() && (invincibleTimer / 6) % 2 == 0)
    {
        return;
    }

    IMAGE* currentImage = idleImg;
    bool isImageLoaded = hasIdle;

    // 受伤图优先显示，没有受伤图时才用普通图
    if (hurtState && hasHurt)
    {
        currentImage = hurtImg;
        isImageLoaded = true;
    }

    if (!isImageLoaded)
    {
        // 正式素材没有加载成功时不画占位图，避免回到第一版临时图
        return;
    }

    // 图片是 80x80，坐标 x/y 是玩家中心点，所以绘制时要减去一半尺寸
    int drawX = (int)x - PLAYER_DRAW_SIZE / 2;
    int drawY = (int)y - PLAYER_DRAW_SIZE / 2;
    drawPngAlpha(drawX, drawY, currentImage, false);
}

void Player::hurt()
{
    // 已经没血或正在无敌时，不重复扣血
    if (hp <= 0 || isInvincible())
    {
        return;
    }

    hp--;
    // 扣血后给 60 帧无敌时间，避免一碰到技能就连续掉血
    invincibleTimer = 60;
    hurtState = true;
}

void Player::startDefense()
{
    // 只有没有冷却、也没有正在防御时，J 防御才能释放成功
    if (defenseCooldown <= 0 && defenseTimer <= 0)
    {
        defenseTimer = DEFENSE_KEEP_FRAME;
        defenseCooldown = DEFENSE_COOLDOWN_FRAME;
        usingDefense = true;
        // 防御释放成功才播放音效，冷却中按 J 不播放
        playSoundEffect(_T("assets/sounds/gwen_W.wav"));
    }
}

void Player::setMoveTarget(float newTargetX, float newTargetY)
{
    // 右键目标点也限制在窗口内，避免角色往窗口外走
    targetX = newTargetX;
    targetY = newTargetY;

    if (targetX < radius) targetX = (float)radius;
    if (targetX > PLAYER_WINDOW_WIDTH - radius) targetX = (float)(PLAYER_WINDOW_WIDTH - radius);
    if (targetY < radius) targetY = (float)radius;
    if (targetY > PLAYER_WINDOW_HEIGHT - radius) targetY = (float)(PLAYER_WINDOW_HEIGHT - radius);

    movingByMouse = true;
}

void Player::updateMouseMove()
{
    if (!movingByMouse)
    {
        return;
    }

    // 计算当前位置到右键目标点的方向
    float distanceX = targetX - x;
    float distanceY = targetY - y;
    float distance = sqrt(distanceX * distanceX + distanceY * distanceY);

    if (distance <= speed)
    {
        // 距离目标点很近时，直接停在目标点
        x = targetX;
        y = targetY;
        movingByMouse = false;
        return;
    }

    // 沿着目标方向移动一小步，速度和键盘移动保持一致
    x += distanceX / distance * speed;
    y += distanceY / distance * speed;
}

// 下面这些 get 函数给 Game 类读取玩家状态，避免在外面乱改数据
float Player::getX() const { return x; }
float Player::getY() const { return y; }
int Player::getRadius() const { return radius; }
int Player::getHp() const { return hp; }
bool Player::isInvincible() const { return invincibleTimer > 0; }
bool Player::isUsingDefense() const { return usingDefense; }
int Player::getDefenseTimer() const { return defenseTimer; }
int Player::getDefenseCooldown() const { return defenseCooldown; }
