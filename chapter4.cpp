#include "tgaimage.h"
#include "model.h"
#include "common.h"
#include "geometry.h"

// 用灰度图表示深度，记录像素点的深度，如果深度小于当前深度则丢弃不绘制，否则绘制并更新深度值
void triangleZbuffer(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage& zbuffer, TGAImage& framebuffer, TGAColor color)
{
    int bbminx = std::min(std::min(ax, bx), cx);
    int bbminy = std::min(std::min(ay, by), cy);
    int bbmaxx = std::max(std::max(ax, bx), cx);
    int bbmaxy = std::max(std::max(ay, by), cy);
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area < 0)
    {
        return ;
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
            if (z <= zbuffer.get(x, y)[0])
            {
                continue;
            }
            zbuffer.set(x, y, {z});
            framebuffer.set(x, y, color);
        }
    }
}

void chapter4Func()
{
    constexpr int width = 800;
    constexpr int height = 800;
    Model model("obj\\diablo3_pose\\diablo3_pose.obj");
    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    for (int i = 0; i < model.nfaces(); ++i)
    {
        auto [ax, ay, az] = projectGray(model.vert(i, 0), width, height);
        auto [bx, by, bz] = projectGray(model.vert(i, 1), width, height);
        auto [cx, cy, cz] = projectGray(model.vert(i, 2), width, height);
        TGAColor rnd;
        for (int c = 0; c < 3; ++c)
        {
            rnd[c] = std::rand() % 255;
        }
        triangleZbuffer(ax, ay, az, bx, by, bz, cx, cy, cz, zbuffer, framebuffer, rnd);
    }

    framebuffer.write_tga_file("framebuffer.tga");
    zbuffer.write_tga_file("zbuffer.tga");
}