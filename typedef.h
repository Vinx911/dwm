#ifndef TYPEDEF_H
#define TYPEDEF_H

typedef struct
{
    Cursor cursor;
} Cur;

typedef struct Fnt
{
    Display     *dpy;      // 所属的显示器
    unsigned int h;        // 字体高度（字号大小）
    XftFont     *xfont;    // 指向X库中的xftFont结构体，是字体实现的核心
    FcPattern   *pattern;  // 模式
    struct Fnt  *next;     // 指向下一个字体
} Fnt;

enum {
    ColFg,
    ColBg,
    ColBorder,
}; /* Clr scheme index */
typedef XftColor Clr;

/**
 * 可绘制视窗的管理结构体
 */
typedef struct
{
    unsigned int w, h;    // 宽，高
    Display     *dpy;     // 所属显示器
    int          screen;  // 屏幕号
    Window       root;    // 根视窗
    Visual      *visual;  // visual
    unsigned int depth;   // 颜色位深
    Colormap     cmap;    // 颜色映射
    Drawable     drawable;
    Picture      picture;  // 窗口图标
    GC           gc;       // 图形上下文,存储前景色、背景色、线条样式等
    Clr         *scheme;   // 配色
    Fnt         *fonts;    // 字体
} Drw;

/* enums */
enum {
    Manager,
    Xembed,
    XembedInfo,
    XLast
}; /* Xembed atoms */
enum {
    CurNormal,
    CurResize,
    CurMove,
    CurLast
}; /* cursor */
enum {
    SchemeNorm,       // 普通
    SchemeSel,        // 选中的
    SchemeSelGlobal,  // 全局并选中的
    SchemeHid,        // 隐藏的
    SchemeSystray,    // 托盘
    SchemeNormTag,    // 普通标签
    SchemeSelTag,     // 选中的标签
    SchemeUnderline,  // 下划线
    SchemeBarEmpty,   // 状态栏空白部分
    SchemeStatusText  // 状态栏文本
};                    /* color schemes */
enum {
    NetSupported,
    NetWMIcon,
    NetSystemTray,
    NetSystemTrayOP,
    NetSystemTrayOrientation,
    NetSystemTrayVisual,
    NetWMName,
    NetWMState,
    NetWMFullscreen,
    NetActiveWindow,
    NetWMWindowType,
    NetWMWindowTypeDock,
    NetSystemTrayOrientationHorz,
    NetWMWindowTypeDialog,
    NetClientList,
    NetWMCheck,
    NetLast
}; /* EWMH atoms */
enum {
    WMProtocols,
    WMDelete,
    WMState,
    WMTakeFocus,
    WMLast
}; /* default atoms */

enum {
    ClkTagBar,
    ClkLtSymbol,
    ClkStatusText,
    ClkWinTitle,
    ClkClientWin,
    ClkRootWin,
    ClkLast
}; /* clicks */

typedef struct
{
    int          i;
    unsigned int ui;
    float        f;
    const void  *v;
} Arg;

typedef struct
{
    unsigned int click;
    unsigned int mask;
    unsigned int button;
    void (*func)(const Arg *arg);
    const Arg arg;
} Button;

typedef struct Monitor Monitor;
typedef struct Client  Client;
struct Client
{
    char         name[256]; /* 窗口标题 */
    float        mina, maxa;
    int          x, y, w, h;             /* 位置尺寸 */
    int          oldx, oldy, oldw, oldh; /* 旧位置尺寸 */
    int          basew, baseh;           /* 基本尺寸 */
    int          incw, inch;             /* 指定调整大小增量 */
    int          maxw, maxh, minw, minh; /* 最大最小尺寸 */
    int          hintsvalid;             /* 尺寸hint有效 */
    int          bw, oldbw;              /* 边框宽度 */
    unsigned int tags;                   /* 所属tag */
    int          isfixed;                /* 固定大小 */
    int          isfloating;             /* 浮动的 */
    int          isurgent;               /* 紧急性 */
    int          neverfocus;             /* 永远没有焦点 */
    int          oldstate;               /* 旧状态 */
    int          isfullscreen;           /* 全屏的 */
    int          isfakefullscreen;       /* 伪全屏的 */
    int          nooverview;             /*不参与预览 */
    unsigned int icw, ich;
    Picture      icon;  /* 客户端窗口图标 */
    Client      *next;  /* 列表下一个客户端 */
    Client      *snext; /* 栈中下一个客户端 */
    Monitor     *mon;   /* 所属监视器 */
    Window       win;   /* 客户端窗口 */
};

typedef struct
{
    unsigned int mod;
    KeySym       keysym;
    void (*func)(const Arg *);
    const Arg arg;
} Key;

typedef struct
{
    const char *symbol;
    void (*arrange)(Monitor *);
} Layout;

typedef struct
{
    unsigned int curtag, prevtag; /* current and previous tag */
    struct LayoutConfig
    {
        int           nmasters;  /* number of windows in master area */
        float         mfacts;    /* mfacts per tag */
        unsigned int  sellts;    /* selected layouts */
        const Layout *ltidxs[2]; /* matrix of tags and layouts indexes  */
        int           showbars;  /* display bar for the current tag */
    } *layout;
} Pertag;

struct Monitor
{
    char          ltsymbol[16]; /* 布局指示符号 */
    float         mfact;        /* 主区域大小的因子 */
    int           nmaster;
    int           num;
    int           by;             /* bar geometry */
    int           task_bar_width; /* width of tasks portion of bar */
    int           task_count;     /* number of tasks */
    int           mx, my, mw, mh; /* 屏幕尺寸 */
    int           wx, wy, ww, wh; /* 窗口区域的尺寸  */
    int           gappx;          /* 窗口间的间距 */
    unsigned int  seltags;        /* 当前使用的标签集索引 */
    unsigned int  sellt;          /* 选中的布局 */
    unsigned int  tagset[2];      /* 标签集,记录最近两次选中的tag */
    int           showbar;        /* 显示Bar */
    int           topbar;         /* bar在顶部显示 */
    int           hidsel;         /* 隐藏窗口选中, *临时显示* */
    Client       *clients;        /* 客户端列表 */
    Client       *select;         /* 当前选择的客户端 */
    Client       *stack;          /* 客户端的栈 */
    Monitor      *next;           /* 下一个监视器 */
    Window        bar_window;     /* Bar的窗口 */
    const Layout *lt[2];          /* 布局 */
    Pertag       *pertag;
    uint          is_overview;
};

typedef struct
{
    const char *class;
    const char  *instance;
    const char  *title;
    unsigned int tags;
    int          isfloating;
    int          noborder;
    int          nooverview;
    int          isfakefullscreen;
    int          monitor;
    uint         floatposition;
} Rule;

typedef struct Systray Systray;
struct Systray
{
    Window  win;
    Client *icons;
};

#endif  // TYPEDEF_H