#include <am.h>
#include <klib.h>
#include <klib-macros.h>

static uint32_t target_colors[] = {
    0x000000, 0xff0000, 0x00ff00, 0x0000ff,
    0xffff00, 0xff00ff, 0x00ffff, 0xffffff
};
#define COLOR_COUNT (sizeof(target_colors) / sizeof(target_colors[0]))

#define R(p) (((p) >> 16) & 0xff)
#define G(p) (((p) >> 8) & 0xff)
#define B(p) ((p) & 0xff)

static inline uint32_t make_pixel(uint8_t r, uint8_t g, uint8_t b) {
    return (r << 16) | (g << 8) | b;
}

// 修正 1: 使用更安全的静态缓冲区，防止不同分辨率下的溢出
static uint32_t pixels[1024 * 768];

void draw_full_screen(uint32_t color, int w, int h) {
    for (int i = 0; i < w * h; i++) {
        pixels[i] = color;
    }
    io_write(AM_GPU_FBDRAW, 0, 0, pixels, w, h, true);
}

int main() {
    ioe_init();

    AM_GPU_CONFIG_T config = io_read(AM_GPU_CONFIG);
    int w = config.width;
    int h = config.height;

    uint32_t cur_c = target_colors[0];
    int target_idx = 1;
    int k = 100; // 渐变步数
    int i = 0;
    bool fast_mode = false;
    unsigned long last_time = 0;

    while (1) {
        // 处理按键
        AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
        if (ev.keycode != AM_KEY_NONE) {
            if (ev.keycode == AM_KEY_ESCAPE && ev.keydown) halt(0);
            fast_mode = ev.keydown;
        }

        unsigned long now = io_read(AM_TIMER_UPTIME).us / 1000;
        int interval = fast_mode ? 4 : 20;
        if (now - last_time < interval) continue;
        last_time = now;

        uint32_t next_c = target_colors[target_idx];

        // 修正 2: 显式转换为 int 类型进行有符号减法运算，避免无符号溢出
        int r0 = R(cur_c), g0 = G(cur_c), b0 = B(cur_c);
        int rk = R(next_c), gk = G(next_c), bk = B(next_c);

        uint8_t r = r0 + (rk - r0) * i / k;
        uint8_t g = g0 + (gk - g0) * i / k;
        uint8_t b = b0 + (bk - b0) * i / k;

        draw_full_screen(make_pixel(r, g, b), w, h);

        i++;
        if (i > k) {
            i = 0;
            cur_c = next_c; // 这一轮结束，当前色变为目标色
            target_idx = (target_idx + 1) % COLOR_COUNT; // 指向下一个目标
        }
    }
    return 0;
}
//参考资料：
//1. AM 设备模型文档
//2. keyboard.c, video.c, rtc.c 示例代码
//本人已理解原理,绘制与按钮应当独立
