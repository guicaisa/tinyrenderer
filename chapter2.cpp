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

// 根据三角形的三个点，计算出左上/右上/左下/右下四个点，绘制出一个矩形
void triangleBoundingBox(int ax, int ay, int bx, int by, int cx, int cy, TGAImage& framebuffer, TGAColor color)
{
    int bbminx = min(min(ax, bx), cx);
    int bbminy = min(min(ay, by), cy);
    int bbmaxx = max(max(ax, bx), cx);
    int bbmaxy = max(max(ay, by), cy);
#pragma omp parallel for
    for (int x = bbminx; x <= bbmaxx; ++x)
    {
        for (int y = bbminy; y <= bbmaxy; ++y)
        {
            framebuffer.set(x, y, color);
        }
    }
}

// double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy)
// {
//     return 0.5 * ((by-ay)*(bx+ax) + (cy-by)*(cx+bx) + (ay-cy)*(ax+cx));
// }

// 在triangleBoundingBox的基础上，仍然绘制出一个矩形，但是对矩形中的每个元素进行判断其是否处于三角形的内部，如果存在才绘制，否则不绘制，从而绘制出一个实际的三角形
void triangleBoundingBoxFilter(int ax, int ay, int bx, int by, int cx, int cy, TGAImage& framebuffer, TGAColor color)
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
            framebuffer.set(x, y, color);
        }
    }
}

void wireFrameRenderingWithDrawTriangle()
{
    std::ifstream file("obj\\african_head\\african_head.obj");
    if (!file.is_open())
    {
        printf("cant open file\n");
        return;
    }

    int width = 800;
    int height = 800;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    std::vector<std::vector<float>> vertices;
    std::vector<std::vector<int>> faces;
    
    std::string line;
    while (std::getline(file, line))
    {
        if (line.size() < 2)
        {
            continue;
        }
        //vertex, v开头，空格分割的3个浮点数，表示顶点坐标xyz
        if (line.substr(0, 2) == "v ")
        {
            std::vector<float> vertex;
            int start_idx = 2;
            for (int i = 0; i < 3; ++i)
            {
                int idx = line.find_first_of(" ", start_idx);
                float v = std::stof(line.substr(start_idx, idx - start_idx));
                vertex.emplace_back(v);
                start_idx = idx + 1;
            }
            vertices.emplace_back(vertex);
        }
        //face，f开头，空格分割3个顶点数据，每个顶点由/分割，第一值表示顶点索引，指向v开头的顶点数组元素，3个顶点构成1个面
        if (line.substr(0, 2) == "f ")
        {
            std::vector<int> face;
            int start_idx = 2;
            for (int i = 0; i < 3; ++i)
            {
                int idx = line.find_first_of(" ", start_idx);
                std::string str = line.substr(start_idx, idx - start_idx);
                start_idx = idx + 1;
                
                int slash_idx = str.find_first_of("/", 0);
                int f = std::stoi(str.substr(0, slash_idx));
                face.emplace_back(f);
            }
            faces.emplace_back(face);
        }
    }
    file.close();

    //遍历所有面
    for (int i = 0; i < faces.size(); ++i)
    {
        //一个面关联的3个顶点
        std::vector<int> face = faces[i];
        std::vector<float> v1 = vertices[face[0]-1];
        std::vector<float> v2 = vertices[face[1]-1];
        std::vector<float> v3 = vertices[face[2]-1];
        //顶点坐标投影到屏幕像素坐标中
        auto [ax, ay] = project(v1[0], v1[1], 0, width, height);
        auto [bx, by] = project(v2[0], v2[1], 0, width, height);
        auto [cx, cy] = project(v3[0], v3[1], 0, width, height);
        TGAColor rnd;
        for (int c=0; c<3; c++) 
        {
            rnd[c] = std::rand()%255;
        }
        //画三角形
        triangleBoundingBoxFilter(ax, ay, bx, by, cx, cy, framebuffer, rnd);
    }

    for (int i = 0; i < vertices.size(); ++i)
    {
        std::vector<float> v = vertices[i];
        auto [x, y] = project(v[0], v[1], 0, width, height);
        framebuffer.set(x, y, white);
    }

    framebuffer.write_tga_file("framebuffer.tga");
}

void chapter2Func()
{
    wireFrameRenderingWithDrawTriangle();
    return;

    constexpr int width = 250;
    constexpr int height = 250;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    triangleBoundingBoxFilter(7, 45, 35, 100, 45, 60, framebuffer, red);
    triangleBoundingBoxFilter(120, 35, 90, 5, 45, 110, framebuffer, white);
    triangleBoundingBoxFilter(115, 83, 80, 90, 85, 120, framebuffer, green);
    framebuffer.write_tga_file("framebuffer.tga");
}