#include <SDL.h>
#include <cmath>
#include <iostream>

static inline float RAD(float deg) { return deg * M_PI / 180.0f; }

// 将点 (x,y) 先相对于 pivot 做缩放、旋转，再平移 tx,ty
void transform_about(float x, float y,
                     float pivotx, float pivoty,
                     float scale_x, float scale_y,
                     float angle_deg,
                     float tx, float ty,
                     float &outx, float &outy)
{
    // 移到 pivot 原点
    float X = x - pivotx;
    float Y = y - pivoty;
    // 缩放
    X *= scale_x;
    Y *= scale_y;
    // 旋转
    float c = cosf(RAD(angle_deg));
    float s = sinf(RAD(angle_deg));
    float RX = X * c - Y * s;
    float RY = X * s + Y * c;
    // 移回并平移
    outx = RX + pivotx + tx;
    outy = RY + pivoty + ty;
}

// 参数化绘制椭圆（中心在 ox,oy，长短轴 a,b），并在绘制时应用 transform_about
void drawEllipseParam(SDL_Renderer *rend,
                      float ox, float oy, float a, float b,
                      float pivotx, float pivoty,
                      float sx, float sy, float angle_deg,
                      float tx, float ty,
                      SDL_Color color)
{
    SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, 255);
    const float step = 0.01f; // 角度步长 (rad)，步长越小曲线越平滑
    for (float t = 0; t <= 2.0f * M_PI + step; t += step)
    {
        float x = ox + a * cosf(t);
        float y = oy + b * sinf(t);
        float txp, typ;
        transform_about(x, y, pivotx, pivoty, sx, sy, angle_deg, tx, ty, txp, typ);
        SDL_RenderDrawPoint(rend, int(std::round(txp)), int(std::round(typ)));
    }
}

// 绘制圆（可复用 drawEllipseParam 以 a=b）
void drawCircleParam(SDL_Renderer *rend,
                     float ox, float oy, float r,
                     float pivotx, float pivoty,
                     float sx, float sy, float angle_deg,
                     float tx, float ty,
                     SDL_Color color)
{
    drawEllipseParam(rend, ox, oy, r, r, pivotx, pivoty, sx, sy, angle_deg, tx, ty, color);
}

// 绘制抛物线：y = k * x^2（在范围 [-rx, rx]），样点按 x 采样并变换后绘制
void drawParabolaParam(SDL_Renderer *rend,
                       float ox, float oy, float rx, float k,
                       float pivotx, float pivoty,
                       float sx, float sy, float angle_deg,
                       float tx, float ty,
                       SDL_Color color)
{
    SDL_SetRenderDrawColor(rend, color.r, color.g, color.b, 255);
    const float step = 0.5f; // 采样步长，减小可更平滑但开销更大
    bool first = true;
    float prev_x = 0.0f, prev_y = 0.0f;

    for (float t = -rx; t <= rx; t += step)
    {
        // 在局部坐标系计算抛物线点 (以 ox,oy 为抛物线原点)
        double local_x = t;
        double local_y = k * t * t;

        // 局部点在世界坐标
        double wx = ox + local_x;
        double wy = oy + local_y;

        // 对点执行相对于 pivot 的变换（与 transform_about 保持一致的顺序）
        double X = wx - pivotx;
        double Y = wy - pivoty;
        // 缩放
        X *= sx;
        Y *= sy;
        // 旋转
        double c = cos(angle_deg * M_PI / 180.0);
        double s = sin(angle_deg * M_PI / 180.0);
        double RX = X * c - Y * s;
        double RY = X * s + Y * c;
        // 移回并加全局平移
        double outx = RX + pivotx + tx;
        double outy = RY + pivoty + ty;

        // 连线绘制以避免“断裂”和取整导致的错位
        if (first)
        {
            prev_x = outx;
            prev_y = outy;
            first = false;
        }
        else
        {
            SDL_RenderDrawLine(rend,
                               int(std::round(prev_x)), int(std::round(prev_y)),
                               int(std::round(outx)), int(std::round(outy)));
            prev_x = outx;
            prev_y = outy;
        }
    }
}

int main(int argc, char **argv)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::cerr << "SDL_Init error\n";
        return -1;
    }
    int w = 800, h = 600;
    SDL_Window *win = SDL_CreateWindow("Shape Transform Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, 0);
    SDL_Renderer *rend = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    // 形状原始参数（以投影面像素坐标为单位）
    float ell_cx = 400, ell_cy = 300, a = 300, b = 220;
    // 两个内圆
    float c1x = ell_cx - 100, c1y = ell_cy - 40, cr = 30;
    float c2x = ell_cx + 100, c2y = ell_cy - 40;
    // 抛物线参数（嘴巴）
    float para_ox = ell_cx;      // 以椭圆中心为水平中心
    float para_rx = 140;         // 横向半宽，减少使嘴更窄
    float para_k = -0.0024f;     // 取负使开口向下，调整绝对值控制深浅
    float para_oy = ell_cy + 120; // 抛物线垂直位置（下移到脸下部）
    // 全局变换参数（可用交互改变）
    float tx = 0, ty = 0;
    float angle = 0;
    float sx = 1.0f, sy = 1.0f;
    // 变换基点（默认以椭圆中心为基点）
    float pivotx = ell_cx, pivoty = ell_cy;

    bool quit = false;
    SDL_Event e;
    Uint32 lastTicks = SDL_GetTicks();
    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                quit = true;
            else if (e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                case SDLK_LEFT:
                    tx -= 5;
                    break;
                case SDLK_RIGHT:
                    tx += 5;
                    break;
                case SDLK_UP:
                    ty -= 5;
                    break;
                case SDLK_DOWN:
                    ty += 5;
                    break;
                case SDLK_q:
                    angle -= 2.0f;
                    break; // 逆时针
                case SDLK_e:
                    angle += 2.0f;
                    break; // 顺时针
                case SDLK_w:
                    sx *= 1.02f;
                    sy *= 1.02f;
                    break; // 放大
                case SDLK_s:
                    sx *= 0.98f;
                    sy *= 0.98f;
                    break; // 缩小
                case SDLK_a:
                    pivotx -= 5;
                    break; // 移动变换基点（演示任意点）
                case SDLK_d:
                    pivotx += 5;
                    break;
                case SDLK_z:
                    pivoty -= 5;
                    break;
                case SDLK_x:
                    pivoty += 5;
                    break;
                case SDLK_r: // reset
                    tx = ty = 0;
                    angle = 0;
                    sx = sy = 1;
                    pivotx = ell_cx;
                    pivoty = ell_cy;
                    break;
                }
            }
            else if (e.type == SDL_MOUSEWHEEL)
            {
                if (e.wheel.y > 0)
                {
                    sx *= 1.05f;
                    sy *= 1.05f;
                }
                else if (e.wheel.y < 0)
                {
                    sx *= 0.95f;
                    sy *= 0.95f;
                }
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                if (e.button.button == SDL_BUTTON_LEFT)
                {
                    // 把当前鼠标设为变换基点
                    pivotx = (float)e.button.x;
                    pivoty = (float)e.button.y;
                }
            }
        }

        // 渲染
        SDL_SetRenderDrawColor(rend, 200, 200, 200, 255);
        SDL_RenderClear(rend);

        // 椭圆外边框
        drawEllipseParam(rend, ell_cx, ell_cy, a, b, pivotx, pivoty, sx, sy, angle, tx, ty, {0, 0, 0, 255});
        // 两个眼睛（圆）
        drawCircleParam(rend, c1x, c1y, cr, pivotx, pivoty, sx, sy, angle, tx, ty, {0, 0, 0, 255});
        drawCircleParam(rend, c2x, c2y, cr, pivotx, pivoty, sx, sy, angle, tx, ty, {0, 0, 0, 255});
        // 抛物线（嘴）——使用 para_oy 作为垂直位置
        drawParabolaParam(rend, para_ox, para_oy, para_rx, para_k, pivotx, pivoty, sx, sy, angle, tx, ty, {0, 0, 0, 255});

        // 显示一个小圆标注 pivot
        SDL_SetRenderDrawColor(rend, 255, 0, 0, 255);
        for (int dx = -3; dx <= 3; ++dx)
            for (int dy = -3; dy <= 3; ++dy)
                SDL_RenderDrawPoint(rend, int(std::round(pivotx)) + dx, int(std::round(pivoty)) + dy);

        SDL_RenderPresent(rend);

        // 保持简单定帧
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}