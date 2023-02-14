
static       int          showsystray               = 1;    /* 是否显示系统托盘 */
static const int          newclientathead           = 0;    /* 定义新窗口在栈顶还是栈底 */
static const int          minclientheight           = 100;  /* 最小窗口高度 */
static const unsigned int borderpx                  = 1;    /* 窗口边框大小 */
static const unsigned int gappx                     = 5;    /* 窗口间隔 */
static const unsigned int overviewgappx             = 5;    /* 预览窗口间隔 */
static const unsigned int snap                      = 32;   /* 边缘依附宽度 */
static const int          showbar                   = 1;    /* 是否显示状态栏 */
static const int          topbar                    = 1;    /* 指定状态栏位置 0底部 1顶部 */
static const int           userbarheight            = 8;    /* bar的额外高度, 总高度为 字体高度 + userbarheight */
static const unsigned int systrayiconsize           = 20;   /* 系统托盘图标尺寸 */
static const unsigned int systraypinning            = 2;    /* 托盘跟随的显示器 0代表不指定显示器 */
static const unsigned int systrayspacing            = 2;    /* 系统托盘间距 */
static const int          systraypinningfailfirst   = 1;    /* 1：如果 pinning 失败，在第一台显示器上显示系统托盘，0：在最后一台显示器上显示系统托盘 */
static const int          winiconsize               = 16;   /* 窗口图标尺寸 */
static const int          winiconspacing            = 5;    /* 窗口图标与窗口标题间的间距*/
static const float        mfact                     = 0.55; /* 主工作区 大小比例 */
static const int          nmaster                   = 1;    /* 主工作区 窗口数量 */
static const int          resizehints               = 1;    /* 1 means respect size hints in tiled resizals */
static const int          lockfullscreen            = 1;    /* 强制焦点在全屏窗口上 */
static const char         scratchpadname[]          = { "scratchpad" };

/* Lockfile */
static const char         lockfile[]                = "/tmp/dwm.lock";

static const unsigned int baralpha                  = 0xa0; /* 状态栏透明度 */
static const unsigned int borderalpha               = OPAQUE;/* 边框透明度 */

static const char *fonts[]          = { "JetBrainsMono Nerd Font:style=medium:size=13", "monospace:size=13" };
static const char *colors[][3]      = {
    /*                       fg         bg         border   */
    [SchemeNorm]      = { "#bbbbbb", "#333333", "#444444" },
    [SchemeSel]       = { "#ffffff", "#37474F", "#42A5F5" },
    [SchemeHid]       = { "#dddddd", NULL,      NULL      },
    [SchemeUnderline] = { "#7799AA", "#7799AA", "#7799AA" },
};
static const unsigned int alphas[][3]      = {
    /*               fg      bg        border     */
    [SchemeNorm] = { OPAQUE, baralpha, borderalpha },
    [SchemeSel]  = { OPAQUE, baralpha, borderalpha },
};

/* 自定义tag名称 */
/* 自定义特定实例的显示状态 */\
static const char *tags[] = { "", "", "", "", "", "", "", "", "", "", "", "", "", "﬐", "" };\

static const Rule rules[] = {
    /* class                 instance              title             tags mask     isfloating   noborder  nooverview   monitor */
    { "netease-cloud-music", NULL,                 NULL,             1 << 10,      1,           0,        0,           -1 },
    { "Thunar",              NULL,                 NULL,             1 << 9,       0,           0,        0,           -1 },
    { "Google-chrome",       NULL,                 NULL,             1 << 10,      0,           0,        0,           -1 },
    { "Clash for Windows",   NULL,                 NULL,             1 << 14,      1,           0,        1,           -1 },


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
};

static const char *overviewtag = "OVERVIEW";
static const Layout overviewlayout = { "",  overview };

/* 自定义布局 */
static const Layout layouts[] = {
    { "﬿", tile },      /* 平铺*/
    { "﩯", magicgrid }, /* 网格 */
    { "", monocle },   /* 单窗口 */
    { "", NULL },      /* 浮动 */
};

#define MODKEY Mod4Mask
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }
#define TAGKEYS(KEY,TAG) \
    { MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
    { MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

#define TAGKEYS1(KEY,TAG, cmd1) \
    { MODKEY,                       KEY,      view,           {.ui = 1 << TAG, .v = cmd1} }, \
    { MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
    { MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
    { MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

#define TAGKEYS2(KEY,TAG, cmd1, cmd2) \
    { MODKEY,                       KEY,      view,           {.ui = 1 << TAG, .v = cmd1} }, \
    { MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
    { MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG, .v = cmd2} }, \
    { MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* commands */
static const Key keys[] = {
    /* modifier                     key        function        argument */
    { MODKEY|ShiftMask,             XK_q,      quit,           {0} },                               /* super shift q             |  退出dwm */
    { MODKEY,                       XK_q,      killclient,     {0} },                               /* super q                   |  关闭窗口 */

         
    { MODKEY|ShiftMask,             XK_t,      setlayout,      {.v = &layouts[0]} },                /* super shift t             |  切换到平铺布局 */
    { MODKEY|ShiftMask,             XK_g,      setlayout,      {.v = &layouts[1]} },                /* super shift g             |  切换到网格布局 */
    { MODKEY|ShiftMask,             XK_m,      setlayout,      {.v = &layouts[2]} },                /* super shift m             |  切换到单窗口布局 */
    { MODKEY|ShiftMask,             XK_w,      setlayout,      {.v = &layouts[3]} },                /* super shift f             |  切换到浮动布局 */
    { MODKEY|ShiftMask,             XK_space,  setlayout,      {0} },                               /* super shift space         |  切换上一个布局 */
         
    { MODKEY|ShiftMask,             XK_minus,  setgaps,        {.i = -1 } },                        /* super shift -             |  窗口间距减小 */
    { MODKEY|ShiftMask,             XK_equal,  setgaps,        {.i = +1 } },                        /* super shift =             |  窗口间距增大 */
    { MODKEY|ControlMask,           XK_minus,  setgaps,        {.i = 0 } },                         /* super ctrl -              |  窗口间距重置 */
    { MODKEY|ControlMask,           XK_equal,  setgaps,        {.i = 0 } },                         /* super ctrl =              |  窗口间距重置 */

    { MODKEY,                       XK_comma,  setmfact,       {.f = -0.05} },                      /* super ,                   |  缩小主工作区 */
    { MODKEY,                       XK_period, setmfact,       {.f = +0.05} },                      /* super .                   |  放大主工作区 */

    { MODKEY,                       XK_r,      rotatestack,    {.i = +1 } },                        /* super r                   |  旋转窗口栈 */
    { MODKEY|ShiftMask,             XK_r,      rotatestack,    {.i = -1 } },                        /* super shift r             |  反向旋转窗口栈 */
    { MODKEY,                       XK_j,      focusstackvis,  {.i = +1 } },                        /* super ctrl -              |  切换显示窗口焦点 */
    { MODKEY,                       XK_k,      focusstackvis,  {.i = -1 } },                        /* super ctrl -              |  反向切换显示窗口焦点 */
    { MODKEY|ShiftMask,             XK_j,      focusstackhid,  {.i = +1 } },                        /* super ctrl -              |  切换隐藏窗口焦点 */
    { MODKEY|ShiftMask,             XK_k,      focusstackhid,  {.i = -1 } },                        /* super ctrl -              |  反向切换隐藏窗口焦点 */

    { MODKEY|ControlMask,           XK_comma,  focusmon,       {.i = -1 } },                        /* super ctrl ,              |  光标移动到上一个显示器 */
    { MODKEY|ControlMask,           XK_period, focusmon,       {.i = +1 } },                        /* super ctrl .              |  光标移动到下一个显示器 */
    { MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },                        /* super shift .             |  将聚焦窗口移动到上一个显示器 */
    { MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },                        /* super shift .             |  将聚焦窗口移动到下一个显示器 */

    { MODKEY,                       XK_h,      hide,           {0} },                               /* super h                   |  隐藏窗口 */
    { MODKEY,                       XK_s,      show,           {0} },                               /* super s                   |  显示窗口 */
    { MODKEY|ShiftMask,             XK_s,      showall,        {0} },                               /* super shift s             |  显示全部窗口 */

    { MODKEY|ShiftMask,             XK_z,      zoom,           {0} },                               /* super shift z             |  将当前聚焦窗口置为主窗口 */

    { MODKEY,                       XK_grave,  togglesystray,  {0} },                               /* super ~                   |  切换 托盘栏显示状态 */
    { MODKEY,                       XK_o,      toggleoverview, {0} },                               /* super o                   |  显示所有tag 或 跳转到聚焦窗口的tag */
    { MODKEY,                       XK_w,      togglefloating, {0} },                               /* super w                   |  开启/关闭 聚焦目标的float模式 */
    { MODKEY|ControlMask,           XK_w,      toggleallfloating,{0} },                             /* super ctrl w              |  开启/关闭 全部目标的float模式 */
    { MODKEY|ShiftMask,             XK_v,      togglebar,      {0} },                               /* super shift v             |  开启/关闭 状态栏 */
    { MODKEY|ControlMask,           XK_m,      fullscreen,     {0} },                               /* super ctrl m              |  开启/关闭 全屏 */

    { MODKEY|ShiftMask,             XK_n,      incnmaster,     {.i = +1 } },                        /* super shift n             |  改变主工作区窗口数量 (1 2中切换) */

    { MODKEY,                       XK_Tab,    view,           {0} },                               /* super tab                 |  显示上一个标签*/
    { MODKEY,                       XK_0,      view,           {.ui = ~0 } },                       /* super tab                 |  显示全部标签*/
    { MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },                       /* super tab                 |  移动到全部标签*/

    /* spawn + SHCMD 执行对应命令 */
    { MODKEY,              XK_Return,       spawn,            SHCMD("st") },
    { MODKEY,              XK_Return,       spawn,            SHCMD("~/.dwm/scripts/app-starter.sh terminal") },
    //{ MODKEY|ShiftMask,    XK_Return,       togglescratch,    SHCMD("~/.dwm/scripts/app-starter.sh scratchpad") },
    { MODKEY|ShiftMask,    XK_Return,       togglescratch,    SHCMD("st -t scratchpad -c float") },
    { MODKEY,              XK_p,            spawn,            SHCMD("~/.dwm/scripts/app-starter.sh rofi") },

    /* super key : 跳转到对应tag */
    /* super shift key : 将聚焦窗口移动到对应tag */
    /* 若跳转后的tag无窗口且附加了cmd1或者cmd2就执行对应的cmd */
    /* key tag cmd1 cmd2 */
    TAGKEYS(XK_1,  0)
    TAGKEYS(XK_2,  1)
    TAGKEYS(XK_3,  2)
    TAGKEYS(XK_4,  3)
    TAGKEYS(XK_5,  4)
    TAGKEYS(XK_6,  5)
    TAGKEYS(XK_7,  6)
    TAGKEYS(XK_8,  7)
    TAGKEYS(XK_9,  8)
    TAGKEYS2(XK_a, 9,  "~/.dwm/scripts/app-starter.sh filemanager",  "~/.dwm/scripts/app-starter.sh filemanager")
    TAGKEYS2(XK_b, 10, "~/.dwm/scripts/app-starter.sh chrome",   "~/.dwm/scripts/app-starter.sh chrome")
    TAGKEYS2(XK_c, 11, "~/.dwm/scripts/app-starter.sh music",     "~/.dwm/scripts/app-starter.sh music")
    TAGKEYS2(XK_d, 12, "~/.dwm/scripts/app-starter.sh video",  "~/.dwm/scripts/app-starter.sh video")
    TAGKEYS2(XK_e, 13, "~/.dwm/scripts/app-starter.sh wechat",  "~/.dwm/scripts/app-starter.sh wechat")
    TAGKEYS2(XK_f,  14, "~/.dwm/scripts/app-starter.sh ssr",  "~/.dwm/scripts/app-starter.sh ssr")
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
    /* click                event mask      button          function        argument */
    { ClkLtSymbol,          0,              Button1,        setlayout,        {0} },
    { ClkLtSymbol,          0,              Button3,        setlayout,        {.v = &layouts[2]} },
    { ClkLtSymbol,          ControlMask,    Button1,        togglemonocle,    {0} },
    { ClkWinTitle,          0,              Button1,        togglewin,        {0} },
    { ClkWinTitle,          0,              Button2,        zoom,             {0} },
    { ClkClientWin,         MODKEY,         Button1,        movemouse,        {0} },
    { ClkClientWin,         MODKEY,         Button2,        togglefloating,   {0} },
    { ClkClientWin,         MODKEY,         Button3,        resizemouse,      {0} },
    { ClkTagBar,            0,              Button1,        view,             {0} },
    { ClkTagBar,            0,              Button3,        toggleview,       {0} },
    { ClkTagBar,            MODKEY,         Button1,        tag,              {0} },
    { ClkTagBar,            MODKEY,         Button3,        toggletag,        {0} },
};

