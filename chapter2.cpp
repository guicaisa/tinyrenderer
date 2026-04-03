#include "tgaimage.h"
#include "common.h"
#include <unordered_map>

using namespace std;

//根据三角形的3个点，绘制3条直线，构成一个三角形，只有边框，中间部分未被填充
void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage& framebuffer, TGAColor color)
{
    drawLine(ax, ay, bx, by, framebuffer, color);
    drawLine(bx, by, cx, cy, framebuffer, color);
    drawLine(cx, cy, ax, ay, framebuffer, color);
}

//填充三角形内部像素的自己实现，在画线的过程中记录每个y坐标关联的x坐标的左右范围
//在画完线之后，将每个y坐标滚来你的2个x坐标当作一条直线，绘制中间的每一个点，从而达到填充整个三角形的效果
void triangleFillSelfImpl(int ax, int ay, int bx, int by, int cx, int cy, TGAImage& framebuffer, TGAColor color) 
{
    unordered_map<int, pair<int, int>> row_range;
    drawLineRecordRowRange(ax, ay, bx, by, framebuffer, color, row_range);
    drawLineRecordRowRange(bx, by, cx, cy, framebuffer, color, row_range);
    drawLineRecordRowRange(cx, cy, ax, ay, framebuffer, color, row_range);

    for (auto it = row_range.begin(); it != row_range.end(); ++it)
    {
        for (int x = it->second.first; x <= it->second.second; ++x)
        {
            framebuffer.set(x, it->first, color);
        }
    }
}

//根据每个点的y坐标进行升序排列
void triangleSort(int ax, int ay, int bx, int by, int cx, int cy, TGAImage& framebuffer, TGAColor color) 
{
    if (ay > by)
    {
        swap(ax, bx);
        swap(ay, by);
    }
    if (ay > cy)
    {
        swap(ax, cx);
        swap(ay, cy);
    }
    if (by > cy)
    {
        swap(bx, cx);
        swap(by, cy);
    }
    drawLine(ax, ay, bx, by, framebuffer, green);
    drawLine(bx, by, cx, cy, framebuffer, green);
    drawLine(cx, cy, ax, ay, framebuffer, red);
}

//从y坐标中位的点为切点，横切一半，绘制下半部分的内容
void triangleCutHorizon(int ax, int ay, int bx, int by, int cx, int cy, TGAImage& framebuffer, TGAColor color) 
{
    if (ay > by)
    {
        swap(ax, bx);
        swap(ay, by);
    }
    if (ay > cy)
    {
        swap(ax, cx);
        swap(ay, cy);
    }
    if (by > cy)
    {
        swap(bx, cx);
        swap(by, cy);
    }
    int total_height = cy - ay;

    //ay == by的情况下，底边是平行于x轴的
    if (ay != by)
    {
        int segment_height = by - ay; //下半段的高度
        for (int y = ay; y <= by; ++y)
        {
            //计算出左右两点，绘制线段
            int x1 = ax + ((cx - ax) * (y - ay)) / total_height;
            int x2 = ax + ((bx - ax) * (y - ay)) / segment_height;
            framebuffer.set(x1, y, red);
            framebuffer.set(x2, y, green);
        }
    }
}

// triangleCutHorizon的进阶，绘制出下半部分的三角形之后，将同y坐标同一水平线上的x1,x2全部绘制出来，填充整个下半部分的三角形
void triangleCutHorizonFill(int ax, int ay, int bx, int by, int cx, int cy, TGAImage& framebuffer, TGAColor color) 
{
if (ay > by)
    {
        swap(ax, bx);
        swap(ay, by);
    }
    if (ay > cy)
    {
        swap(ax, cx);
        swap(ay, cy);
    }
    if (by > cy)
    {
        swap(bx, cx);
        swap(by, cy);
    }
    int total_height = cy - ay;

    //ay == by的情况下，底边是平行于x轴的
    if (ay != by)
    {
        int segment_height = by - ay; //下半段的高度
        for (int y = ay; y <= by; ++y)
        {
            //计算出左右两点，绘制线段
            int x1 = ax + ((cx - ax) * (y - ay)) / total_height;
            int x2 = ax + ((bx - ax) * (y - ay)) / segment_height;
            for (int x = min(x1, x2); x < max(x1, x2); ++x)
            {
                framebuffer.set(x, y, color);
            }
        }
    }
}

// 在triangleCutHorizonFill的基础上，使用相同的方法绘制出三角形的上半部分，从而填充满整个三角形
void triangleScanLine(int ax, int ay, int bx, int by, int cx, int cy, TGAImage& framebuffer, TGAColor color) 
{
if (ay > by)
    {
        swap(ax, bx);
        swap(ay, by);
    }
    if (ay > cy)
    {
        swap(ax, cx);
        swap(ay, cy);
    }
    if (by > cy)
    {
        swap(bx, cx);
        swap(by, cy);
    }
    int total_height = cy - ay;

    //ay == by的情况下，底边是平行于x轴的
    if (ay != by)
    {
        int segment_height = by - ay; //下半段的高度
        for (int y = ay; y <= by; ++y)
        {
            //计算出左右两点，绘制线段
            int x1 = ax + ((cx - ax) * (y - ay)) / total_height;
            int x2 = ax + ((bx - ax) * (y - ay)) / segment_height;
            for (int x = min(x1, x2); x < max(x1, x2); ++x)
            {
                framebuffer.set(x, y, color);
            }
        }
    }
    if (by != cy)
    {
        int segment_height = cy - by; //上半段的高度
        for (int y = by; y <= cy; ++y)
        {
            int x1 = ax + ((cx - ax) * (y - ay)) / total_height;
            int x2 = bx + ((cx - bx) * (y - by)) / segment_height;
            for (int x = min(x1, x2); x < max(x1, x2); ++x)
            {
                framebuffer.set(x, y, color);
            }
        }
    }
}


void chapter2Func()
{
    constexpr int width = 250;
    constexpr int height = 250;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    triangleScanLine(7, 45, 35, 100, 45, 60, framebuffer, red);
    triangleScanLine(120, 35, 90, 5, 45, 110, framebuffer, white);
    triangleScanLine(115, 83, 80, 90, 85, 120, framebuffer, green);
    framebuffer.write_tga_file("framebuffer.tga");
}