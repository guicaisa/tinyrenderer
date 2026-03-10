#include "tgaimage.h"
#include "common.h"
#include <cmath>
#include <vector>

//以t为间隔来进行循环，绘制一条线段上的所有点
//问题1：黄色的线被红色的覆盖，但是由于绘制的点不完全相同，漏出了部分黄色的点，理想情况应该是红色和黄色线的像素点完全一致
//问题2：红色线段中间会有间隙，因为红色线段x轴为62-7=55，需要55个像素点，但是t为0.02，只有51个采样像素点，导致中间会出间隙
void lineStepByT(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color) 
{
    for (float t = 0; t < 1; t += 0.02)
    {
        int x = round(ax + (bx - ax) * t);
        int y = round(ay + (by - ay) * t);
        framebuffer.set(x, y, color);
    }
}

//以x轴从左到右作为遍历方向，保证能绘制出x轴上的每个像素点
//问题1: 绿色的线由于函数参数ax > bx的原因，没有被绘制出来(未进行循环)
//问题2：蓝色的线x轴间距太短，y轴间距很大(x轴的长度作为采样精度)，比较陡峭，导致y轴上有大量的间隔
void lineAlongXAxis(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color)
{
    for (int x = ax; x <= bx; ++x)
    {
        float t = (x-ax) / static_cast<float>(bx - ax);
        int y = std::round(ay + (by - ay) * t);
        framebuffer.set(x, y, color);
    }
}

//先解决绿色的线没有被绘制出来的问题
//检查ax和bx大小，保证ax始终小于bx，这也保证了红色和黄色线绘制的所有像素都完全一致，只是由于绘制顺序黄色的线被红色的覆盖了
void lineAlongXAxisFromLeftToRight(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color)
{
    //保证x轴从左到右绘制
    if (ax > bx)
    {
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    for (int x = ax; x <= bx; ++x)
    {
        float t = (x-ax) / static_cast<float>(bx - ax);
        int y = std::round(ay + (by - ay) * t);
        framebuffer.set(x, y, color);
    }
}

//根据x轴和y轴的陡峭程度来判断，如果是y轴比较陡峭的话，就进行一次转置，以y轴为循环基础进行绘制
//同时在绘制具体的点的时候，将点数据再转置一次后再绘制，这种情况下以较长的一边作为遍历采样点个数，保证不会出现间隙
void lineAlongDecideBySteep(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color)
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
    for (int x = ax; x <= bx; ++x)
    {
        float t = (x-ax) / static_cast<float>(bx - ax);
        int y = std::round(ay + (by - ay) * t);
        //如果之前转置过了，再转置一次后再绘制
        if (steep)
        {
            framebuffer.set(y, x, color);
        }
        else
        {
            framebuffer.set(x, y, color);
        }
    }
}

//优化y的计算方式，初始化为ay，然后每次递增固定长度，节省一次计算
void lineOptimization1(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color)
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
    float y = ay;
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
        //每次递增固定值，因为原计算方式中(x-ax)的结果是0,1,2...可以通过累加y得到，省掉一个乘法
        y += (by-ay) / static_cast<float>(bx - ax);
    }
}

//去掉y从float到int的转换，使用error来累计每次递增的值，超过0.5即让y+1(round函数的四舍五入)
//由于经过转置，所以当前y轴的陡峭程度肯定比x小，所以y每次递增的值一定小于1
//y在范围[0.5, 1.5)之间都会被视作1，所以error在每次y+1之后都需要-1
void lineOptimization2(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color)
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
    float error = 0;
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
        error +=  std::abs(by-ay) / static_cast<float>(bx - ax);
        if (error > .5) 
        {
            y += by > ay ? 1 : -1;
            error -= 1.;
        }
    }
}

//将优化2中的浮点类型error改成了整型ierror，相关操作通过公式变化成了整型加减法
//std::abs(by-ay) / static_cast<float>(bx - ax) > 0.5  =======> 2 * std::abs(by-ay) > static_cast<float>(bx - ax)
void lineOptimization3(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color)
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
        if (ierror > (bx -ax)) 
        {
            y += by > ay ? 1 : -1;
            ierror -= 2 * (bx-ax);
        }
    }
}

//去掉if (ierror > (bx -ax))语句
void lineOptimization4(int ax, int ay, int bx, int by, TGAImage& framebuffer, TGAColor color)
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

void wireFrameRendering()
{
    std::ifstream file("obj\\diablo3_pose\\diablo3_pose.obj");
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
        //画线，构成一个面
        drawLine(ax, ay, bx, by, framebuffer, red);
        drawLine(bx, by, cx, cy, framebuffer, red);
        drawLine(cx, cy, ax, ay, framebuffer, red);
    }

    for (int i = 0; i < vertices.size(); ++i)
    {
        std::vector<float> v = vertices[i];
        auto [x, y] = project(v[0], v[1], 0, width, height);
        framebuffer.set(x, y, white);
    }

    framebuffer.write_tga_file("framebuffer.tga");
}

void chapter1Func() 
{
    wireFrameRendering();
    return;

    constexpr int width = 64;
    constexpr int height = 64;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    int ax = 7, ay = 3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    lineOptimization4(ax, ay, bx, by, framebuffer, blue);
    lineOptimization4(cx, cy, bx, by, framebuffer, green);
    lineOptimization4(cx, cy, ax, ay, framebuffer, yellow);
    lineOptimization4(ax, ay, cx, cy, framebuffer, red);

    framebuffer.set(ax, ay, white);
    framebuffer.set(bx, by, white);
    framebuffer.set(cx, cy, white);

    framebuffer.write_tga_file("framebuffer.tga");
}