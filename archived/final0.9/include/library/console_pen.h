#ifndef CONSOLE_PEN_H
#define CONSOLE_PEN_H
// 定义颜色位域
#define BLINK       (1 << 7)     // 闪烁位
#define BG_RED      (1 << 6)    // 背景红色
#define BG_GREEN    (1 << 5)  // 背景绿色
#define BG_BLUE     (1 << 4)   // 背景蓝色
#define INTENSITY   (1 << 3) // 高亮位
#define FG_RED      (1 << 2)    // 前景红色
#define FG_GREEN    (1 << 1)  // 前景绿色
#define FG_BLUE     (1 << 0)   // 前景蓝色

// 定义颜色枚举
typedef enum {
    COLOR_BLACK = 0,
    COLOR_BLUE = FG_BLUE,
    COLOR_GREEN = FG_GREEN,
    COLOR_CYAN = FG_GREEN | FG_BLUE,
    COLOR_RED = FG_RED,
    COLOR_MAGENTA = FG_RED | FG_BLUE,
    COLOR_BROWN = FG_RED | FG_GREEN,
    COLOR_WHITE = FG_RED | FG_GREEN | FG_BLUE,
    COLOR_BRIGHT_BLUE = FG_BLUE | INTENSITY,
    COLOR_BRIGHT_GREEN = FG_GREEN | INTENSITY,
    COLOR_BRIGHT_CYAN = FG_GREEN | FG_BLUE | INTENSITY,
    COLOR_BRIGHT_RED = FG_RED | INTENSITY,
    COLOR_BRIGHT_MAGENTA = FG_RED | FG_BLUE | INTENSITY,
    COLOR_YELLOW = FG_RED | FG_GREEN | INTENSITY,
    COLOR_BRIGHT_WHITE = FG_RED | FG_GREEN | FG_BLUE | INTENSITY,
    COLOR_MAX
} Color;

#endif