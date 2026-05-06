#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Game.h"

#include <windows.h>

// 程序入口：Windows 图形程序从 wWinMain 开始运行
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    // 创建游戏对象，然后进入游戏主循环
    Game game;
    game.run();
    return 0;
}
