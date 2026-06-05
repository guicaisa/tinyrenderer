#include "tgaimage.h"
#include "model.h"
#include "common.h"
#include "geometry.h"
#include <algorithm>

mat<4,4> TempModelView, ViewPort, TempPerspective;

void tempLookat(const vec3 eye, const vec3 center, const vec3 up)
{
    vec3 n = normalized(eye - center);
    vec3 l = normalized(cross(up, n));
    vec3 m = normalized(cross(n, l));
    TempModelView = mat<4,4>{{{l.x, l.y, l.z, 0}, {m.x, m.y, m.z, 0}, {n.x, n.y, n.z, 0}, {0,0,0,1}}} *
    mat<4, 4>{{{1,0,0,-center.x}, {0, 1, 0, -center.y}, {0,0,1,-center.z}, {0,0,0,1}}};
}

void perspective(const double f)
{
    TempPerspective = {{{1, 0, 0, 0}, {0,1,0,0}, {0,0,1,0}, {0,0,-1/f, 1}}};
}

void viewport(const int x, const int y, const int w, const int h)
{
    ViewPort = {{{w/2., 0, 0, x + w/2.}, {0, h/2., 0, y + h/2.}, {0,0,1,0}, {0,0,0,1}}};
}

void rasterize(const vec4 clip[3], std::vector<double>& zbuffer, TGAImage& framebuffer, const TGAColor color)
{
    vec4 ndc[3] = {clip[0]/clip[0].w, clip[1]/clip[1].w, clip[2]/clip[2].w};
    vec2 screen[3] = {(ViewPort*ndc[0]).xy(), (ViewPort*ndc[1]).xy(), (ViewPort*ndc[2]).xy()};

    mat<3,3> ABC = {{{screen[0].x, screen[0].y, 1.}, {screen[1].x, screen[1].y, 1.}, {screen[2].x, screen[2].y, 1.}}};
    if (ABC.det() < 1)
    {
        return ;
    }

    auto [bbminx, bbmaxx] = std::minmax({screen[0].x, screen[1].x, screen[2].x});
    auto [bbminy, bbmaxy] = std::minmax({screen[0].y, screen[1].y, screen[2].y});
#pragma omp parallel for
    for (int x=std::max<int>(bbminx, 0); x <= std::min<int>(bbmaxx, framebuffer.width() - 1); ++x)
    {
        for (int y = std::max<int>(bbminy, 0); y <= std::min<int>(bbmaxy, framebuffer.height() - 1); ++y)
        {
            vec3 bc = ABC.invert_transpose() * vec3{static_cast<double>(x), static_cast<double>(y), 1.};
            if (bc.x < 0 || bc.y < 0 || bc.z < 0)
            {
                continue;
            }
            double z = bc * vec3{ndc[0].z, ndc[1].z, ndc[2].z};
            if (z <= zbuffer[x+y*framebuffer.width()])
            {
                continue;
            }
            zbuffer[x+y*framebuffer.width()] = z;
            framebuffer.set(x, y, color);
        }
    }

}

void chapter6Func() 
{
    constexpr int width = 800;
    constexpr int height = 800;
    constexpr vec3 eye{-1,0,2};
    constexpr vec3 center{0,0,0};
    constexpr vec3 up{0,1,0};

    tempLookat(eye, center, up);
    perspective(norm(eye - center));
    viewport(width/16, height/16, width*7/8, height*7/8);

    TGAImage framebuffer(width, height, TGAImage::RGB);
    std::vector<double> zbuffer(width*height, -std::numeric_limits<double>::max());

    Model model("obj\\diablo3_pose\\diablo3_pose.obj");
    for (int i = 0; i < model.nfaces(); ++i)
    {
        vec4 clip[3];
        for (int d : {0, 1, 2})
        {
            vec4 v = model.vert(i, d);
            clip[d] = TempPerspective * TempModelView * vec4{v.x, v.y, v.z, 1.};
        }
        TGAColor rnd;
        for (int c = 0; c < 3; ++c)
        {
            rnd[c] = std::rand() % 255;
        }
        rasterize(clip, zbuffer, framebuffer, rnd);
    }

    framebuffer.write_tga_file("framebuffer.tga");
}