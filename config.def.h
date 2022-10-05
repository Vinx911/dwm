
static       int          showsystray             	= 1;   	/* 是否显示系统托盘 */
static const int          newclientathead           = 0;    /* 定义新窗口在栈顶还是栈底 */
static const unsigned int borderpx  				= 1;	/* 窗口边框大小 */
static const unsigned int gappx     				= 5;	/* 窗口间隔 */
static const unsigned int overviewgappx          	= 5;	/* 预览窗口间隔 */
static const unsigned int snap      				= 32;	/* 边缘依附宽度 */
static const int          showbar            		= 1;	/* 是否显示状态栏 */
static const int          topbar             		= 1;	/* 指定状态栏位置 0底部 1顶部 */
static const int 		  userbarheight             = 8;    /* bar的额外高度, 总高度为 字体高度 + userbarheight */
static const unsigned int systrayiconsize 			= 20;   /* 系统托盘图标尺寸 */
static const unsigned int systraypinning 			= 2;   	/* 托盘跟随的显示器 0代表不指定显示器 */
static const unsigned int systrayspacing 			= 2;   	/* 系统托盘间距 */
static const int          systraypinningfailfirst 	= 1;   	/* 1：如果 pinning 失败，在第一台显示器上显示系统托盘，0：在最后一台显示器上显示系统托盘 */
static const int          winiconsize             	= 16;   /* 窗口图标尺寸 */
static const int          winiconspacing            = 5;    /* 窗口图标与窗口标题间的间距*/
static const float        mfact                     = 0.55; /* 主工作区 大小比例 */
static const int          nmaster                   = 1;    /* 主工作区 窗口数量 */
static const int          resizehints               = 1;    /* 1 means respect size hints in tiled resizals */
static const int          lockfullscreen            = 1;    /* 强制焦点在全屏窗口上 */

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
/* 自定义特定实例的显示状态 */
//            ﮸ 
static const char *tags[] = { "", "", "", "", "", "", "", "", "", "", "", "ﬄ", "﬐", "" };
// static const char *tags[] = { "", "", "", "", "", "", "", "﬏", "", "" };

static const Rule rules[] = {
	/* class                instance    title       tags mask     isfloating   noborder  nooverview   monitor */
    {"netease-cloud-music", NULL,       NULL,       1 << 10,      1,           0,        0,           -1 },
	{ "Gimp",               NULL,       NULL,       0,            1,           0,        0,           -1 },
	{ "Firefox",            NULL,       NULL,       1 << 8,       0,           0,        0,           -1 },
};

static const char *overviewtag = "OVERVIEW";
static const Layout overviewlayout = { "",  overview };

/* 自定义布局 */
static const Layout layouts[] = {
	{ "﬿", tile },      /* 平铺*/
	{ "", NULL },      /* 浮动 */
	{ "", monocle },   /* 单窗口 */
	{ "﩯", magicgrid }, /* 网格 */
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

#define TAGKEYS1B(KEY,TAG, cmd1) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG, .v = cmd1} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG, .v = cmd1} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

#define TAGKEYS2(KEY,TAG, cmd1, cmd2) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG, .v = cmd1} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG, .v = cmd2} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },


/* commands */
static const char *roficmd[] = { "rofi", "-show", "drun", "-show-icons", NULL };
static const char *termcmd[]  = { "alacritty", NULL };
static const char scratchpadname[]  = { "scratchpad" };
static const char *scratchpadcmd[]  = { "alacritty", "-t", scratchpadname };

static const Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          {.v = roficmd } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },
	// { MODKEY,                       XK_grave,  togglescratch,  {.v = scratchpadcmd } },
	{ MODKEY,                       XK_grave,  togglescratch,  {.v = scratchpadcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	// { MODKEY|ShiftMask,             XK_j,      rotatestack,    {.i = +1 } },
	// { MODKEY|ShiftMask,             XK_k,      rotatestack,    {.i = -1 } },
	{ MODKEY,                       XK_j,      focusstackvis,  {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstackvis,  {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_j,      focusstackhid,  {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_k,      focusstackhid,  {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,                       XK_g,      setlayout,      {.v = &layouts[3]} },
	{ MODKEY|ShiftMask,             XK_f,      fullscreen,     {0} },
	{ MODKEY,                       XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	{ MODKEY,                       XK_s,      show,           {0} },
	{ MODKEY|ShiftMask,             XK_s,      showall,        {0} },
	// { MODKEY,                       XK_h,      hide,           {0} },
	{ MODKEY,                       XK_minus,  setgaps,        {.i = -1 } },
	{ MODKEY,                       XK_equal,  setgaps,        {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_equal,  setgaps,        {.i = 0 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	TAGKEYS2(XK_c, 8,  "google-chrome-stable", "google-chrome-stable")
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
	{ MODKEY,              XK_a,            toggleoverview,   {0} },                     /* super a            |  显示所有tag 或 跳转到聚焦窗口的tag */
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button1,        togglewin,      {0} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

