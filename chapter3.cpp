#include "tgaimage.h"
#include "common.h"

using namespace std;

// 根据子三角形的面积占比绘制灰度 
void triangleWithGray(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage& framebuffer)
{
    int bbminx = min(min(ax, bx), cx);
    int bbminy = min(min(ay, by), cy);
    int bbmaxx = max(max(ax, bx), cx);
    int bbmaxy = max(max(ay, by), cy);
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    //简易的背面剔除
    if (total_area < 0)
    {
        return;
    }

#pragma omp parallel for
    for (int x = bbminx; x <= bbmaxx; ++x)
    {
        for (int y = bbminy; y <= bbmaxy; ++y)
        {
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha < 0 || beta < 0 || gamma < 0)
            {
                continue;
            }
            unsigned char z = static_cast<unsigned char>(alpha * az + beta * bz + gamma * cz);
            framebuffer.set(x, y, {z});
        }
    }
}

// 根据子三角形的面积占比决定绘制的颜色
void triangleWithColor(int ax, int ay, int bx, int by, int cx, int cy, TGAImage& framebuffer)
{
    int bbminx = min(min(ax, bx), cx);
    int bbminy = min(min(ay, by), cy);
    int bbmaxx = max(max(ax, bx), cx);
    int bbmaxy = max(max(ay, by), cy);
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    //简易的背面剔除
    if (total_area < 0)
    {
        return;
    }

#pragma omp parallel for
    for (int x = bbminx; x <= bbmaxx; ++x)
    {
        for (int y = bbminy; y <= bbmaxy; ++y)
        {
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha < 0 || beta < 0 || gamma < 0)
            {
                continue;
            }
            //bgr
            TGAColor color;
            color[0] = 255 * alpha;
            color[1] = 255 * beta;
            color[2] = 255 * gamma;
            framebuffer.set(x, y, color);
        }
    }
}

// 先计算出三角形的中心，再获取内部等比缩小后的小三角形的三个顶点，在绘制原三角形中的每个像素点的时候，如果在小三角形中则不绘制像素，最终结果就是中间位置是个空三角形
void triangleWithColorWireFrame(int ax, int ay, int bx, int by, int cx, int cy, TGAImage& framebuffer)
{
    int bbminx = min(min(ax, bx), cx);
    int bbminy = min(min(ay, by), cy);
    int bbmaxx = max(max(ax, bx), cx);
    int bbmaxy = max(max(ay, by), cy);
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    //简易的背面剔除
    if (total_area < 0)
    {
        return;
    }

    //计算出三角形的中心点
    int center_x = 0.33 * ax + 0.33 * bx + 0.33 * cx;
    int center_y = 0.33 * ay + 0.33 * by + 0.33 * cy;
    //三角形等比缩小后的小三角形的三个顶点
    int temp_ax = center_x + 0.65 * (ax - center_x);
    int temp_ay = center_y + 0.65 * (ay - center_y);
    int temp_bx = center_x + 0.65 * (bx - center_x);
    int temp_by = center_y + 0.65 * (by - center_y);
    int temp_cx = center_x + 0.65 * (cx - center_x);
    int temp_cy = center_y + 0.65 * (cy - center_y);
    double temp_total_area = signed_triangle_area(temp_ax, temp_ay, temp_bx, temp_by, temp_cx, temp_cy);

#pragma omp parallel for
    for (int x = bbminx; x <= bbmaxx; ++x)
    {
        for (int y = bbminy; y <= bbmaxy; ++y)
        {
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha < 0 || beta < 0 || gamma < 0)
            {
                continue;
            }
            double t_alpha = signed_triangle_area(x, y, temp_bx, temp_by, temp_cx, temp_cy) / temp_total_area;
            double t_beta = signed_triangle_area(x, y, temp_cx, temp_cy, temp_ax, temp_ay) / temp_total_area;
            double t_gamma = signed_triangle_area(x, y, temp_ax, temp_ay, temp_bx, temp_by) / temp_total_area;
            //如果坐标点不在小三角形内部则进行绘制，否则不处理，形成将中间"掏空"的效果
            if (t_alpha < 0 || t_beta < 0 || t_gamma < 0)
            {
                //bgr
                TGAColor color;
                color[0] = 255 * alpha;
                color[1] = 255 * beta;
                color[2] = 255 * gamma;
                framebuffer.set(x, y, color);
            }
            else
            {
                continue;
            }
        }
    }
}

void chapter3Func() 
{
    constexpr int width = 64;
    constexpr int height = 64;
    //TGAImage framebuffer(width, height, TGAImage::GRAYSCALE);
    TGAImage framebuffer(width, height, TGAImage::RGB);

    int ax = 17, ay = 4, az = 13;
    int bx = 55, by = 39, bz = 128;
    int cx = 23, cy = 59, cz = 255;

    //triangleWithGray(ax, ay, az, bx, by, bz, cx, cy, cz, framebuffer);
    //triangleWithColor(ax, ay, bx, by, cx, cy, framebuffer);
    triangleWithColorWireFrame(ax, ay, bx, by, cx, cy, framebuffer);

    framebuffer.write_tga_file("framebuffer.tga");
}