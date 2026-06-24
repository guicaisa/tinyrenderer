#include "our_gl.h"
#include "model.h"

extern mat<4,4> ModelView, Perspective;

struct TempPhongShader : IShader 
{
    const Model &model;
    vec4 l;
    vec4 tri[3];

    TempPhongShader(const vec3 light, const Model &m) : model(m)
    {
        l = normalized((ModelView*vec4{light.x, light.y, light.z, 0.}));
    }

    virtual vec4 vertex(const int face, const int vert)
    {
        vec4 v = model.vert(face, vert);
        vec4 gl_Position = ModelView * v;
        tri[vert] = gl_Position;
        return Perspective * gl_Position;
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const 
    {
        TGAColor gl_FragColor = {255, 255, 255, 255};
        vec3 n = normalized(cross(tri[1].xyz()-tri[0].xyz(), tri[2].xyz()-tri[0].xyz()));
        vec3 r = normalized(n * (n * l.xyz()) * 2 - l.xyz());
        double ambient = .3;
        double diff = std::max(0., n * l.xyz());
        double spec = std::pow(std::max(r.z, 0.), 35);
        for (int channel : {0,1,2})
        {
            gl_FragColor[channel] *= std::min(1., ambient + .4*diff + .9*spec);
        }
        return {false, gl_FragColor};
    }
};


void chapter7Func() 
{
    constexpr int width = 800;
    constexpr int height = 800;
    constexpr vec3 light{1, 1, 1}; //光源方向
    constexpr vec3 eye{-1, 0, 2}; //相机位置
    constexpr vec3 center{0, 0, 0}; //相机方向
    constexpr vec3 up{0,1,1}; //相机上方方向

    lookat(eye, center, up); //模型视图矩阵
    init_perspective(norm(eye-center)); //正交矩阵
    init_viewport(width/16, height/16, width*7/8, height*7/8); //视口矩阵
    init_zbuffer(width, height);
    TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});
    Model model("obj\\african_head\\african_head.obj");
    TempPhongShader shader(light, model);
    for (int f = 0; f < model.nfaces(); ++f)
    {
        Triangle clip = {
            shader.vertex(f, 0),
            shader.vertex(f, 1),
            shader.vertex(f, 2)
        };
        rasterize(clip, shader, framebuffer);
    }

    framebuffer.write_tga_file("framebuffer.tga");
}