#include "tgaimage.h"
#include "model.h"
#include "common.h"
#include "geometry.h"

// 旋转
vec4 rot(vec4 v)
{
    constexpr double a = M_PI / 6;
    mat<4,4> Ry = {{{std::cos(a), 0, std::sin(a)}, {0,1,0}, {-std::sin(a), 0, std::cos(a)}}};
    vec4 result = Ry*v;

    return result;
}

// 透视投影
vec4 persp(vec4 v)
{
    constexpr double c = 3;
    return v / (1 - v.z / c);
}

void chapter5Func() 
{
    constexpr int width = 800;
    constexpr int height = 800;
    //Model model("obj\\african_head\\african_head.obj");
    Model model("obj\\diablo3_pose\\diablo3_pose.obj");
    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    for (int i = 0; i < model.nfaces(); ++i)
    {
        auto [ax, ay, az] = projectGray(persp(rot(model.vert(i, 0))), width, height);
        auto [bx, by, bz] = projectGray(persp(rot(model.vert(i, 1))), width, height);
        auto [cx, cy, cz] = projectGray(persp(rot(model.vert(i, 2))), width, height);
        if (az > 255) 
        {
            az = 255;
        }
        if (bz > 255) 
        {
            bz = 255;
        }
        if (cz > 255) 
        {
            cz = 255;
        }
        TGAColor rnd;
        for (int c = 0; c < 3; ++c)
        {
            rnd[c] = std::rand() % 255;
        }
        triangleWithZbuffer(ax, ay, az, bx, by, bz, cx, cy, cz, zbuffer, framebuffer, rnd);
    }

    framebuffer.write_tga_file("framebuffer.tga");
    zbuffer.write_tga_file("zbuffer.tga");
}