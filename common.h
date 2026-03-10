#pragma once
#include "tgaimage.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

void drawLine(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color)
{
    bool steep = std::abs(ax-bx) < std::abs(ay-by);
    //y轴比较陡峭，则将线段转置
    if (steep)
    {
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    //保证x轴从左到右绘制
    if (ax > bx)
    {
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    int y = ay;
    int ierror = 0;
    for (int x = ax; x <= bx; ++x)
    {
        //如果之前转置过了，再转置一次后再绘制
        if (steep)
        {
            framebuffer.set(y, x, color);
        }
        else
        {
            framebuffer.set(x, y, color);
        }
        ierror += 2 * std::abs(by-ay);
        y += (by > ay ? 1 : -1) * (ierror > bx - ax);
        ierror -= 2 * (bx-ax)   * (ierror > bx - ax);
    }
}

std::tuple<int, int> project(float x, float y, float z, int width, int height)
{
    return {
        (x + 1.) * width / 2,
        (y + 1.) * height / 2,
    };
}