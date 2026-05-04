#include "Game.h"
#include "Collision.h"

#include <cstdlib>
#include <ctime>
#include <tchar.h>
#include <windows.h>

const int WIN_SURVIVE_FRAME = 60 * 60;

// 判断图片文件在不在，避免 assets 文件夹不存在时程序出错。
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

// 有图片才加载，没图片就返回 false，后面继续画占位图。
bool gameLoadImage(IMAGE* image, LPCTSTR fileName, int imageWidth, int imageHeight)
{
    if (gameFileExists(fileName))
    {
        loadimage(image, fileName, imageWidth, imageHeight, true);
        return true;
    }

    // 如果从 bin\Debug 里启动 exe，当前目录可能不是项目目录，所以再多试几个位置。
    TCHAR currentPath[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, currentPath);

    TCHAR tryPath[MAX_PATH];
    _stprintf_s(tryPath, _T("%s\\GwenDodgeGame\\%s"), currentPath, fileName);
    if (gameFileExists(tryPath))
    {
        loadimage(image, tryPath, imageWidth, imageHeight, true);
        return true;
    }

    TCHAR exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);

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

        _stprintf_s(tryPath, _T("%s\\%s"), exePath, fileName);
        if (gameFileExists(tryPath))
        {
            loadimage(image, tryPath, imageWidth, imageHeight, true);
            return true;
        }

        _stprintf_s(tryPath, _T("%s\\..\\..\\%s"), exePath, fileName);
        if (gameFileExists(tryPath))
        {
            loadimage(image, tryPath, imageWidth, imageHeight, true);
            return true;
        }

        _stprintf_s(tryPath, _T("%s\\..\\..\\GwenDodgeGame\\%s"), exePath, fileName);
        if (gameFileExists(tryPath))
        {
            loadimage(image, tryPath, imageWidth, imageHeight, true);
            return true;
        }
    }

    return false;
}

// 随机一个整数，包含最小值和最大值。
int gameRandom(int minValue, int maxValue)
{
    return minValue + rand() % (maxValue - minValue + 1);
}

// 画带透明通道的 PNG。普通 putimage 有时会把透明部分画成黑色，所以这里自己混合像素。
void gameDrawPng(int drawX, int drawY, IMAGE* image)
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

            // 素材没有透明通道时，把接近白色的背景当成透明。
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

Game::Game()
{
    width = 960;
    height = 540;
    state = MENU;
    frameCount = 0;
    surviveFrame = 0;
    score = 0;
    spawnTimer = 0;
    spawnInterval = 90;
    running = true;

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
}

// 启动窗口，进入游戏循环。
void Game::run()
{
    init();
    BeginBatchDraw();

    while (running)
    {
        handleInput();
        update();
        draw();
        FlushBatchDraw();
        Sleep(16);
    }

    EndBatchDraw();
    closegraph();
}

// 初始化窗口和图片。
void Game::init()
{
    srand((unsigned)time(nullptr));
    initgraph(width, height);
    SetWindowText(GetHWnd(), _T("格温小姐大冒险"));
    loadResources();
}

// 这里检查 assets 下面的图片，路径不对就会自动使用占位图。
void Game::loadResources()
{
    hasBg = gameLoadImage(&imgBg, _T("assets/map2.png"), 960, 540);
    hasCover = gameLoadImage(&imgCover, _T("assets/cover.png"), 960, 540);
    hasEnd = gameLoadImage(&imgEnd, _T("assets/coverend.png"), 960, 540);
    hasGwenIdle = gameLoadImage(&imgGwenIdle, _T("assets/格温状态图.png"), 80, 80);
    hasGwenHurt = gameLoadImage(&imgGwenHurt, _T("assets/格温受击图.png"), 80, 80);
    hasEzQ = gameLoadImage(&imgEzQ, _T("assets/ezq.png"), 80, 32);
    hasAsheR = gameLoadImage(&imgAsheR, _T("assets/aceyr.png"), 140, 52);
    hasLuxWarning = gameLoadImage(&imgLuxWarning, _T("assets/Lux1.png"), 120, 120);
    hasLuxBoom = gameLoadImage(&imgLuxBoom, _T("assets/Lux2.png"), 120, 120);
    hasHeart = gameLoadImage(&imgHeart, _T("assets/hp.png"), 28, 28);
}

// 重新开始一局时，把所有数据改回初始值。
void Game::resetGame()
{
    player.reset();
    skills.clear();
    frameCount = 0;
    surviveFrame = 0;
    score = 0;
    spawnTimer = 0;
    spawnInterval = 90;
    state = PLAYING;
}

// 处理菜单、重开、退出和游戏中的按键。
void Game::handleInput()
{
    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
    {
        running = false;
        return;
    }

    if (state == MENU)
    {
        if (GetAsyncKeyState(VK_RETURN) & 0x8000)
        {
            resetGame();
            Sleep(120);
        }
        return;
    }

    if (state == WIN || state == GAME_OVER)
    {
        if (GetAsyncKeyState('R') & 0x8000)
        {
            resetGame();
            Sleep(120);
        }
        return;
    }

    if (state == PLAYING)
    {
        player.handleInput();
    }
}

// 每一帧更新游戏内容。
void Game::update()
{
    if (state != PLAYING)
    {
        return;
    }

    frameCount++;
    surviveFrame++;

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
        score += 10;
    }

    spawnTimer++;
    if (spawnTimer >= spawnInterval)
    {
        spawnSkill();
        spawnTimer = 0;
    }

    player.update();

    for (int i = 0; i < (int)skills.size(); i++)
    {
        skills[i].update();

        if (!skills[i].canCauseDamage())
        {
            continue;
        }

        bool isHit = false;
        if (skills[i].getType() == LUX_E)
        {
            isHit = circleCollide(skills[i].getX(), skills[i].getY(), (float)skills[i].getRadius(),
                player.getX(), player.getY(), (float)player.getRadius());
        }
        else
        {
            isHit = rectCircleCollide(skills[i].getRect(), player.getX(), player.getY(), (float)player.getRadius());
        }

        if (isHit)
        {
            if (!player.isInvincible())
            {
                player.hurt();
                skills[i].markDamaged();
                if (skills[i].getType() != LUX_E)
                {
                    skills[i].deactivate();
                }
            }
            else
            {
                skills[i].markDamaged();
            }
        }
    }

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

    if (surviveFrame >= WIN_SURVIVE_FRAME)
    {
        state = WIN;
    }
    else if (player.getHp() <= 0)
    {
        state = GAME_OVER;
    }
}

// 根据当前状态画不同界面。
void Game::draw()
{
    cleardevice();

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

// 生成一个新的技能。
void Game::spawnSkill()
{
    int type = gameRandom(0, 99);
    if (type < 40)
    {
        int edge = gameRandom(0, 3);
        float startX = 0.0f;
        float startY = 0.0f;

        if (edge == 0)
        {
            startX = -100.0f;
            startY = (float)gameRandom(0, height);
        }
        else if (edge == 1)
        {
            startX = (float)(width + 100);
            startY = (float)gameRandom(0, height);
        }
        else if (edge == 2)
        {
            startX = (float)gameRandom(0, width);
            startY = -100.0f;
        }
        else
        {
            startX = (float)gameRandom(0, width);
            startY = (float)(height + 100);
        }

        skills.push_back(Skill::createEzQ(startX, startY, player.getX(), player.getY()));
    }
    else if (type < 70)
    {
        int edge = gameRandom(0, 3);
        float startX = 0.0f;
        float startY = 0.0f;

        if (edge == 0)
        {
            startX = -100.0f;
            startY = (float)gameRandom(0, height);
        }
        else if (edge == 1)
        {
            startX = (float)(width + 100);
            startY = (float)gameRandom(0, height);
        }
        else if (edge == 2)
        {
            startX = (float)gameRandom(0, width);
            startY = -100.0f;
        }
        else
        {
            startX = (float)gameRandom(0, width);
            startY = (float)(height + 100);
        }

        skills.push_back(Skill::createAsheR(startX, startY, player.getX(), player.getY()));
    }
    else
    {
        int offsetX = gameRandom(-160, 160);
        int offsetY = gameRandom(-120, 120);
        float luxX = player.getX() + (float)offsetX;
        float luxY = player.getY() + (float)offsetY;

        if (luxX < 80.0f) luxX = 80.0f;
        if (luxX > width - 80.0f) luxX = (float)(width - 80);
        if (luxY < 80.0f) luxY = 80.0f;
        if (luxY > height - 80.0f) luxY = (float)(height - 80);

        skills.push_back(Skill::createLuxE(luxX, luxY));
    }
}

void Game::drawMenu()
{
    if (hasCover)
    {
        putimage(0, 0, &imgCover);
        return;
    }

    drawFallbackMap();
    setbkmode(TRANSPARENT);
    settextcolor(RGB(245, 250, 255));
    settextstyle(48, 0, _T("微软雅黑"));
    outtextxy(280, 130, _T("格温小姐大冒险"));

    settextstyle(24, 0, _T("微软雅黑"));
    settextcolor(RGB(225, 245, 255));
    outtextxy(285, 235, _T("WASD移动，躲避技能，存活60秒"));

    settextstyle(28, 0, _T("微软雅黑"));
    settextcolor(RGB(150, 240, 255));
    outtextxy(355, 315, _T("按 Enter 开始游戏"));
}

void Game::drawPlaying()
{
    if (hasBg)
    {
        putimage(0, 0, &imgBg);
    }
    else
    {
        drawFallbackMap();
    }

    for (int i = 0; i < (int)skills.size(); i++)
    {
        skills[i].draw(&imgEzQ, &imgAsheR, &imgLuxWarning, &imgLuxBoom,
            hasEzQ, hasAsheR, hasLuxWarning, hasLuxBoom);
    }

    player.draw(&imgGwenIdle, &imgGwenHurt, hasGwenIdle, hasGwenHurt);

    drawUI();
}

void Game::drawWin()
{
    if (hasEnd)
    {
        putimage(0, 0, &imgEnd);
        return;
    }

    drawFallbackMap();
    setbkmode(TRANSPARENT);
    settextstyle(42, 0, _T("微软雅黑"));
    settextcolor(RGB(235, 255, 245));
    outtextxy(220, 170, _T("胜利！成功躲过所有技能"));

    TCHAR text[64];
    _stprintf_s(text, _T("最终分数：%d"), score);
    settextstyle(28, 0, _T("微软雅黑"));
    outtextxy(385, 255, text);
    outtextxy(360, 320, _T("按 R 重新开始"));
}

void Game::drawGameOver()
{
    if (hasEnd)
    {
        putimage(0, 0, &imgEnd);
        return;
    }

    drawFallbackMap();
    setbkmode(TRANSPARENT);
    settextstyle(46, 0, _T("微软雅黑"));
    settextcolor(RGB(255, 220, 220));
    outtextxy(375, 170, _T("游戏失败"));

    TCHAR text[64];
    _stprintf_s(text, _T("最终分数：%d"), score);
    settextstyle(28, 0, _T("微软雅黑"));
    settextcolor(RGB(245, 245, 245));
    outtextxy(385, 255, text);
    outtextxy(360, 320, _T("按 R 重新开始"));
}

// 没有地图图片时，用简单图形画一个中路河道。
void Game::drawFallbackMap()
{
    setfillcolor(RGB(45, 90, 62));
    solidrectangle(0, 0, width, height);

    setfillcolor(RGB(30, 120, 135));
    solidrectangle(0, 205, width, 330);
    setfillcolor(RGB(45, 155, 160));
    solidrectangle(0, 235, width, 290);

    POINT road[4] = {
        {0, 495},
        {105, 540},
        {960, 95},
        {960, 20}
    };
    setfillcolor(RGB(118, 118, 105));
    solidpolygon(road, 4);

    setlinecolor(RGB(160, 160, 145));
    for (int i = -80; i < 960; i += 80)
    {
        line(i, 540, i + 960, 40);
    }

    setfillcolor(RGB(28, 120, 55));
    for (int grassX = 0; grassX < width; grassX += 28)
    {
        int topOffset = (grassX / 28) % 3 * 4;
        int bottomOffset = (grassX / 28) % 4 * 3;
        solidellipse(grassX, 30 + topOffset, grassX + 48, 105 + topOffset);
        solidellipse(grassX, 420 + bottomOffset, grassX + 50, 535);
    }

    setfillcolor(RGB(83, 82, 76));
    solidrectangle(0, 0, width, 26);
    solidrectangle(0, height - 26, width, height);
    solidrectangle(0, 0, 35, height);
    solidrectangle(width - 35, 0, width, height);

    setlinecolor(RGB(72, 185, 190));
    setlinestyle(PS_SOLID, 2);
    line(0, 205, width, 205);
    line(0, 330, width, 330);
    setlinestyle(PS_SOLID, 1);
}

// 画血量、时间、分数和难度。
void Game::drawUI()
{
    setbkmode(TRANSPARENT);
    settextstyle(22, 0, _T("微软雅黑"));
    settextcolor(RGB(255, 255, 255));

    for (int i = 0; i < player.getHp(); i++)
    {
        if (hasHeart)
        {
            gameDrawPng(20 + i * 34, 18, &imgHeart);
        }
        else
        {
            setfillcolor(RGB(255, 80, 105));
            solidcircle(31 + i * 34, 32, 11);
            solidcircle(43 + i * 34, 32, 11);
            POINT heart[3] = {
                {20 + i * 34, 36},
                {54 + i * 34, 36},
                {37 + i * 34, 56}
            };
            solidpolygon(heart, 3);
        }
    }

    TCHAR text[128];
    _stprintf_s(text, _T("时间：%d / 60 秒"), surviveFrame / 60);
    outtextxy(20, 62, text);

    _stprintf_s(text, _T("分数：%d"), score);
    outtextxy(20, 92, text);

    const TCHAR* stage = _T("初级");
    if (surviveFrame >= 40 * 60)
    {
        stage = _T("终局");
    }
    else if (surviveFrame >= 20 * 60)
    {
        stage = _T("中级");
    }
    _stprintf_s(text, _T("难度阶段：%s"), stage);
    outtextxy(20, 122, text);

}
