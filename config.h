#ifndef CONFIG_H
#define CONFIG_H

#define TAGMASK ((1 << TAGS_COUNT) - 1)
#define OPAQUE 0xffU

extern int                show_systray;      /* 是否显示系统托盘 */
extern const int          new_client_ahead;  /* 定义新窗口在栈顶还是栈底 */
extern const int          min_client_height; /* 最小窗口高度 */
extern const unsigned int border_px;         /* 窗口边框大小 */
extern const unsigned int gappx;             /* 窗口间隔 */
extern const unsigned int overview_gappx;    /* 预览窗口间隔 */
extern const unsigned int snap;              /* 边缘依附宽度 */
extern const int          showbar;           /* 是否显示状态栏 */
extern const int          topbar;            /* 指定状态栏位置 0底部 1顶部 */
extern const int          vertpad;           /* vertical padding of bar */
extern const int          sidepad;           /* horizontal padding of bar */
extern const int          user_bar_height;   /* bar的额外高度, 总高度为 字体高度 + user_bar_height */
extern const unsigned int systray_icon_size; /* 系统托盘图标尺寸 */
extern const unsigned int systray_pinning;   /* 托盘跟随的显示器 0代表不指定显示器 */
extern const unsigned int systray_spacing;   /* 系统托盘间距 */
extern const unsigned int systray_padding;   /* 托盘和状态栏的间隙 */
extern const int          systray_pinning_fail_first; /* 1：如果 pinning
                                                         失败，在第一台显示器上显示系统托盘，0：在最后一台显示器上显示系统托盘 */
extern const int   win_icon_size;                     /* 窗口图标尺寸 */
extern const int   win_icon_spacing;                  /* 窗口图标与窗口标题间的间距*/
extern const float mfact;                             /* 主工作区 大小比例 */
extern const int   nmaster;                           /* 主工作区 窗口数量 */
extern const int   resize_hints;                      /* 1 means respect size hints in tiled resizals */
extern const int   lock_full_screen;                  /* 强制焦点在全屏窗口上 */

extern const char *broken; /* 无法获取到窗口标题时显示文本 */

/* 自定义脚本位置 */
extern const char *dwmdir;     /* dwm目录 */
extern const char *localshare; /* .local/share */
extern const char *app_starter_sh;
extern const char *auto_start_script;
extern const char *status_bar_script;

/* 自定义 scratchpad instance */
extern const char scratchpadname[];

/* Lockfile */
extern const char lockfile[];

extern const unsigned int baralpha;    /* 状态栏透明度 */
extern const unsigned int borderalpha; /* 边框透明度 */

extern const char        *fonts[];
extern const char        *colors[][3];
extern const unsigned int alphas[][3];

/* 自定义tag名称 */
/* 自定义特定实例的显示状态 */
#define TAGS_COUNT 16
extern const char *tags[TAGS_COUNT];

extern const Rule rules[];

extern const char  *overviewtag;
extern const Layout overviewlayout;

/* 自定义布局 */
extern const Layout layouts[];
/* commands */
extern const Key keys[];

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
extern const Button buttons[];

#define ARRAY_ITEM_COUNT(X) int X##_count();

#define ARRAY_ITEM_COUNT_DEF(X) \
    int X##_count()             \
    {                           \
        return LENGTH(X);       \
    }

ARRAY_ITEM_COUNT(fonts)
ARRAY_ITEM_COUNT(colors)
ARRAY_ITEM_COUNT(alphas)
ARRAY_ITEM_COUNT(tags)
ARRAY_ITEM_COUNT(rules)
ARRAY_ITEM_COUNT(layouts)
ARRAY_ITEM_COUNT(keys)
ARRAY_ITEM_COUNT(buttons)

// int fonts_count();
// int colors_count();
// int alphas_count();
// int tags_count();
// int rules_count();
// int layouts_count();
// int keys_count();
// int buttons_count();
#endif  // CONFIG_H
