#ifndef DWM_H
#define DWM_H

#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifdef XINERAMA
    #include <X11/extensions/Xinerama.h>
#endif /* XINERAMA */

#include "typedef.h"
#include "drw.h"
#include "util.h"

#define LENGTH(X) (sizeof X / sizeof X[0])
#define TEXTW(X) (drw_fontset_getwidth(drw, (X)) + text_lr_pad)

extern char        *dwm_script_path;   /* dwm脚本路径 */
extern char         status_text[2048]; /* 状态栏文本 */
extern int          screen;            /* 默认屏幕 */
extern int          screen_width;      /* 默认屏幕的宽 */
extern int          screen_height;     /* 默认屏幕的高 */
extern int          bar_height;        /* bar 高度 */
extern int          tag_bar_width;     /* 标签栏宽度 */
extern int          lt_symbol_width;   /* 布局符号宽度 */
extern int          statsu_bar_width;  /* 状态栏宽度 */
extern int          text_lr_pad;       /* 文本左右填充的总和 */
extern int          bar_ver_padding;   /* vertical padding for bar */
extern int          bar_side_padding;  /* side padding for bar */
extern unsigned int numlockmask;       /* 数字键盘锁按键掩码 */

extern Atom     wmatom[WMLast], netatom[NetLast], xatom[XLast];
extern Cur     *cursor[CurLast]; /* 光标 */
extern Clr    **scheme;          /* 配色 */
extern Display *display;         /* 默认显示器 */
extern Drw     *drw;
extern Monitor *monitor_list;   /* 监视器列表 */
extern Monitor *select_monitor; /* 当前选择的监视器 */
extern Window   root_window;    /* 根窗口 */

extern int      useargb;   /* 使用argb */
extern Visual  *visual;    /* 视觉 */
extern int      depth;     /* 颜色位深 */
extern Colormap color_map; /* 颜色映射 */

extern unsigned int scratchtag;

extern void (*handler[LASTEvent])(XEvent *);

/**
 * 更新数字键盘锁按键掩码
 */
void update_numlock_mask(void);

int xerror(Display *dpy, XErrorEvent *ee);
int xerrordummy(Display *dpy, XErrorEvent *ee);

/**
 * 运行app
 */
void app_starter(const Arg *arg);

/**
 * 执行命令
 */
void spawn(const Arg *arg);

/**
 * 退出dwm
 */
void quit(const Arg *arg);

/**
 * 切换便签薄
 */
void toggle_scratch(const Arg *arg);

#endif  // DWM_H
