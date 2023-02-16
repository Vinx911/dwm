#ifndef DWM_H
#define DWM_H

#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
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

#include "layout.h"
#include "tag.h"
#include "window.h"
#include "monitor.h"
#include "client.h"
#include "bar.h"
#include "button_press.h"
#include "status_bar.h"
#include "systray.h"

/* macros */
#define BUTTONMASK (ButtonPressMask | ButtonReleaseMask)
#define CLEANMASK(mask)               \
    (mask & ~(numlockmask | LockMask) \
     & (ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask))
#define INTERSECT(x, y, w, h, m)                                   \
    (MAX(0, MIN((x) + (w), (m)->wx + (m)->ww) - MAX((x), (m)->wx)) \
     * MAX(0, MIN((y) + (h), (m)->wy + (m)->wh) - MAX((y), (m)->wy)))
#define ISVISIBLE(C) (((C->mon->is_overview && !C->nooverview) || C->tags & C->mon->tagset[C->mon->seltags]))
#define HIDDEN(C) ((getstate(C->win) == IconicState))
#define LENGTH(X) (sizeof X / sizeof X[0])
#define MOUSEMASK (BUTTONMASK | PointerMotionMask)
#define WIDTH(X) ((X)->w + 2 * (X)->bw)
#define HEIGHT(X) ((X)->h + 2 * (X)->bw)

#define TEXTW(X) (drw_fontset_getwidth(drw, (X)) + lrpad)

#define TAGMASK ((1 << TAGS_COUNT) - 1)
#define OPAQUE 0xffU

#define SYSTEM_TRAY_REQUEST_DOCK 0
#define _NET_SYSTEM_TRAY_ORIENTATION_HORZ 0

/* XEMBED messages */
#define XEMBED_EMBEDDED_NOTIFY 0
#define XEMBED_WINDOW_ACTIVATE 1
#define XEMBED_FOCUS_IN 4
#define XEMBED_MODALITY_ON 10

#define XEMBED_MAPPED (1 << 0)
#define XEMBED_WINDOW_ACTIVATE 1
#define XEMBED_WINDOW_DEACTIVATE 2

#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define XEMBED_EMBEDDED_VERSION (VERSION_MAJOR << 16) | VERSION_MINOR

void clientmessage(XEvent *e);
void configurenotify(XEvent *e);
void configurerequest(XEvent *e);
void destroynotify(XEvent *e);
void enternotify(XEvent *e);
void expose(XEvent *e);
void focus(Client *c);
void focusin(XEvent *e);
void focusmon(const Arg *arg);
void focusstackvis(const Arg *arg);
void focusstackhid(const Arg *arg);
void focusstack(int inc, int vis);
int getrootptr(int *x, int *y);
void grabbuttons(Client *c, int focused);
void incnmaster(const Arg *arg);
void keypress(XEvent *e);
void mappingnotify(XEvent *e);
void maprequest(XEvent *e);
void motionnotify(XEvent *e);
void pointerfocuswin(Client *c);
void propertynotify(XEvent *e);
void quit(const Arg *arg);
void resizerequest(XEvent *e);
int sendevent(Window w, Atom proto, int m, long d0, long d1, long d2, long d3, long d4);
void setgaps(const Arg *arg);
void setmfact(const Arg *arg);
void seturgent(Client *c, int urg);
void sigchld(int unused);
void spawn(const Arg *arg);
void toggleallfloating(const Arg *arg);
void togglefloating(const Arg *arg);
void togglescratch(const Arg *arg);
void unfocus(Client *c, int setfocus);
void unmanage(Client *c, int destroyed);
void unmapnotify(XEvent *e);
void updatenumlockmask(void);
int  xerror(Display *dpy, XErrorEvent *ee);
int  xerrordummy(Display *dpy, XErrorEvent *ee);
int  xerrorstart(Display *dpy, XErrorEvent *ee);
void xinitvisual();
void zoom(const Arg *arg);


#include "config.h"

extern const char autostartblocksh[];
extern const char autostartsh[];
extern const char broken[];          /* 无法获取到窗口标题时显示文本 */
extern const char dwmdir[];          /* dwm目录 */
extern const char localshare[];      /* .local/share */
extern char       status_text[1024]; /* 状态栏文本 */
extern int        screen;            /* 默认屏幕 */
extern int        screen_width;      /* 默认屏幕的宽 */
extern int        screen_height;     /* 默认屏幕的高 */
extern int        bar_height;        /* bar 高度 */
extern int        tag_bar_width;     /* 标签栏宽度 */
extern int        lt_symbol_width;   /* 布局符号宽度 */
extern int        statsu_bar_width;  /* 状态栏宽度 */
extern int        lrpad;             /* 文本左右填充的总和 */
extern int        vp;                /* vertical padding for bar */
extern int        sp;                /* side padding for bar */
extern int (*xerrorxlib)(Display *, XErrorEvent *);
extern unsigned int numlockmask;      /* 数字键盘锁按键掩码 */
extern unsigned int monocleshowcount; /* monocle显示窗口个数 */
extern void (*handler[LASTEvent])(XEvent *);

extern Atom     wmatom[WMLast], netatom[NetLast], xatom[XLast];
extern int      running;
extern Cur     *cursor[CurLast]; /* 光标 */
extern Clr    **scheme;          /* 配色 */
extern Display *dpy;             /* 默认显示器 */
extern Drw     *drw;
extern Monitor *mons;           /* 监视器列表 */
extern Monitor *select_monitor; /* 当前选择的监视器 */
extern Window   root;           /* 根窗口 */
extern Window   wmcheckwin;

extern int      useargb; /* 使用argb */
extern Visual  *visual;  /* 视觉 */
extern int      depth;   /* 颜色位深 */
extern Colormap cmap;    /* 颜色映射 */



#endif  // DWM_H