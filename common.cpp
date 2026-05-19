#include "common.h"
#include <unordered_map>

using namespace std;

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

void drawLineRecordRowRange(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color, unordered_map<int, pair<int, int>>& row_range)
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
        int temp_x = x;
        int temp_y = y;
        //如果之前转置过了，再转置一次后再绘制
        if (steep)
        {
            framebuffer.set(y, x, color);
            temp_x = y;
            temp_y = x;
        }
        else
        {
            framebuffer.set(x, y, color);
        }
        ierror += 2 * std::abs(by-ay);
        y += (by > ay ? 1 : -1) * (ierror > bx - ax);
        ierror -= 2 * (bx-ax)   * (ierror > bx - ax);

        if (row_range.find(temp_y) == row_range.end())
        {
            row_range[temp_y] = make_pair(temp_x, temp_x);
        }
        else
        {
            row_range[temp_y].first = min( row_range[temp_y].first, temp_x);
            row_range[temp_y].second = max( row_range[temp_y].second, temp_x);
        }
    }
}

std::tuple<int, int> project(float x, float y, float z, int width, int height)
{
    return {
        (x + 1.) * width / 2,
        (y + 1.) * height / 2,
    };
}

std::tuple<int, int, int> projectGray(vec4 v, int width, int height)
{
    return {
        (v.x + 1.) * width / 2,  
        (v.y + 1.) * height / 2,
        (v.z + 1.) * 255 / 2,
    };
}

double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy)
{
    return 0.5 * ((by-ay)*(bx+ax) + (cy-by)*(cx+bx) + (ay-cy)*(ax+cx));
}