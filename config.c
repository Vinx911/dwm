
#include "dwm.h"
#include "layout.h"
#include "tag.h"
#include "window.h"
#include "monitor.h"
#include "client.h"
#include "bar.h"
#include "status_bar.h"
#include "systray.h"
#include "config.h"

int                show_systray      = 1;   /* 是否显示系统托盘 */
const int          new_client_ahead  = 0;   /* 定义新窗口在栈顶还是栈底 */
const int          min_client_height = 100; /* 最小窗口高度 */
const unsigned int border_px         = 1;   /* 窗口边框大小 */
const unsigned int gappx             = 5;   /* 窗口间隔 */
const unsigned int overview_gappx    = 5;   /* 预览窗口间隔 */
const unsigned int snap              = 32;  /* 边缘依附宽度 */
const int          showbar           = 1;   /* 是否显示状态栏 */
const int          topbar            = 1;   /* 指定状态栏位置 0底部 1顶部 */
const int          vertpad           = 10;  /* vertical padding of bar */
const int          sidepad           = 10;  /* horizontal padding of bar */
const int          user_bar_height   = 8;   /* bar的额外高度, 总高度为 字体高度 + user_bar_height */
const unsigned int systray_icon_size = 20;  /* 系统托盘图标尺寸 */
const unsigned int systray_pinning   = 1;   /* 托盘跟随的显示器 0代表不指定显示器 */
const unsigned int systray_spacing   = 10;  /* 系统托盘间距 */
const unsigned int systray_padding   = 2;   /* 托盘和状态栏的间隙 */
const int systray_pinning_fail_first = 1; /* 1：pinning 失败，在第一台显示器上显示，0：在最后一台显示*/
const int   win_icon_size    = 16;        /* 窗口图标尺寸 */
const int   win_icon_spacing = 5;         /* 窗口图标与窗口标题间的间距*/
const float mfact            = 0.55; /* 主工作区 大小比例 */
const int   nmaster          = 1;    /* 主工作区 窗口数量 */
const int   resize_hints     = 1;    /* 1 means respect size hints in tiled resizals */
const int   lock_full_screen = 0;    /* 强制焦点在全屏窗口上 */

const char *broken = "broken"; /* 无法获取到窗口标题时显示文本 */

/* 自定义脚本位置 */
const char *app_starter_sh    = "$DWM_PATH/appstarter.sh";
const char *auto_start_script = "$DWM_PATH/autostart.sh";
const char *status_bar_script = "$DWM_PATH/statusbar/statusbar.sh";

/* 自定义 scratchpad instance */
const char scratchpadname[] = {"scratchpad"};

/* Lockfile */
const char lockfile[] = "/tmp/dwm.lock";

const unsigned int baralpha    = 0xa0;   /* 状态栏透明度 */
const unsigned int borderalpha = OPAQUE; /* 边框透明度 */

const char *fonts[] = {"JetBrainsMono Nerd Font:style=medium:size=13", "monospace:size=13"};

// clang-format off
const char *colors[][3] = {
    /*                       fg         bg         border   */
    [SchemeNorm]      = {"#bbbbbb", "#333333",  "#444444"},
    [SchemeSel]       = {"#ffffff", "#37474F",  "#42A5F5"},
    [SchemeSelGlobal] = {"#ffffff", "#37474F",  "#FFC0CB"},
    [SchemeHid]       = {"#dddddd", NULL,       NULL},
    [SchemeSystray]   = {NULL,      "#7799AA",  NULL},
    [SchemeUnderline] = {"#7799AA", NULL,       NULL},
    [SchemeNormTag]   = {"#bbbbbb", "#333333",  NULL},
    [SchemeSelTag]    = {"#eeeeee", "#333333",  NULL},
    [SchemeBarEmpty]  = {NULL,      "#111111",  NULL},
};
// clang-format on
// clang-format off
const unsigned int alphas[][3] = {
    /*               fg      bg        border     */
    [SchemeNorm]        = { OPAQUE, baralpha, borderalpha },
    [SchemeSel]         = { OPAQUE, baralpha, borderalpha },
    [SchemeSelGlobal]   = { OPAQUE, baralpha, borderalpha },
    [SchemeNormTag]     = { OPAQUE, baralpha, borderalpha }, 
    [SchemeSelTag]      = { OPAQUE, baralpha, borderalpha },
    [SchemeBarEmpty]    = { 0x00U,  0xa0a,    0x00U },
    [SchemeStatusText]  = { OPAQUE, 0x88,     0x00U },
};
// clang-format on

/* 自定义tag名称 */
/* 自定义特定实例的显示状态 */
const char *tags[16] = {"󰎤", "󰎧", "󰎪", "󰎭", "󰎱", "󰎳", "󰎶", "󰎹",
                        "󰎼", "", "", "", "󰝚", "", "󰘑", ""};

// clang-format off
const Rule rules[] = {
    /* class                 instance              title             tags mask     isfloating   noborder  nooverview   isfakefullscreen monitor floatposition */
    { "netease-cloud-music", NULL,                 NULL,             1 << 12,      1,           0,        0,           0,               -1, 0 },
    { "nemo",                NULL,                 NULL,             1 << 10,      0,           0,        0,           0,               -1, 0 },
    { "Google-chrome",       NULL,                 NULL,             1 << 11,      0,           0,        0,           1,               -1, 0 },
    { "Clash for Windows",   NULL,                 NULL,             1 << 15,      1,           0,        1,           0,               -1, 0 },
    { "listen1",             NULL,                 NULL,             1 << 12,      1,           0,        0,           0,               -1, 0 },
    { "xfce4-screenshooter", NULL,                 NULL,             0,            1,           0,        0,           0,               -1, 0 },
    { "xfce-polkit",         NULL,                 NULL,             0,            1,           0,        0,           0,               -1, 5 },
    { "chinese-calendar",    NULL,                 NULL,             0,            1,           1,        1,           0,               -1, 3 },
    { "gnome-calculator",    NULL,                 NULL,             0,            1,           1,        0,           0,               -1, 0 },

    // {"netease-cloud-music",  NULL,                 NULL,             1 << 10,      1,           0,        0,       -1 },
    // {"music",                NULL,                 NULL,             1 << 10,      1,           1,        0,       -1 },
    // {"lx-music-desktop",     NULL,                 NULL,             1 << 10,      1,           1,        0,       -1 },
    // { NULL,                 "tim.exe",             NULL,             1 << 11,      0,           0,        0,       -1 },
    // { NULL,                 "icalingua",           NULL,             1 << 11,      0,           1,        0,       -1 },
    // { NULL,                 "wechat.exe",          NULL,             1 << 12,      0,           0,        0,       -1 },
    // { NULL,                 "wxwork.exe",          NULL,             1 << 13,      0,           0,        0,       -1 },
    // { NULL,                  NULL,                "broken",          0,            1,           0,        0,       -1 },
    // { NULL,                  NULL,                "图片查看",         0,            1,           0,        0,       -1 },
    // { NULL,                  NULL,                "图片预览",         0,            1,           0,        0,       -1 },
    // { NULL,                  NULL,                "crx_",            0,            1,           0,        0,       -1 },
    // {"chrome",               NULL,                 NULL,             1 << 9,       0,           0,        0,       -1 },
    // {"Chromium",             NULL,                 NULL,             1 << 9,       0,           0,        0,       -1 },
    // {"flameshot",            NULL,                 NULL,             0,            1,           0,        0,       -1 },
    // {"float",                NULL,                 NULL,             0,            1,           0,        0,       -1 },
    // {"noborder",             NULL,                 NULL,             0,            1,           1,        0,       -1 },

       /** 部分特殊class的规则 */
    {"float",                NULL,                 NULL,             0,            1,           0,   0,     0,         -1,      0}, // class = float       浮动
    {"global",               NULL,                 NULL,             TAGMASK,      0,           0,   1,      0,        -1,      0}, // class = gloabl      全局
    {"noborder",             NULL,                 NULL,             0,            0,           1,   0,      0,        -1,      0}, // class = noborder    无边框
    {"FGN",                  NULL,                 NULL,             TAGMASK,      1,           1,   1,      0,        -1,      0}, // class = FGN         浮动、全局、无边框
    {"FG",                   NULL,                 NULL,             TAGMASK,      1,           0,   1,      0,        -1,      0}, // class = FG          浮动、全局
    {"FN",                   NULL,                 NULL,             0,            1,           1,   0,      0,        -1,      0}, // class = FN          浮动、无边框
    {"GN",                   NULL,                 NULL,             TAGMASK,      0,           1,   1,      0,        -1,      0}, // CLASS = GN          全局、无边框

    // /** 优先度低 越在上面优先度越低 */
    // { NULL,                  NULL,                "crx_",            0,            1,          0,          0,        -1,      0}, // 错误载入时 会有crx_ 浮动
    // { NULL,                  NULL,                "broken",          0,            1,          0,          0,        -1,      0}, // 错误载入时 会有broken 浮动
};
// clang-format on

const char  *overviewtag    = "OVERVIEW";
const Layout overviewlayout = {"", overview};

/* 自定义布局 */
const Layout layouts[] = {
    {"󰙀", tile},       /* 平铺*/
    {"", magic_grid}, /* 网格 */
    {"", monocle},    /* 单窗口 */
    {"", NULL},       /* 浮动 */
};

// clang-format off
#define MODKEY Mod4Mask
#define SHCMD(cmd)                     \
    {                                  \
        .v = (const char *[])          \
        {                              \
            "/bin/sh", "-c", cmd, NULL \
        }                              \
    }

#define APPNAME(name)                  \
    {                                  \
        .v = name                      \
    }

#define TAGKEYS(KEY, TAG, cmd)                                                          \
    {MODKEY,                        KEY, view_tag,          {.ui = 1 << TAG, .v = cmd}},\
    {MODKEY|ControlMask,            KEY, toggle_tag_view,   {.ui = 1 << TAG}},          \
    {MODKEY|ShiftMask,              KEY, move_to_tag,       {.ui = 1 << TAG}},          \
    {MODKEY|ControlMask|ShiftMask,  KEY, append_to_tag,     {.ui = 1 << TAG}},
// clang-format on

// clang-format off
const Key keys[] = {
    /* modifier             key         function        argument */
    { MODKEY|ControlMask,   XK_Delete,  quit,                       {0} },                  /* super ctrl delete    |  退出dwm */
    { MODKEY,               XK_q,       kill_client,                {0} },                  /* super q              |  关闭窗口 */
    { MODKEY|ControlMask,   XK_q,       force_kill_client,          {0} },                  /* super q              |  关闭窗口 */
 
    { MODKEY|ShiftMask,     XK_t,       set_layout,                 {.v = &layouts[0]} },   /* super shift t        |  切换到平铺布局 */
    { MODKEY|ShiftMask,     XK_g,       set_layout,                 {.v = &layouts[1]} },   /* super shift g        |  切换到网格布局 */
    { MODKEY|ShiftMask,     XK_m,       set_layout,                 {.v = &layouts[2]} },   /* super shift m        |  切换到单窗口布局 */
    { MODKEY|ShiftMask,     XK_w,       set_layout,                 {.v = &layouts[3]} },   /* super shift f        |  切换到浮动布局 */
    { MODKEY|ShiftMask,     XK_space,   set_layout,                 {0} },                  /* super shift space    |  切换上一个布局 */
          
    { MODKEY|ShiftMask,     XK_minus,   set_gaps,                   {.i = -1 } },           /* super shift -        |  窗口间距减小 */
    { MODKEY|ShiftMask,     XK_equal,   set_gaps,                   {.i = +1 } },           /* super shift =        |  窗口间距增大 */
    { MODKEY|ControlMask,   XK_minus,   set_gaps,                   {.i = 0 } },            /* super ctrl -         |  窗口间距重置 */
    { MODKEY|ControlMask,   XK_equal,   set_gaps,                   {.i = 0 } },            /* super ctrl =         |  窗口间距重置 */
 
    { MODKEY,               XK_comma,   set_mfact,                  {.f = -0.05} },         /* super ,              |  缩小主工作区 */
    { MODKEY,               XK_period,  set_mfact,                  {.f = +0.05} },         /* super .              |  放大主工作区 */
    { MODKEY|ShiftMask,     XK_comma,   set_mfact,                  {.f = -0.005} },        /* super ,              |  缩小主工作区 */
    { MODKEY|ShiftMask,     XK_period,  set_mfact,                  {.f = +0.005} },        /* super .              |  放大主工作区 */
 
    { MODKEY,               XK_r,       rotate_client_stack,        {.i = +1 } },           /* super r              |  旋转窗口栈 */
    { MODKEY|ShiftMask,     XK_r,       rotate_client_stack,        {.i = -1 } },           /* super shift r        |  反向旋转窗口栈 */
    { MODKEY,               XK_j,       focusstackvis,              {.i = +1 } },           /* super ctrl -         |  切换显示窗口焦点 */
    { MODKEY,               XK_k,       focusstackvis,              {.i = -1 } },           /* super ctrl -         |  反向切换显示窗口焦点 */
    { MODKEY|ShiftMask,     XK_j,       focusstackhid,              {.i = +1 } },           /* super ctrl -         |  切换隐藏窗口焦点 */
    { MODKEY|ShiftMask,     XK_k,       focusstackhid,              {.i = -1 } },           /* super ctrl -         |  反向切换隐藏窗口焦点 */
 
    { MODKEY|ControlMask,   XK_bracketleft,   focus_monitor,              {.i = -1 } },           /* super ctrl ,         |  光标移动到上一个显示器 */
    { MODKEY|ControlMask,   XK_bracketright,  focus_monitor,              {.i = +1 } },           /* super ctrl .         |  光标移动到下一个显示器 */
    { MODKEY|ShiftMask,     XK_bracketleft,   move_to_monitor,            {.i = -1 } },           /* super shift .        |  将聚焦窗口移动到上一个显示器 */
    { MODKEY|ShiftMask,     XK_bracketright,  move_to_monitor,            {.i = +1 } },           /* super shift .        |  将聚焦窗口移动到下一个显示器 */
 
    { MODKEY,               XK_h,       hide_client,                {0} },                  /* super h              |  隐藏窗口 */
    { MODKEY,               XK_s,       show_client,                {0} },                  /* super s              |  显示窗口 */
    { MODKEY|ShiftMask,     XK_s,       show_all_client,            {0} },                  /* super shift s        |  显示全部窗口 */
    { MODKEY,               XK_o,       show_only_or_all,           {0} },                  /* super o              |  切换 只显示一个窗口 / 全部显示 */

    { MODKEY|ShiftMask,     XK_z,       zoom,                       {0} },                  /* super shift z        |  将当前聚焦窗口置为主窗口 */
 
    { MODKEY,               XK_grave,   toggle_systray,             {0} },                  /* super ~              |  切换 托盘栏显示状态 */
    { MODKEY|ControlMask,   XK_o,       toggle_overview,            {0} },                  /* super o              |  显示所有tag 或 跳转到聚焦窗口的tag */
    { MODKEY,               XK_w,       toggle_floating,            {0} },                  /* super w              |  开启/关闭 聚焦目标的float模式 */
    { MODKEY|ControlMask,   XK_w,       toggle_all_floating,        {0} },                  /* super ctrl w         |  开启/关闭 全部目标的float模式 */
    { MODKEY|ShiftMask,     XK_v,       toggle_bar,                 {0} },                  /* super shift v        |  开启/关闭 状态栏 */
    { MODKEY,               XK_m,       toggle_full_screen,         {0} },                  /* super ctrl m         |  开启/关闭 全屏 */
    { MODKEY|ControlMask,   XK_m,       toggle_fake_full_screen,    {0} },                  /* super ctrl m         |  开启/关闭 全屏 */
 
    { MODKEY|ShiftMask,     XK_n,       inc_nmaster,                {.i = +1 } },           /* super shift n        |  改变主工作区窗口数量 (1 2中切换) */
 
    { MODKEY,               XK_Tab,     view_tag,                   {0} },                  /* super tab            |  显示上一个标签*/
    { MODKEY|ControlMask,   XK_0,       view_tag,                   {.ui = ~0 } },          /* super tab            |  显示全部标签*/
    { MODKEY|ShiftMask,     XK_0,       move_to_tag,                {.ui = ~0 } },          /* super tab            |  移动到全部标签*/


    { MODKEY|ControlMask,   XK_Up,      move_window,                {.ui = UP} },           /* super ctrl up        |  移动窗口 */
    { MODKEY|ControlMask,   XK_Down,    move_window,                {.ui = DOWN} },         /* super ctrl down      |  移动窗口 */
    { MODKEY|ControlMask,   XK_Left,    move_window,                {.ui = LEFT} },         /* super ctrl left      |  移动窗口 */
    { MODKEY|ControlMask,   XK_Right,   move_window,                {.ui = RIGHT} },        /* super ctrl right     |  移动窗口 */
 
    { MODKEY|Mod1Mask,      XK_Up,      resize_window,              {.ui = V_REDUCE} },     /* super alt up         |  调整窗口 */
    { MODKEY|Mod1Mask,      XK_Down,    resize_window,              {.ui = V_EXPAND} },     /* super alt down       |  调整窗口 */
    { MODKEY|Mod1Mask,      XK_Left,    resize_window,              {.ui = H_REDUCE} },     /* super alt left       |  调整窗口 */
    { MODKEY|Mod1Mask,      XK_Right,   resize_window,              {.ui = H_EXPAND} },     /* super alt right      |  调整窗口 */

    /* app_starter + APPNAME 启动App */
    { MODKEY,               XK_Return,  app_starter,            APPNAME("terminal") },
    { MODKEY|ShiftMask,     XK_Return,  toggle_scratch,         APPNAME("scratchpad") },
    { MODKEY,               XK_slash,   app_starter,            APPNAME("rofi_drun") },
    { MODKEY|ControlMask,   XK_slash,   app_starter,            APPNAME("rofi_run") },
    { MODKEY|ShiftMask,     XK_slash,   app_starter,            APPNAME("rofi_window") },
    { MODKEY|Mod1Mask,      XK_slash,   app_starter,            APPNAME("rofi_custom") },
    { MODKEY|ControlMask,   XK_l,       app_starter,            APPNAME("blurlock") },
    { MODKEY|ShiftMask,     XK_x,       app_starter,            APPNAME("screenshot") },
    { MODKEY|ControlMask,   XK_k,       app_starter,            APPNAME("screenkey") },
    { MODKEY,               XK_p,       app_starter,            APPNAME("set_screen") },

    { 0,                    XF86XK_AudioMute,         spawn, SHCMD("$DWM_PATH/set_vol.sh toggle") },    /* Fn F5 | 禁音切换                 */
    { 0,                    XF86XK_AudioRaiseVolume,  spawn, SHCMD("$DWM_PATH/set_vol.sh up") },        /* Fn F6   | 音量加                 */
    { 0,                    XF86XK_AudioLowerVolume,  spawn, SHCMD("$DWM_PATH/set_vol.sh down") },      /* Fn F7 | 音量减                   */

    { MODKEY|ShiftMask,      XK_q,      spawn, SHCMD("kill -9 $(xprop | grep _NET_WM_PID | awk '{print $3}')") }, /* super shift q    | 选中某个窗口并强制kill */
    { ShiftMask|ControlMask, XK_c,      spawn, SHCMD("xclip -o | xclip -selection c") },                          /* super shift c    | 进阶复制               */

    /* super key : 跳转到对应tag */
    /* super shift key : 将聚焦窗口移动到对应tag */
    /* 若跳转后的tag无窗口且附加了cmd1或者cmd2就执行对应的cmd */
    /* key tag cmd1 cmd2 */
    TAGKEYS(XK_1, 0, 0)
    TAGKEYS(XK_2, 1, 0)
    TAGKEYS(XK_3, 2, 0)
    TAGKEYS(XK_4, 3, 0)
    TAGKEYS(XK_5, 4, 0)
    TAGKEYS(XK_6, 5, 0)
    TAGKEYS(XK_7, 6, 0)
    TAGKEYS(XK_8, 7, 0)
    TAGKEYS(XK_9, 8, 0)
    TAGKEYS(XK_a, 9, 0)
    TAGKEYS(XK_a, 10, "filemanager")
    TAGKEYS(XK_b, 11, "browser")
    TAGKEYS(XK_c, 12, "music")
    TAGKEYS(XK_d, 13, "video")
    TAGKEYS(XK_e, 14, "wechat")
    TAGKEYS(XK_f, 15, "ssr")
};
// clang-format on

// clang-format off
const Button buttons[] = {
    /* click                event mask      button          function        argument */
    /* 点击布局符号操作 */
    { ClkLtSymbol,      0,              Button1,    set_layout,         {0} },                  // 左键       | 点击布局符号 | 切换上一个布局
    { ClkLtSymbol,      0,              Button3,    set_layout,         {.v = &layouts[0]} },   // 右键       | 点击布局符号 | 切换平铺布局
    { ClkLtSymbol,      MODKEY,         Button1,    set_layout,         {.v = &layouts[1]} },   // super+左键 | 点击布局符号 | 切换网格布局
    { ClkLtSymbol,      MODKEY,         Button3,    set_layout,         {.v = &layouts[2]} },   // super+右键 | 点击布局符号 | 切换单窗口布局
    { ClkLtSymbol,      ControlMask,    Button3,    toggle_monocle,     {0} },                  // ctrl+右键  | 点击布局符号 | 切换单窗口布局显示窗口数
    /* 点击窗口标题栏操作 */
    { ClkWinTitle,      0,              Button1,    hide_other_wins,    {0} },                  // 左键       | 点击标题     | 隐藏其他窗口仅保留该窗口
    { ClkWinTitle,      0,              Button3,    toggle_window,      {0} },                  // 右键       | 点击标题     | 切换窗口显示状态
    { ClkWinTitle,      0,              Button2,    zoom,               {0} },                  // 中键       | 点击标题     | 将当前聚焦窗口置为主窗口
    /* 点击窗口操作 */
    { ClkClientWin,     MODKEY,         Button1,    move_by_mouse,      {0} },                  // super+左键 | 拖拽窗口     | 拖拽窗口
    { ClkClientWin,     MODKEY,         Button2,    toggle_floating,    {0} },                  // super+中键 | 拖拽窗口     | 浮动窗口
    { ClkClientWin,     MODKEY,         Button3,    resize_by_mouse,    {0} },                  // super+右键 | 拖拽窗口     | 改变窗口大小
    /* 点击tag操作 */
    { ClkTagBar,        0,              Button1,    view_tag,           {0} },                  // 左键       | 点击tag     | 切换tag
    { ClkTagBar,        0,              Button3,    toggle_tag_view,    {0} },                  // 右键       | 点击tag     | 切换是否显示tag
    { ClkTagBar,        MODKEY,         Button1,    move_to_tag,        {0} },                  // super+左键 | 点击tag     | 将窗口移动到对应tag
    { ClkTagBar,        MODKEY,         Button3,    append_to_tag,      {0} },                  // super+右键 | 点击tag     | 将窗口添加到对应tag

    /* 点击状态栏操作 */
    { ClkStatusText,    0,               Button1,   click_status_bar,   {0} },                  // 左键       | 点击状态栏   | 根据状态栏的信号执行 statusbar.sh $signal L
    { ClkStatusText,    0,               Button2,   click_status_bar,   {0} },                  // 中键       | 点击状态栏   | 根据状态栏的信号执行 statusbar.sh $signal M
    { ClkStatusText,    0,               Button3,   click_status_bar,   {0} },                  // 右键       | 点击状态栏   | 根据状态栏的信号执行 statusbar.sh $signal R
    { ClkStatusText,    0,               Button4,   click_status_bar,   {0} },                  // 鼠标滚轮上  | 状态栏      | 根据状态栏的信号执行 statusbar.sh $signal U
    { ClkStatusText,    0,               Button5,   click_status_bar,   {0} },                  // 鼠标滚轮下  | 状态栏      | 根据状态栏的信号执行 statusbar.sh $signal D
    /* 点击bar空白处 */
    { ClkBarEmpty,      0,               Button1,   app_starter, APPNAME("rofi_window") },      // 左键       | bar空白处   |  rofi 执行 window
    { ClkBarEmpty,      0,               Button3,   app_starter, APPNAME("rofi_drun") },        // 右键       | bar空白处   |  rofi 执行 drun                                                                                                                   
};
// clang-format on

ARRAY_ITEM_COUNT_DEF(fonts)
ARRAY_ITEM_COUNT_DEF(colors)
ARRAY_ITEM_COUNT_DEF(alphas)
ARRAY_ITEM_COUNT_DEF(tags)
ARRAY_ITEM_COUNT_DEF(rules)
ARRAY_ITEM_COUNT_DEF(layouts)
ARRAY_ITEM_COUNT_DEF(keys)
ARRAY_ITEM_COUNT_DEF(buttons)
