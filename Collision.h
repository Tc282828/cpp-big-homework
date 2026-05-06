#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <algorithm>
#include <cmath>
#include <graphics.h>

// 矩形和圆形碰撞：飞行技能用矩形，玩家用圆形
// 做法：把圆心夹到矩形里最近的点，再判断这个点到圆心的距离
inline bool rectCircleCollide(const RECT& rect, float cx, float cy, float radius)
{
    // 找到矩形上离圆心最近的点
    const float nearestX = std::max(static_cast<float>(rect.left), std::min(cx, static_cast<float>(rect.right)));
    const float nearestY = std::max(static_cast<float>(rect.top), std::min(cy, static_cast<float>(rect.bottom)));

    // 最近点到圆心的距离小于半径，就算碰撞
    const float dx = cx - nearestX;
    const float dy = cy - nearestY;
    return dx * dx + dy * dy <= radius * radius;
}

// 圆形和圆形碰撞：Lux E 爆炸圈和玩家都按圆形处理
inline bool circleCollide(float x1, float y1, float r1, float x2, float y2, float r2)
{
    // 两个圆心距离小于半径和，就算碰撞
    const float dx = x1 - x2;
    const float dy = y1 - y2;
    const float sum = r1 + r2;
    return dx * dx + dy * dy <= sum * sum;
}
