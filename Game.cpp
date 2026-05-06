#include "Game.h"
#include "Collision.h"

#include <cmath>
#include <cstdlib>
#include <ctime>
#include <mmsystem.h>
#include <tchar.h>
#include <vector>
#include <windows.h>

#pragma comment(lib, "winmm.lib")

const int WIN_SURVIVE_FRAME = 60 * 60;
const int W_MIST_DRAW_SIZE = 220;

// 检查音效文件存在后再播放，避免找不到文件时播放系统提示音
bool playSoundIfExists(LPCTSTR soundPath)
{
    DWORD fileInfo = GetFileAttributes(soundPath);
    if (fileInfo == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }
    if (fileInfo & FILE_ATTRIBUTE_DIRECTORY)
    {
        return false;
    }

    // SND_ASYNC 表示异步播放，游戏画面不会因为音效卡住
    PlaySound(soundPath, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
    return true;
}

void playSoundEffect(const TCHAR* soundPath)
{
    // 先按代码里写的相对路径找
    if (playSoundIfExists(soundPath))
    {
        return;
    }

    TCHAR currentPath[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentPath);

    // 有时 VS 的运行目录在项目上一级，所以多尝试一次 GwenDodgeGame 子目录
    TCHAR tryPath[MAX_PATH];
    _stprintf_s(tryPath, _T("%s\\GwenDodgeGame\\%s"), currentPath, soundPath);
    if (playSoundIfExists(tryPath))
    {
        return;
    }

    TCHAR exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);

    // 找到 exe 所在文件夹，方便从输出目录反推 assets 目录
    int lastSlash = -1;
    for (int i = 0; exePath[i] != _T('\0'); i++)
    {
        if (exePath[i] == _T('\\') || exePath[i] == _T('/'))
        {
            lastSlash = i;
        }
    }

    if (lastSlash >= 0)
    {
        exePath[lastSlash] = _T('\0');

        // 尝试 exe 同级目录
        _stprintf_s(tryPath, _T("%s\\%s"), exePath, soundPath);
        if (playSoundIfExists(tryPath))
        {
            return;
        }

        // 尝试 exe 上两级目录，适配 bin/Debug 这种输出路径
        _stprintf_s(tryPath, _T("%s\\..\\..\\%s"), exePath, soundPath);
        if (playSoundIfExists(tryPath))
        {
            return;
        }

        // 尝试上两级后再进入项目目录
        _stprintf_s(tryPath, _T("%s\\..\\..\\GwenDodgeGame\\%s"), exePath, soundPath);
        playSoundIfExists(tryPath);
    }
}

// 判断文件是否存在，图片和音效加载都会用到类似思路
bool gameFileExists(LPCTSTR fileName)
{
    DWORD fileInfo = GetFileAttributes(fileName);
    if (fileInfo == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }
    if (fileInfo & FILE_ATTRIBUTE_DIRECTORY)
    {
        return false;
    }
    return true;
}

bool gameLoadImage(IMAGE* image, LPCTSTR fileName, int imageWidth, int imageHeight)
{
    // 第一种情况：直接从当前运行目录找 assets
    if (gameFileExists(fileName))
    {
        loadimage(image, fileName, imageWidth, imageHeight, true);
        return true;
    }

    TCHAR currentPath[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentPath);

    // 第二种情况：当前目录下面还有一个项目文件夹
    TCHAR tryPath[MAX_PATH];
    _stprintf_s(tryPath, _T("%s\\GwenDodgeGame\\%s"), currentPath, fileName);
    if (gameFileExists(tryPath))
    {
        loadimage(image, tryPath, imageWidth, imageHeight, true);
        return true;
    }

    TCHAR exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);

    // 第三种情况：从 exe 所在目录往回找资源
    int lastSlash = -1;
    for (int i = 0; exePath[i] != _T('\0'); i++)
    {
        if (exePath[i] == _T('\\') || exePath[i] == _T('/'))
        {
            lastSlash = i;
        }
    }

    if (lastSlash >= 0)
    {
        exePath[lastSlash] = _T('\0');

        // 尝试 exe 同级目录
        _stprintf_s(tryPath, _T("%s\\%s"), exePath, fileName);
        if (gameFileExists(tryPath))
        {
            loadimage(image, tryPath, imageWidth, imageHeight, true);
            return true;
        }

        // 尝试 exe 上两级目录
        _stprintf_s(tryPath, _T("%s\\..\\..\\%s"), exePath, fileName);
        if (gameFileExists(tryPath))
        {
            loadimage(image, tryPath, imageWidth, imageHeight, true);
            return true;
        }

        // 尝试 exe 上两级后再进入项目目录
        _stprintf_s(tryPath, _T("%s\\..\\..\\GwenDodgeGame\\%s"), exePath, fileName);
        if (gameFileExists(tryPath))
        {
            loadimage(image, tryPath, imageWidth, imageHeight, true);
            return true;
        }
    }

    return false;
}

int gameRandom(int minValue, int maxValue)
{
    // 生成 [minValue, maxValue] 之间的随机整数
    return minValue + rand() % (maxValue - minValue + 1);
}

bool isPngBackColor(DWORD color, DWORD cornerColor[4], bool cleanRotateBack, bool useWhiteBack)
{
    // EasyX 的颜色顺序这里按 BGR 拆出 RGB 分量
    int red = color & 0xff;
    int green = (color >> 8) & 0xff;
    int blue = (color >> 16) & 0xff;
    int maxColor = red;
    int minColor = red;

    if (green > maxColor) maxColor = green;
    if (blue > maxColor) maxColor = blue;
    if (green < minColor) minColor = green;
    if (blue < minColor) minColor = blue;

    bool isGrayLike = maxColor - minColor < 35;

    // 旋转图片时 EasyX 可能补黑边，这里把黑边当成透明背景
    if (cleanRotateBack && red < 8 && green < 8 && blue < 8)
    {
        return true;
    }

    // 旋转后的假透明背景有时会留下深灰色虚线边，也一起当背景去掉
    if (cleanRotateBack && isGrayLike && red < 170 && green < 170 && blue < 170)
    {
        return true;
    }

    // 有些素材是假透明白底，接近白色的灰白背景要去掉
    if (useWhiteBack && isGrayLike && red > 185 && green > 185 && blue > 185)
    {
        return true;
    }

    if (useWhiteBack)
    {
        // 再用四个角的颜色判断棋盘格或灰白背景
        for (int i = 0; i < 4; i++)
        {
            int cornerRed = cornerColor[i] & 0xff;
            int cornerGreen = (cornerColor[i] >> 8) & 0xff;
            int cornerBlue = (cornerColor[i] >> 16) & 0xff;
            int cornerMax = cornerRed;
            int cornerMin = cornerRed;

            if (cornerGreen > cornerMax) cornerMax = cornerGreen;
            if (cornerBlue > cornerMax) cornerMax = cornerBlue;
            if (cornerGreen < cornerMin) cornerMin = cornerGreen;
            if (cornerBlue < cornerMin) cornerMin = cornerBlue;

            bool cornerIsGrayLike = cornerMax - cornerMin < 35;
            int colorDistance =
                abs(red - cornerRed) +
                abs(green - cornerGreen) +
                abs(blue - cornerBlue);

            if (cornerIsGrayLike && isGrayLike && colorDistance < 90)
            {
                return true;
            }
        }
    }

    return false;
}

void markPngBackPixel(int pixelIndex, DWORD* imageBuffer, DWORD cornerColor[4],
    bool cleanRotateBack, bool useWhiteBack, std::vector<char>* skipPixel, std::vector<int>* queue)
{
    // 已经标记过的像素不用重复处理
    if ((*skipPixel)[pixelIndex])
    {
        return;
    }

    if (isPngBackColor(imageBuffer[pixelIndex], cornerColor, cleanRotateBack, useWhiteBack))
    {
        // 是背景色就标记跳过，并加入队列继续向周围扩散
        (*skipPixel)[pixelIndex] = 1;
        queue->push_back(pixelIndex);
    }
}

void drawPngAlpha(int drawX, int drawY, IMAGE* image, bool cleanRotateBack)
{
    // 直接操作 EasyX 的图像缓冲区，自己处理 PNG 透明效果
    DWORD* screenBuffer = GetImageBuffer();
    DWORD* imageBuffer = GetImageBuffer(image);

    int screenWidth = getwidth();
    int screenHeight = getheight();
    int imageWidth = image->getwidth();
    int imageHeight = image->getheight();

    DWORD cornerColor[4];
    // 记录四个角颜色，方便判断白底或棋盘格背景
    cornerColor[0] = imageBuffer[0];
    cornerColor[1] = imageBuffer[imageWidth - 1];
    cornerColor[2] = imageBuffer[(imageHeight - 1) * imageWidth];
    cornerColor[3] = imageBuffer[(imageHeight - 1) * imageWidth + imageWidth - 1];

    bool hasAlphaValue = false;
    bool hasNotFullAlpha = false;
    // 检查图片是否真的带 alpha 通道
    for (int i = 0; i < imageWidth * imageHeight; i++)
    {
        int alpha = (imageBuffer[i] >> 24) & 0xff;
        if (alpha > 0)
        {
            hasAlphaValue = true;
        }
        if (alpha < 255)
        {
            hasNotFullAlpha = true;
        }
    }

    bool useAlpha = hasAlphaValue && hasNotFullAlpha;
    bool cleanWhiteBack = !useAlpha || cleanRotateBack;
    int pixelCount = imageWidth * imageHeight;
    std::vector<char> skipPixel(pixelCount, 0);

    if (!useAlpha || cleanRotateBack)
    {
        // 从图片边缘开始找背景，避免把角色或技能主体误删
        std::vector<int> queue;

        for (int x = 0; x < imageWidth; x++)
        {
            markPngBackPixel(x, imageBuffer, cornerColor, cleanRotateBack, cleanWhiteBack, &skipPixel, &queue);
            markPngBackPixel((imageHeight - 1) * imageWidth + x, imageBuffer, cornerColor,
                cleanRotateBack, cleanWhiteBack, &skipPixel, &queue);
        }

        for (int y = 0; y < imageHeight; y++)
        {
            markPngBackPixel(y * imageWidth, imageBuffer, cornerColor, cleanRotateBack, cleanWhiteBack, &skipPixel, &queue);
            markPngBackPixel(y * imageWidth + imageWidth - 1, imageBuffer, cornerColor,
                cleanRotateBack, cleanWhiteBack, &skipPixel, &queue);
        }

        int head = 0;
        while (head < (int)queue.size())
        {
            // 队列里保存已经确认是背景的像素，再检查它上下左右的像素
            int pixelIndex = queue[head];
            head++;

            int x = pixelIndex % imageWidth;
            int y = pixelIndex / imageWidth;

            if (x > 0)
            {
                markPngBackPixel(pixelIndex - 1, imageBuffer, cornerColor,
                    cleanRotateBack, cleanWhiteBack, &skipPixel, &queue);
            }
            if (x < imageWidth - 1)
            {
                markPngBackPixel(pixelIndex + 1, imageBuffer, cornerColor,
                    cleanRotateBack, cleanWhiteBack, &skipPixel, &queue);
            }
            if (y > 0)
            {
                markPngBackPixel(pixelIndex - imageWidth, imageBuffer, cornerColor,
                    cleanRotateBack, cleanWhiteBack, &skipPixel, &queue);
            }
            if (y < imageHeight - 1)
            {
                markPngBackPixel(pixelIndex + imageWidth, imageBuffer, cornerColor,
                    cleanRotateBack, cleanWhiteBack, &skipPixel, &queue);
            }
        }
    }

    for (int y = 0; y < imageHeight; y++)
    {
        // 超出窗口范围的像素直接跳过
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

            int imageIndex = y * imageWidth + x;
            if (skipPixel[imageIndex])
            {
                // 被标记为背景的像素不画
                continue;
            }

            DWORD sourceColor = imageBuffer[imageIndex];
            int alpha = (sourceColor >> 24) & 0xff;
            int sourceRed = sourceColor & 0xff;
            int sourceGreen = (sourceColor >> 8) & 0xff;
            int sourceBlue = (sourceColor >> 16) & 0xff;

            if (useAlpha && alpha == 0)
            {
                // 完全透明的像素不画
                continue;
            }

            if (!useAlpha)
            {
                alpha = 255;
            }

            DWORD oldColor = screenBuffer[screenY * screenWidth + screenX];
            int oldRed = oldColor & 0xff;
            int oldGreen = (oldColor >> 8) & 0xff;
            int oldBlue = (oldColor >> 16) & 0xff;

            // 按 alpha 把图片颜色和屏幕原颜色混合
            int newRed = (sourceRed * alpha + oldRed * (255 - alpha)) / 255;
            int newGreen = (sourceGreen * alpha + oldGreen * (255 - alpha)) / 255;
            int newBlue = (sourceBlue * alpha + oldBlue * (255 - alpha)) / 255;

            screenBuffer[screenY * screenWidth + screenX] = RGB(newRed, newGreen, newBlue);
        }
    }
}

Game::Game()
{
    // 构造函数只设置初始值，真正创建窗口和加载图片在 init 里做
    width = 960;
    height = 540;
    state = MENU;
    frameCount = 0;
    surviveFrame = 0;
    score = 0;
    spawnTimer = 0;
    spawnInterval = 90;
    running = true;
    resourcesReady = false;
    hasPlayedGameOverSound = false;

    hasBg = false;
    hasCover = false;
    hasEnd = false;
    hasGwenIdle = false;
    hasGwenHurt = false;
    hasEzQ = false;
    hasAsheR = false;
    hasLuxWarning = false;
    hasLuxBoom = false;
    hasHeart = false;
    hasWMist = false;
}

void Game::run()
{
    // 初始化窗口和资源
    init();
    BeginBatchDraw();

    while (running)
    {
        // 每一帧按“输入 -> 更新 -> 绘制”的顺序执行
        handleInput();
        update();
        draw();
        FlushBatchDraw();
        Sleep(16);
    }

    EndBatchDraw();
    // 游戏退出时关闭 EasyX 窗口
    closegraph();
}

void Game::init()
{
    // 设置随机种子，技能生成位置会用到随机数
    srand((unsigned)time(nullptr));
    initgraph(width, height);
    SetWindowText(GetHWnd(), _T("格温小姐大冒险"));
    loadResources();
}

void Game::loadResources()
{
    // 加载菜单、结束页和游戏地图
    hasCover = gameLoadImage(&imgCover, _T("assets/cover.png"), 960, 540);
    hasEnd = gameLoadImage(&imgEnd, _T("assets/coverend.png"), 960, 540);
    hasBg = gameLoadImage(&imgBg, _T("assets/map2.png"), 960, 540);

    // 加载玩家正常和受击图片，中文文件名要和 assets 里完全一致
    hasGwenIdle = gameLoadImage(&imgGwenIdle, _T("assets/格温状态图.png"), 80, 80);
    hasGwenHurt = gameLoadImage(&imgGwenHurt, _T("assets/格温受击图.png"), 80, 80);

    // 加载技能、血量和 J 防御迷雾图片
    hasEzQ = gameLoadImage(&imgEzQ, _T("assets/ezq.png"), 80, 32);
    hasAsheR = gameLoadImage(&imgAsheR, _T("assets/aceyr.png"), 140, 52);
    hasLuxWarning = gameLoadImage(&imgLuxWarning, _T("assets/Lux1.png"), 120, 120);
    hasLuxBoom = gameLoadImage(&imgLuxBoom, _T("assets/Lux2.png"), 120, 120);
    hasHeart = gameLoadImage(&imgHeart, _T("assets/hp.png"), 28, 28);
    hasWMist = gameLoadImage(&imgWMist, _T("assets/w_mist.png"), 220, 220);

    // 只要有一张正式素材缺失，就显示资源错误提示
    resourcesReady = hasCover && hasEnd && hasBg && hasGwenIdle && hasGwenHurt
        && hasEzQ && hasAsheR && hasLuxWarning && hasLuxBoom && hasHeart && hasWMist;
}

void Game::resetGame()
{
    // 重新开始时清空技能和计时，把玩家恢复初始状态
    player.reset();
    skills.clear();
    frameCount = 0;
    surviveFrame = 0;
    score = 0;
    spawnTimer = 0;
    spawnInterval = 90;
    hasPlayedGameOverSound = false;
    state = PLAYING;
}

void Game::handleInput()
{
    // ESC 退出游戏
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
    {
        running = false;
        return;
    }

    if (state == MENU)
    {
        if (GetAsyncKeyState(VK_RETURN) & 0x8000)
        {
            // 菜单按 Enter 开始游戏，并播放开始音效
            playSoundEffect(_T("assets/sounds/我觉得ok.wav"));
            resetGame();
            Sleep(120);
        }
        return;
    }

    if (state == WIN || state == GAME_OVER)
    {
        if (GetAsyncKeyState('R') & 0x8000)
        {
            // 胜利或失败界面按 R 重新开始
            resetGame();
            Sleep(120);
        }
        return;
    }

    if (state == PLAYING)
    {
        // 处理鼠标消息，右键点击地图时设置玩家移动目标
        while (MouseHit())
        {
            MOUSEMSG mouseMessage = GetMouseMsg();
            if (mouseMessage.uMsg == WM_RBUTTONDOWN)
            {
                player.setMoveTarget((float)mouseMessage.x, (float)mouseMessage.y);
            }
        }

        // 游戏中把移动和 J 防御交给 Player 处理
        player.handleInput();
    }
}

void Game::update()
{
    // 只有 PLAYING 状态才更新游戏逻辑
    if (state != PLAYING)
    {
        return;
    }

    frameCount++;
    surviveFrame++;

    // 根据存活时间提高难度，越到后期技能出现越快
    if (surviveFrame < 20 * 60)
    {
        spawnInterval = 90;
    }
    else if (surviveFrame < 40 * 60)
    {
        spawnInterval = 60;
    }
    else
    {
        spawnInterval = 35;
    }

    if (surviveFrame % 60 == 0)
    {
        // 每存活 1 秒加 10 分
        score += 10;
    }

    spawnTimer++;
    if (spawnTimer >= spawnInterval)
    {
        // 到达生成间隔后创建一个新技能
        spawnSkill();
        spawnTimer = 0;
    }

    player.update();

    // 更新所有技能，并检查技能是否打中玩家
    for (int i = 0; i < (int)skills.size(); i++)
    {
        skills[i].update();

        if (!skills[i].canCauseDamage())
        {
            // 预警阶段或已经伤害过的技能不再判断伤害
            continue;
        }

        bool isHit = false;
        if (skills[i].getType() == LUX_E)
        {
            // Lux E 是圆形爆炸范围
            isHit = circleCollide(skills[i].getX(), skills[i].getY(), (float)skills[i].getRadius(),
                player.getX(), player.getY(), (float)player.getRadius());
        }
        else
        {
            // 飞行技能按矩形和玩家圆形做碰撞
            isHit = rectCircleCollide(skills[i].getRect(), player.getX(), player.getY(), (float)player.getRadius());
        }

        if (isHit)
        {
            if (player.isUsingDefense())
            {
                // J 防御期间不扣血，技能本身继续正常飞行
                skills[i].markDamaged();
            }
            else if (!player.isInvincible())
            {
                // 不在防御和无敌时才真正扣血
                int oldHp = player.getHp();
                player.hurt();
                if (player.getHp() < oldHp)
                {
                    // 确认血量真的减少后才播放受击音效
                    playSoundEffect(_T("assets/sounds/啊！！惨叫.wav"));
                }
                skills[i].markDamaged();
                if (skills[i].getType() != LUX_E)
                {
                    // 飞行技能打中玩家后消失，Lux E 爆炸按时间自然结束
                    skills[i].deactivate();
                }
            }
            else
            {
                // 玩家无敌时不扣血，但这个技能不再重复造成伤害
                skills[i].markDamaged();
            }
        }
    }

    // 删除已经失效的技能，没打中过玩家的技能离场时给一点分数
    for (int i = 0; i < (int)skills.size(); i++)
    {
        if (!skills[i].isActive())
        {
            if (!skills[i].alreadyDamaged())
            {
                score += 5;
            }
            skills.erase(skills.begin() + i);
            i--;
        }
    }

    // 血量为 0 才算失败，失败时播放一次死亡音效
    if (player.getHp() <= 0)
    {
        if (!hasPlayedGameOverSound)
        {
            playSoundEffect(_T("assets/sounds/超级玛丽死亡音效.wav"));
            hasPlayedGameOverSound = true;
        }
        state = GAME_OVER;
    }
    // 玩家活到 60 秒算胜利，正常胜利不播放死亡音效
    else if (surviveFrame >= WIN_SURVIVE_FRAME)
    {
        state = WIN;
    }
}

void Game::draw()
{
    // 每帧先清屏，再按当前状态绘制对应界面
    cleardevice();

    if (!resourcesReady)
    {
        // 正式素材缺失时只显示错误提示，不再画占位图
        drawResourceError();
        return;
    }

    if (state == MENU)
    {
        drawMenu();
    }
    else if (state == PLAYING)
    {
        drawPlaying();
    }
    else if (state == WIN)
    {
        drawWin();
    }
    else if (state == GAME_OVER)
    {
        drawGameOver();
    }
}

void Game::getRandomEdgePoint(float* startX, float* startY)
{
    // edge 表示从窗口哪一条边生成技能
    int edge = gameRandom(0, 3);

    if (edge == 0)
    {
        // 左边生成
        *startX = -100.0f;
        *startY = (float)gameRandom(0, height);
    }
    else if (edge == 1)
    {
        // 右边生成
        *startX = (float)(width + 100);
        *startY = (float)gameRandom(0, height);
    }
    else if (edge == 2)
    {
        // 上边生成
        *startX = (float)gameRandom(0, width);
        *startY = -100.0f;
    }
    else
    {
        // 下边生成
        *startX = (float)gameRandom(0, width);
        *startY = (float)(height + 100);
    }
}

void Game::getLineSkillPoint(float* startX, float* startY, float* targetX, float* targetY)
{
    // 生成一条穿过玩家附近的长直线弹道
    double angle = (double)gameRandom(0, 359) * 3.1415926 / 180.0;
    float directionX = (float)cos(angle);
    float directionY = (float)sin(angle);

    // 让技能不是每次都严格穿过玩家中心，而是在玩家附近有一点随机偏移
    float centerX = player.getX() + (float)gameRandom(-80, 80);
    float centerY = player.getY() + (float)gameRandom(-60, 60);

    // 技能删除范围是屏幕外 180 像素，所以出生点不能放得太远
    float left = -160.0f;
    float right = (float)width + 160.0f;
    float top = -160.0f;
    float bottom = (float)height + 160.0f;

    float backDistance = 900.0f;
    float forwardDistance = 900.0f;

    // 算出沿着反方向走多远会碰到扩展后的窗口边界
    if (directionX > 0.001f)
    {
        float distance = (centerX - left) / directionX;
        if (distance < backDistance) backDistance = distance;
    }
    else if (directionX < -0.001f)
    {
        float distance = (right - centerX) / (-directionX);
        if (distance < backDistance) backDistance = distance;
    }

    if (directionY > 0.001f)
    {
        float distance = (centerY - top) / directionY;
        if (distance < backDistance) backDistance = distance;
    }
    else if (directionY < -0.001f)
    {
        float distance = (bottom - centerY) / (-directionY);
        if (distance < backDistance) backDistance = distance;
    }

    // 目标点放到另一侧屏幕外，技能会沿着这个方向一直飞出去
    *startX = centerX - directionX * (backDistance - 5.0f);
    *startY = centerY - directionY * (backDistance - 5.0f);
    *targetX = centerX + directionX * forwardDistance;
    *targetY = centerY + directionY * forwardDistance;
}

void Game::spawnSkill()
{
    // 随机决定本次生成哪一种技能
    int type = gameRandom(0, 99);
    if (type < 40)
    {
        // 40% 概率生成 EZ Q，飞行路线是一条穿过玩家附近的长直线
        float startX = 0.0f;
        float startY = 0.0f;
        float targetX = 0.0f;
        float targetY = 0.0f;
        getLineSkillPoint(&startX, &startY, &targetX, &targetY);
        skills.push_back(Skill::createEzQ(startX, startY, targetX, targetY));
    }
    else if (type < 70)
    {
        // 30% 概率生成寒冰大招，也使用长直线弹道
        float startX = 0.0f;
        float startY = 0.0f;
        float targetX = 0.0f;
        float targetY = 0.0f;
        getLineSkillPoint(&startX, &startY, &targetX, &targetY);
        skills.push_back(Skill::createAsheR(startX, startY, targetX, targetY));
    }
    else
    {
        // 30% 概率生成 Lux E，位置在玩家附近随机偏移
        int offsetX = gameRandom(-160, 160);
        int offsetY = gameRandom(-120, 120);
        float luxX = player.getX() + (float)offsetX;
        float luxY = player.getY() + (float)offsetY;

        // 防止 Lux E 生成到窗口外面
        if (luxX < 80.0f) luxX = 80.0f;
        if (luxX > width - 80.0f) luxX = (float)(width - 80);
        if (luxY < 80.0f) luxY = 80.0f;
        if (luxY > height - 80.0f) luxY = (float)(height - 80);

        skills.push_back(Skill::createLuxE(luxX, luxY));
        // Lux E 预警圈出现时播放提示音效
        playSoundEffect(_T("assets/sounds/lux.wav"));
    }
}

void Game::drawMenu()
{
    // 菜单页直接显示正式封面图，文字已经做到图片里了
    putimage(0, 0, &imgCover);
}

void Game::drawPlaying()
{
    // 先画地图背景
    putimage(0, 0, &imgBg);

    // 再画所有敌方技能
    for (int i = 0; i < (int)skills.size(); i++)
    {
        skills[i].draw(&imgEzQ, &imgAsheR, &imgLuxWarning, &imgLuxBoom,
            hasEzQ, hasAsheR, hasLuxWarning, hasLuxBoom);
    }

    if (player.isUsingDefense())
    {
        // J 防御迷雾画在玩家下面，中心跟随玩家中心
        int mistX = (int)player.getX() - W_MIST_DRAW_SIZE / 2;
        int mistY = (int)player.getY() - W_MIST_DRAW_SIZE / 2;
        drawPngAlpha(mistX, mistY, &imgWMist, false);
    }

    // 最后画玩家和 UI
    player.draw(&imgGwenIdle, &imgGwenHurt, hasGwenIdle, hasGwenHurt);
    drawUI();
}

void Game::drawWin()
{
    // 胜利界面使用结束页图片，只额外显示最终分数
    putimage(0, 0, &imgEnd);

    TCHAR text[64];
    _stprintf_s(text, _T("最终分数：%d"), score);
    setbkmode(TRANSPARENT);
    settextstyle(26, 0, _T("微软雅黑"));
    settextcolor(RGB(255, 255, 255));
    outtextxy(710, 485, text);
}

void Game::drawGameOver()
{
    // 失败界面也使用结束页图片，只额外显示最终分数
    putimage(0, 0, &imgEnd);

    TCHAR text[64];
    _stprintf_s(text, _T("最终分数：%d"), score);
    setbkmode(TRANSPARENT);
    settextstyle(26, 0, _T("微软雅黑"));
    settextcolor(RGB(255, 255, 255));
    outtextxy(710, 485, text);
}

void Game::drawUI()
{
    // 游戏中左上角显示血量、时间、分数、难度和防御状态
    setbkmode(TRANSPARENT);
    settextstyle(22, 0, _T("微软雅黑"));
    settextcolor(RGB(255, 255, 255));

    for (int i = 0; i < player.getHp(); i++)
    {
        // 每一点血画一个爱心图片
        drawPngAlpha(20 + i * 34, 18, &imgHeart, false);
    }

    TCHAR text[128];
    _stprintf_s(text, _T("时间：%d / 60 秒"), surviveFrame / 60);
    outtextxy(20, 62, text);

    _stprintf_s(text, _T("分数：%d"), score);
    outtextxy(20, 92, text);

    const TCHAR* stage = _T("初级");
    if (surviveFrame >= 40 * 60)
    {
        // 40 秒后进入最终阶段
        stage = _T("终局");
    }
    else if (surviveFrame >= 20 * 60)
    {
        // 20 秒后进入中级阶段
        stage = _T("中级");
    }
    _stprintf_s(text, _T("难度阶段：%s"), stage);
    outtextxy(20, 122, text);

    if (player.isUsingDefense())
    {
        // 防御正在持续
        _stprintf_s(text, _T("J防御：持续中"));
    }
    else if (player.getDefenseCooldown() > 0)
    {
        // 防御冷却中，按秒显示剩余时间
        int coolSecond = player.getDefenseCooldown() / 60 + 1;
        _stprintf_s(text, _T("J防御：冷却 %d 秒"), coolSecond);
    }
    else
    {
        // 防御可以释放
        _stprintf_s(text, _T("J防御：可用"));
    }
    outtextxy(20, 152, text);
}

void Game::drawResourceError()
{
    // 图片缺失时显示简单错误提示，不再画临时地图或临时人物
    setbkcolor(RGB(25, 25, 30));
    cleardevice();
    setbkmode(TRANSPARENT);
    settextstyle(28, 0, _T("微软雅黑"));
    settextcolor(RGB(255, 230, 230));
    outtextxy(225, 225, _T("图片资源加载失败，请检查 assets 文件夹"));

    settextstyle(18, 0, _T("微软雅黑"));
    settextcolor(RGB(220, 220, 220));
    outtextxy(255, 270, _T("请确认图片文件名和代码中的路径完全一致"));
}
