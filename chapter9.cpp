#include "our_gl.h"
#include "model.h"

extern mat<4,4> ModelView, Perspective;

struct C9PhongShader : IShader 
{
    const Model &model;
    vec4 l;
    vec2  varying_uv[3]; 
    vec4 varying_nrm[3]; 
    vec4 tri[3];         

    C9PhongShader(const vec3 light, const Model &m) : model(m)
    {
        l = normalized((ModelView*vec4{light.x, light.y, light.z, 0.}));
    }

    virtual vec4 vertex(const int face, const int vert)
    {
        varying_uv[vert]  = model.uv(face, vert);
        varying_nrm[vert] = ModelView.invert_transpose() * model.normal(face, vert);
        vec4 gl_Position = ModelView * model.vert(face, vert);
        tri[vert] = gl_Position;
        return Perspective * gl_Position;   
    }

    virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const 
    {
        //TGAColor gl_FragColor = {255, 255, 255, 255};
        //vec3 n = normalized(cross(tri[1].xyz()-tri[0].xyz(), tri[2].xyz()-tri[0].xyz()));
        //vec3 n = normalized(tri_n[0].xyz() * bar[0] + tri_n[1].xyz() * bar[1] + tri_n[2].xyz() * bar[2]); //法线插值，使三角面显得平滑
        //vec2 uv = t_uv[0] * bar[0] + t_uv[1] * bar[1] + t_uv[2] * bar[2];
        //TGAColor c = model.diffuse().get(uv[0]*model.diffuse().width(), uv[1] * model.diffuse().height());
        //vec4 tc = normalized(vec4{(double)c[2],(double)c[1],(double)c[0],0}*2./255. - vec4{1,1,1,0});
        //gl_FragColor = {uint8_t(tc.x), uint8_t(tc.y), uint8_t(tc.z), uint8_t(tc.w)};
        //gl_FragColor = c;
        mat<2,4> E = { tri[1]-tri[0], tri[2]-tri[0] };
        mat<2,2> U = { varying_uv[1]-varying_uv[0], varying_uv[2]-varying_uv[0] };
        mat<2,4> T = U.invert() * E;
        mat<4,4> D = {normalized(T[0]),  
                      normalized(T[1]),  
                      normalized(varying_nrm[0]*bar[0] + varying_nrm[1]*bar[1] + varying_nrm[2]*bar[2]), 
                      {0,0,0,1}}; // Darboux frame
        vec2 uv = varying_uv[0] * bar[0] + varying_uv[1] * bar[1] + varying_uv[2] * bar[2];
        vec4 n = normalized(D.transpose() * model.normal(uv));
        vec4 r = normalized(n * (n * l)*2 - l);                   
        double ambient  = .4;                                     
        double diffuse  = 1.*std::max(0., n * l);                 
        double specular = (3.*sample2D(model.specular(), uv)[0]/255.) * std::pow(std::max(r.z, 0.), 35);  
        TGAColor gl_FragColor = sample2D(model.diffuse(), uv);
        for (int channel : {0,1,2})
            gl_FragColor[channel] = std::min<int>(255, gl_FragColor[channel]*(ambient + diffuse + specular));
        return {false, gl_FragColor};          
    }
};


void chapter9Func() 
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
    C9PhongShader shader(light, model);
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