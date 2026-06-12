#include "SDL.h"
#include "canvas.h"

int main(int argc, char* argv[]) {
    //初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "SDL初始化失败:" << SDL_GetError();
        return 1;
    }
    
    // 创建窗口
    int w = 800, h = 600;
    SDL_Window* window = SDL_CreateWindow(
        "三维变换",           // 窗口标题
        SDL_WINDOWPOS_CENTERED,  // 窗口x位置
        SDL_WINDOWPOS_CENTERED,  // 窗口y位置
        w,                     // 窗口宽度
        h,                     // 窗口高度
        SDL_WINDOW_SHOWN         // 窗口标志
    );
    if (!window) {
        std::cerr << "窗口创建失败: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // 创建渲染器
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );
    if (!renderer) {
        std::cerr << "渲染器创建失败: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    //创建画布
    Canvas canvas = Canvas(renderer, w, h);
    canvas.clear(100, 100, 100);
    
    //通过指定三个顶点坐标，定义一个三角形
    std::vector<Vec3f> vertices = {
        {-1.0f, 0.0f, -2.0f},
        {2.0f, 0.0f, -2.0f},
        {0.0f, 3.0f, -3.0f}
    }; 
    std::cout << "世界坐标：" << std::endl;
    for (int i = 0; i < vertices.size(); i++) {
        std::cout << vertices[i] << std::endl;
    }



    // 1. 世界坐标转为相机坐标
    // TODO:定义视图变换矩阵
    Matrix44f M_view;
    for (int i = 0; i < vertices.size(); i++) {

        M_view.multVecMatrix(vertices[i], vertices[i]);
    }
    std::cout << std::endl << "相机坐标：" << std::endl;
    for (int i = 0; i < vertices.size(); i++) {
        std::cout << vertices[i] << std::endl;
    }
    // 2. 相机坐标转为标准设备坐标
    // TODO:定义投影矩阵
    Matrix44f M_proj;
    for (int i = 0; i < vertices.size(); i++) {

        M_proj.multVecMatrix(vertices[i], vertices[i]);
    }
    std::cout << std::endl << "标准设备坐标：" << std::endl;
    for (int i = 0; i < vertices.size(); i++) {
        std::cout << vertices[i] << std::endl;
    }
    // 3. 标准设备坐标转为像素坐标     
    //三角形顶点的二维像素坐标
    std::vector<Vec2i> vertices_screen = {
        {50, 50},
        {100, 50},
        {80, 100}
    };
    // TODO:像素坐标
    std::cout << std::endl << "像素坐标：" << std::endl;
    for (int i = 0; i < vertices_screen.size(); i++) {
        std::cout << vertices_screen[i] << std::endl;
    }
    //绘制三角形    
    canvas.drawTriangles(vertices_screen, 255, 0, 0);

    // 主循环标志
    bool running = true;
    SDL_Event event;
    
    // 主循环
    while (running) {
        // 处理事件
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        //绘制画布
        canvas.render(renderer);

        // 添加短暂延迟以减少CPU使用率
        SDL_Delay(16); // 约60FPS
    }        
    
    //清理资源
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
