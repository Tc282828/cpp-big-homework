#include "Game.h"
#include "Collision.h"

#include <cstdlib>
#include <ctime>
#include <tchar.h>
#include <vector>
#include <windows.h>

const int WIN_SURVIVE_FRAME = 60 * 60;

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
    if (gameFileExists(fileName))
    {
        loadimage(image, fileName, imageWidth, imageHeight, true);
        return true;
    }

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

int gameRandom(int minValue, int maxValue)
{
    return minValue + rand() % (maxValue - minValue + 1);
}

bool isPngBackColor(DWORD color, DWORD cornerColor[4], bool cleanRotateBack, bool useWhiteBack)
{
    int red = color & 0xff;
    int green = (color >> 8) & 0xff;
    int blue = (color >> 16) & 0xff;

    if (cleanRotateBack && red < 8 && green < 8 && blue < 8)
    {
        return true;
    }

    if (useWhiteBack && red > 190 && green > 190 && blue > 190)
    {
        return true;
    }

    if (useWhiteBack)
    {
        for (int i = 0; i < 4; i++)
        {
            int cornerRed = cornerColor[i] & 0xff;
            int cornerGreen = (cornerColor[i] >> 8) & 0xff;
            int cornerBlue = (cornerColor[i] >> 16) & 0xff;
            int colorDistance =
                abs(red - cornerRed) +
                abs(green - cornerGreen) +
                abs(blue - cornerBlue);

            if (colorDistance < 120)
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
    if ((*skipPixel)[pixelIndex])
    {
        return;
    }

    if (isPngBackColor(imageBuffer[pixelIndex], cornerColor, cleanRotateBack, useWhiteBack))
    {
        (*skipPixel)[pixelIndex] = 1;
        queue->push_back(pixelIndex);
    }
}

void drawPngAlpha(int drawX, int drawY, IMAGE* image, bool cleanRotateBack)
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

    bool hasAlphaValue = false;
    bool hasNotFullAlpha = false;
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
    int pixelCount = imageWidth * imageHeight;
    std::vector<char> skipPixel(pixelCount, 0);

    if (!useAlpha || cleanRotateBack)
    {
        std::vector<int> queue;

        for (int x = 0; x < imageWidth; x++)
        {
            markPngBackPixel(x, imageBuffer, cornerColor, cleanRotateBack, !useAlpha, &skipPixel, &queue);
            markPngBackPixel((imageHeight - 1) * imageWidth + x, imageBuffer, cornerColor,
                cleanRotateBack, !useAlpha, &skipPixel, &queue);
        }

        for (int y = 0; y < imageHeight; y++)
        {
            markPngBackPixel(y * imageWidth, imageBuffer, cornerColor, cleanRotateBack, !useAlpha, &skipPixel, &queue);
            markPngBackPixel(y * imageWidth + imageWidth - 1, imageBuffer, cornerColor,
                cleanRotateBack, !useAlpha, &skipPixel, &queue);
        }

        int head = 0;
        while (head < (int)queue.size())
        {
            int pixelIndex = queue[head];
            head++;

            int x = pixelIndex % imageWidth;
            int y = pixelIndex / imageWidth;

            if (x > 0)
            {
                markPngBackPixel(pixelIndex - 1, imageBuffer, cornerColor,
                    cleanRotateBack, !useAlpha, &skipPixel, &queue);
            }
            if (x < imageWidth - 1)
            {
                markPngBackPixel(pixelIndex + 1, imageBuffer, cornerColor,
                    cleanRotateBack, !useAlpha, &skipPixel, &queue);
            }
            if (y > 0)
            {
                markPngBackPixel(pixelIndex - imageWidth, imageBuffer, cornerColor,
                    cleanRotateBack, !useAlpha, &skipPixel, &queue);
            }
            if (y < imageHeight - 1)
            {
                markPngBackPixel(pixelIndex + imageWidth, imageBuffer, cornerColor,
                    cleanRotateBack, !useAlpha, &skipPixel, &queue);
            }
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

            int imageIndex = y * imageWidth + x;
            if (skipPixel[imageIndex])
            {
                continue;
            }

            DWORD sourceColor = imageBuffer[imageIndex];
            int alpha = (sourceColor >> 24) & 0xff;
            int sourceRed = sourceColor & 0xff;
            int sourceGreen = (sourceColor >> 8) & 0xff;
            int sourceBlue = (sourceColor >> 16) & 0xff;

            if (useAlpha && alpha == 0)
            {
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
    resourcesReady = false;

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

void Game::init()
{
    srand((unsigned)time(nullptr));
    initgraph(width, height);
    SetWindowText(GetHWnd(), _T("格温小姐大冒险"));
    loadResources();
}

void Game::loadResources()
{
    hasCover = gameLoadImage(&imgCover, _T("assets/cover.png"), 960, 540);
    hasEnd = gameLoadImage(&imgEnd, _T("assets/coverend.png"), 960, 540);
    hasBg = gameLoadImage(&imgBg, _T("assets/map2.png"), 960, 540);

    hasGwenIdle = gameLoadImage(&imgGwenIdle, _T("assets/格温状态图.png"), 80, 80);
    hasGwenHurt = gameLoadImage(&imgGwenHurt, _T("assets/格温受击图.png"), 80, 80);

    hasEzQ = gameLoadImage(&imgEzQ, _T("assets/ezq.png"), 80, 32);
    hasAsheR = gameLoadImage(&imgAsheR, _T("assets/aceyr.png"), 140, 52);
    hasLuxWarning = gameLoadImage(&imgLuxWarning, _T("assets/Lux1.png"), 120, 120);
    hasLuxBoom = gameLoadImage(&imgLuxBoom, _T("assets/Lux2.png"), 120, 120);
    hasHeart = gameLoadImage(&imgHeart, _T("assets/hp.png"), 28, 28);

    resourcesReady = hasCover && hasEnd && hasBg && hasGwenIdle && hasGwenHurt
        && hasEzQ && hasAsheR && hasLuxWarning && hasLuxBoom && hasHeart;
}

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

void Game::draw()
{
    cleardevice();

    if (!resourcesReady)
    {
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
    int edge = gameRandom(0, 3);

    if (edge == 0)
    {
        *startX = -100.0f;
        *startY = (float)gameRandom(0, height);
    }
    else if (edge == 1)
    {
        *startX = (float)(width + 100);
        *startY = (float)gameRandom(0, height);
    }
    else if (edge == 2)
    {
        *startX = (float)gameRandom(0, width);
        *startY = -100.0f;
    }
    else
    {
        *startX = (float)gameRandom(0, width);
        *startY = (float)(height + 100);
    }
}

void Game::spawnSkill()
{
    int type = gameRandom(0, 99);
    if (type < 40)
    {
        float startX = 0.0f;
        float startY = 0.0f;
        getRandomEdgePoint(&startX, &startY);
        skills.push_back(Skill::createEzQ(startX, startY, player.getX(), player.getY()));
    }
    else if (type < 70)
    {
        float startX = 0.0f;
        float startY = 0.0f;
        getRandomEdgePoint(&startX, &startY);
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
    putimage(0, 0, &imgCover);
}

void Game::drawPlaying()
{
    putimage(0, 0, &imgBg);

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
    setbkmode(TRANSPARENT);
    settextstyle(22, 0, _T("微软雅黑"));
    settextcolor(RGB(255, 255, 255));

    for (int i = 0; i < player.getHp(); i++)
    {
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
        stage = _T("终局");
    }
    else if (surviveFrame >= 20 * 60)
    {
        stage = _T("中级");
    }
    _stprintf_s(text, _T("难度阶段：%s"), stage);
    outtextxy(20, 122, text);
}

void Game::drawResourceError()
{
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
