#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Game.h"

#include <windows.h>

// 程序入口：创建游戏对象并启动主循环。
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    Game game;
    game.run();
    return 0;
}
