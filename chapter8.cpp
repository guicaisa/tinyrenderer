#include "our_gl.h"
#include "model.h"

extern mat<4,4> ModelView, Perspective;

struct C8PhongShader : IShader 
{
    const Model &model;
    vec4 l;
    vec4 tri[3];
    vec4 tri_n[3];
    vec2 t_uv[3];

    C8PhongShader(const vec3 light, const Model &m) : model(m)
    {
        l = normalized((ModelView*vec4{light.x, light.y, light.z, 0.}));
    }

    virtual vec4 vertex(const int face, const int vert)
    {
        vec4 v = model.vert(face, vert);
        vec4 gl_Position = ModelView * v;
        tri[vert] = gl_Position;
        vec4 n = model.normal(face, vert);
        tri_n[vert] = ModelView.invert_transpose() * n; //转换后的顶点的法线向量的计算方式为视图矩阵的逆+转置*原始的法线向量
        t_uv[vert] = model.uv(face, vert);
        return Perspective * gl_Position;
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const 
    {
        TGAColor gl_FragColor = {255, 255, 255, 255};
        //vec3 n = normalized(cross(tri[1].xyz()-tri[0].xyz(), tri[2].xyz()-tri[0].xyz()));
        //vec3 n = normalized(tri_n[0].xyz() * bar[0] + tri_n[1].xyz() * bar[1] + tri_n[2].xyz() * bar[2]); //法线插值，使三角面显得平滑
        vec2 uv = t_uv[0] * bar[0] + t_uv[1] * bar[1] + t_uv[2] * bar[2];
        TGAColor c = model.diffuse().get(uv[0]*model.diffuse().width(), uv[1] * model.diffuse().height());
        //vec4 tc = normalized(vec4{(double)c[2],(double)c[1],(double)c[0],0}*2./255. - vec4{1,1,1,0});
        //gl_FragColor = {uint8_t(tc.x), uint8_t(tc.y), uint8_t(tc.z), uint8_t(tc.w)};
        gl_FragColor = c;
        vec4 n = normalized(ModelView.invert_transpose() * model.normal(uv));
        vec4 r = normalized(n * (n * l) * 2 - l);
        double ambient = .3;
        double diff = std::max(0., n * l);
        double spec = std::pow(std::max(r.z, 0.), 35);
        for (int channel : {0,1,2})
        {
            gl_FragColor[channel] *= std::min(1., ambient + .4*diff + .9*spec);
        }
        return {false, gl_FragColor};
    }
};

void chapter8Func() 
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
    C8PhongShader shader(light, model);
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