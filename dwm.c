
/**
 * display: display是X中用来管理物理设备显示器的一个结构体.
 * screen: screen是display下用来更进一步管理屏幕的结构体，
 *         我个人理解为是一层幕布，创建screen后会有一个根视窗(root window)，
 *         个人猜测是觉得screen中的window结构体使用链表，因此需要有一个根节点。
 * window:
 * window就是建立在screen之上的一个个应用的窗口，可以理解为你打开一个可视化程序就是打开了一个窗口。
 *
 * 以上3者的关系:
 * 最大的是display，它表示了电脑的显示屏，screen像是显示屏里的一张幕布，window窗口是绘制于幕布之上的东西
 *
 * monitor:
 * monitor是dwm内定义的概念，指的是监视器,是一个包含当前桌面各种配置信息的结构体。
 * client: 每个应用的窗口
 *
 * dwm的大致桌面结构为：每个屏幕对应的桌面，都是一个monitor,每个桌面中都有一个bar区域和下面的应用放置区域，这2个区域分别是2个window
 */

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

#define CLEANMASK(mask)               \
    (mask & ~(numlockmask | LockMask) \
     & (ShiftMask | ControlMask | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask))

/* variables */
char        *dwm_script_path    = NULL;           /* dwm脚本路径 */
char         status_text[2048];                   /* 状态栏文本 */
int          screen;                              /* 默认屏幕 */
int          screen_width;                        /* 默认屏幕的宽 */
int          screen_height;                       /* 默认屏幕的高 */
int          bar_height;                          /* bar 高度 */
int          tag_bar_width;                       /* 标签栏宽度 */
int          lt_symbol_width;                     /* 布局符号宽度 */
int          statsu_bar_width;                    /* 状态栏宽度 */
int          text_lr_pad;                         /* 文本左右填充的总和 */
int          bar_ver_padding;                     /* vertical padding for bar */
int          bar_side_padding;                    /* side padding for bar */
unsigned int numlockmask = 0;                     /* 数字键盘锁按键掩码 */

Atom     wmatom[WMLast], netatom[NetLast], xatom[XLast];
Cur     *cursor[CurLast]; /* 光标 */
Clr    **scheme;          /* 配色 */
Display *display;         /* 默认显示器 */
Drw     *drw;
Monitor *monitor_list;   /* 监视器列表 */
Monitor *select_monitor; /* 当前选择的监视器 */
Window   root_window;    /* 根窗口 */

int      useargb = 0; /* 使用argb */
Visual  *visual;      /* 视觉 */
int      depth;       /* 颜色位深 */
Colormap color_map;   /* 颜色映射 */

unsigned int scratchtag = 1 << TAGS_COUNT; /* 便签簿标签 */

static int    running = 1;
static Window wmcheck_window;
static int (*xerrorxlib)(Display *, XErrorEvent *);

void button_press(XEvent *e);
void client_message(XEvent *e);
void configure_notify(XEvent *e);
void configure_request(XEvent *e);
void destroy_notify(XEvent *e);
void enter_notify(XEvent *e);
void expose(XEvent *e);
void focusin(XEvent *e);
void key_press(XEvent *e);
void mapping_notify(XEvent *e);
void map_request(XEvent *e);
void motion_notify(XEvent *e);
void property_notify(XEvent *e);
void resize_request(XEvent *e);
void unmap_notify(XEvent *e);
void grab_keys(void);

void (*handler[LASTEvent])(XEvent *) = {
    // 事件数组
    [ButtonPress]      = button_press,
    [ClientMessage]    = client_message,
    [ConfigureRequest] = configure_request,
    [ConfigureNotify]  = configure_notify,
    [DestroyNotify]    = destroy_notify,
    [EnterNotify]      = enter_notify,
    [Expose]           = expose,
    [FocusIn]          = focusin,
    [KeyPress]         = key_press,
    [MappingNotify]    = mapping_notify,
    [MapRequest]       = map_request,
    [MotionNotify]     = motion_notify,
    [PropertyNotify]   = property_notify,
    [ResizeRequest]    = resize_request,
    [UnmapNotify]      = unmap_notify,
};

/**
 * 鼠标按下
 */
void button_press(XEvent *e)
{
    unsigned int         click = ClkRootWin;
    Arg                  arg   = {0};
    XButtonPressedEvent *ev    = &e->xbutton;

    /* client_focus monitor if necessary */
    Monitor *monitor = window_to_monitor(ev->window);
    if (monitor != NULL && monitor != select_monitor) {
        client_unfocus(select_monitor->select, 1);
        select_monitor = monitor;
        client_focus(NULL);
    }

    if (ev->window == select_monitor->bar_window) {
        unsigned int click_tag = 0;  // 点击的tag
        unsigned int x         = 0;

        if (select_monitor->is_overview) {
            x += TEXTW(overviewtag);
            click_tag = ~0;
            if (ev->x > x) {
                click_tag = TAGS_COUNT;
            }
        } else {
            unsigned int occ = 0;
            for (Client *client = monitor->clients; client; client = client->next) {
                occ |= client->tags == 255 ? 0 : client->tags;
            }

            do {
                /* do not reserve space for vacant tags */
                if (!(occ & 1 << click_tag || monitor->tagset[monitor->seltags] & 1 << click_tag)) {
                    continue;
                }
                x += TEXTW(tags[click_tag]);
            } while (ev->x >= x && ++click_tag < TAGS_COUNT);
        }

        // 布局符号起始x坐标
        int lt_symbol_x = tag_bar_width + lt_symbol_width;

        // 系统托盘宽度
        int systray_width = 0;
        if (select_monitor == systray_to_monitor(select_monitor)) {
            systray_width = systray_get_width();
            if (systray_width != 0) {
                systray_width = systray_width + systray_pinning + 2;
            }
        }

        // 状态栏宽度
        int status_bar_width = status_bar_draw(select_monitor, status_text);
        // 状态栏起始x坐标
        int status_bar_x = bar_width(select_monitor) - status_bar_width - systray_width;

        if (click_tag < TAGS_COUNT) {
            click  = ClkTagBar;
            arg.ui = 1 << click_tag;
        } else if (ev->x < lt_symbol_x) {
            click = ClkLtSymbol;
        } else if (ev->x > status_bar_x) {
            click  = ClkStatusText;
            arg.i  = ev->x - status_bar_x;
            arg.ui = ev->button;  // 1 => L，2 => M，3 => R, 5 => U, 6 => D
        } else {
            click = ClkBarEmpty;
            
            x += lt_symbol_width;
            Client *client = monitor->clients;

            if (monitor->task_count == 0)
                return;

            if (client) {
                do {
                    if (ISVISIBLE(client)) {
                        x += (1.0 / (double)monitor->task_count) * monitor->task_bar_width;
                    }
                } while (ev->x > x && (client = client->next));

                click = ClkWinTitle;
                arg.v = client;
            }
        }
    } else {
        Client *client = window_to_client(ev->window);
        if (client != NULL) {
            client_focus(client);
            client_restack(select_monitor);
            XAllowEvents(display, ReplayPointer, CurrentTime);
            click = ClkClientWin;
        }
    }

    for (int i = 0; i < buttons_count(); i++) {
        if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
            && CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
        {
            if ((click == ClkTagBar || click == ClkWinTitle || click == ClkStatusText) && buttons[i].arg.i == 0) {
                buttons[i].func(&arg);
            } else {
                buttons[i].func(&buttons[i].arg);
            }
        }
    }
}

/**
 * 客户端消息
 */
void client_message(XEvent *e)
{
    XClientMessageEvent *cme = &e->xclient;
    Client              *c   = window_to_client(cme->window);

    if (show_systray && cme->window == systray->win && cme->message_type == netatom[NetSystemTrayOP]) {
        systray_client_message(e);
        return;
    }

    if (!c) {
        return;
    }
    if (cme->message_type == netatom[NetWMState]) {
        if (cme->data.l[1] == netatom[NetWMFullscreen] || cme->data.l[2] == netatom[NetWMFullscreen]) {
            client_set_full_screen(c, (cme->data.l[0] == 1     /* _NET_WM_STATE_ADD    */
                                       || (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */
                                           && (!c->isfullscreen || c->isfakefullscreen))));
        }
    } else if (cme->message_type == netatom[NetActiveWindow]) {
        if (c != select_monitor->select && !c->isurgent) {
            client_seturgent(c, 1);
        }

        if (c == select_monitor->select) {
            return;
        }
        // 若不是当前显示器 则跳转到对应显示器
        if (c->mon != select_monitor) {
            focus_monitor(&(Arg){.i = +1});
        }
        // 若不适当前tag 则跳转到对应tag
        if (!ISVISIBLE(c)) {
            view_tag(&(Arg){.ui = c->tags});
        }
        // 选中窗口
        client_focus(c);
        client_pointer_focus_win(c);
    }
}

/**
 * 配置请求
 */
void configure_request(XEvent *e)
{
    Client                 *c;
    Monitor                *m;
    XConfigureRequestEvent *ev = &e->xconfigurerequest;
    XWindowChanges          wc;

    if ((c = window_to_client(ev->window))) {
        if (ev->value_mask & CWBorderWidth) {
            c->bw = ev->border_width;
        } else if (c->isfloating || !select_monitor->lt[select_monitor->sellt]->arrange) {
            m = c->mon;
            if (ev->value_mask & CWX) {
                c->oldx = c->x;
                c->x    = m->mx + ev->x;
            }
            if (ev->value_mask & CWY) {
                c->oldy = c->y;
                c->y    = m->my + ev->y;
            }
            if (ev->value_mask & CWWidth) {
                c->oldw = c->w;
                c->w    = ev->width;
            }
            if (ev->value_mask & CWHeight) {
                c->oldh = c->h;
                c->h    = ev->height;
            }
            if ((c->x + c->w) > m->mx + m->mw && c->isfloating) {
                c->x = m->mx + (m->mw / 2 - WIDTH(c) / 2); /* center in x direction */
            }
            if ((c->y + c->h) > m->my + m->mh && c->isfloating) {
                c->y = m->my + (m->mh / 2 - HEIGHT(c) / 2); /* center in y direction */
            }
            if ((ev->value_mask & (CWX | CWY)) && !(ev->value_mask & (CWWidth | CWHeight))) {
                client_configure(c);
            }
            if (ISVISIBLE(c)) {
                XMoveResizeWindow(display, c->win, c->x, c->y, c->w, c->h);
            }
        } else {
            client_configure(c);
        }
    } else {
        wc.x            = ev->x;
        wc.y            = ev->y;
        wc.width        = ev->width;
        wc.height       = ev->height;
        wc.border_width = ev->border_width;
        wc.sibling      = ev->above;
        wc.stack_mode   = ev->detail;
        XConfigureWindow(display, ev->window, ev->value_mask, &wc);
    }
    XSync(display, False);
}

/**
 * 配置通知
 */
void configure_notify(XEvent *e)
{
    Monitor         *m;
    Client          *c;
    XConfigureEvent *ev = &e->xconfigure;
    int              dirty;

    /* TODO: monitor_update_geometries handling sucks, needs to be simplified */
    if (ev->window == root_window) {
        dirty         = (screen_width != ev->width || screen_height != ev->height);
        screen_width  = ev->width;
        screen_height = ev->height;
        if (monitor_update_geometries() || dirty) {
            drw_resize(drw, screen_width, bar_height);
            bar_update_bars();
            for (m = monitor_list; m; m = m->next) {
                for (c = m->clients; c; c = c->next) {
                    if (c->isfullscreen && !c->isfakefullscreen) {
                        client_resize_client(c, m->mx, m->my, m->mw, m->mh);
                    }
                }
                XMoveResizeWindow(display, m->bar_window, m->wx + bar_side_padding, m->by + bar_ver_padding,
                                  bar_width(m), bar_height);
            }
            client_focus(NULL);
            layout_arrange(NULL);
        }
    }
}

/**
 * 销毁通知
 */
void destroy_notify(XEvent *e)
{
    Client              *c;
    XDestroyWindowEvent *ev = &e->xdestroywindow;

    if ((c = window_to_client(ev->window))) {
        client_unmanage(c, 1);
    } else if (show_systray && (c = window_to_systray_icon(ev->window))) {
        systray_remove_icon(c);
        systray_update(1);
    }
}

/**
 * 进入通知
 */
void enter_notify(XEvent *e)
{
    Client         *c;
    Monitor        *m;
    XCrossingEvent *ev = &e->xcrossing;

    if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root_window) {
        return;
    }
    c = window_to_client(ev->window);
    m = c ? c->mon : window_to_monitor(ev->window);
    if (m != select_monitor) {
        client_unfocus(select_monitor->select, 1);
        select_monitor = m;
    } else if (!c || c == select_monitor->select) {
        return;
    }
    client_focus(c);
}

/**
 * 暴露事件
 */
void expose(XEvent *e)
{
    Monitor      *m;
    XExposeEvent *ev = &e->xexpose;

    if (ev->count == 0 && (m = window_to_monitor(ev->window))) {
        bar_draw_bar(m);

        if (show_systray && m == systray_to_monitor(m)) {
            systray_update(0);
        }
    }
}

/**
 * 焦点进入
 */
void focusin(XEvent *e)
{
    XFocusChangeEvent *ev = &e->xfocus;

    if (select_monitor->select && ev->window != select_monitor->select->win)
        client_set_focus(select_monitor->select);
}

/**
 * 按键按下事件
 */
void key_press(XEvent *e)
{
    unsigned int i;
    KeySym       keysym;
    XKeyEvent   *ev;

    ev     = &e->xkey;
    keysym = XKeycodeToKeysym(display, (KeyCode)ev->keycode, 0);
    for (i = 0; i < keys_count(); i++) {
        if (keysym == keys[i].keysym && CLEANMASK(keys[i].mod) == CLEANMASK(ev->state) && keys[i].func) {
            keys[i].func(&(keys[i].arg));
        }
    }
}

/**
 * 映射通知
 */
void mapping_notify(XEvent *e)
{
    XMappingEvent *ev = &e->xmapping;

    XRefreshKeyboardMapping(ev);
    if (ev->request == MappingKeyboard) {
        grab_keys();
    }
}

/**
 * 映射请求
 */
void map_request(XEvent *e)
{
    static XWindowAttributes wa;
    XMapRequestEvent        *ev = &e->xmaprequest;

    Client *i;
    if (show_systray && (i = window_to_systray_icon(ev->window))) {
        window_send_event(i->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_WINDOW_ACTIVATE, 0,
                          systray->win, XEMBED_EMBEDDED_VERSION);
        systray_update(1);
    }

    if (!XGetWindowAttributes(display, ev->window, &wa) || wa.override_redirect) {
        return;
    }
    if (!window_to_client(ev->window)) {
        client_manage(ev->window, &wa);
    }
}

/**
 * 移动通知
 */
void motion_notify(XEvent *e)
{
    static Monitor *mon = NULL;
    Monitor        *m;
    XMotionEvent   *ev = &e->xmotion;

    if (ev->window != root_window) {
        return;
    }
    if ((m = monitor_rect_to_monitor(ev->x_root, ev->y_root, 1, 1)) != mon && mon) {
        client_unfocus(select_monitor->select, 1);
        select_monitor = m;
        client_focus(NULL);
    }
    mon = m;
}

/**
 * 属性通知
 */
void property_notify(XEvent *e)
{
    Client         *c;
    Window          trans;
    XPropertyEvent *ev = &e->xproperty;

    if (show_systray && (c = window_to_systray_icon(ev->window))) {
        if (ev->atom == XA_WM_NORMAL_HINTS) {
            client_update_size_hints(c);
            systray_update_icon_geom(c, c->w, c->h);
        } else {
            systray_update_icon_state(c, ev);
        }
        systray_update(1);
    }

    if ((ev->window == root_window) && (ev->atom == XA_WM_NAME)) {
        status_bar_update_status();
    } else if (ev->state == PropertyDelete) {
        return; /* ignore */
    } else if ((c = window_to_client(ev->window))) {
        switch (ev->atom) {
        default:
            break;
        case XA_WM_TRANSIENT_FOR:
            if (!c->isfloating && (XGetTransientForHint(display, c->win, &trans))
                && (c->isfloating = (window_to_client(trans)) != NULL))
            {
                layout_arrange(c->mon);
            }
            break;
        case XA_WM_NORMAL_HINTS:
            c->hintsvalid = 0;
            break;
        case XA_WM_HINTS:
            client_update_wm_hints(c);
            bar_draw_bars();
            break;
        }
        if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
            client_update_title(c);
            if (c == c->mon->select) {
                bar_draw_bar(c->mon);
            }
        } else if (ev->atom == netatom[NetWMIcon]) {
            client_update_icon(c);
            if (c == c->mon->select) {
                bar_draw_bar(c->mon);
            }
        }
        if (ev->atom == netatom[NetWMWindowType]) {
            client_update_window_type(c);
        }
    }
}

/**
 * 调整大小请求
 */
void resize_request(XEvent *e)
{
    XResizeRequestEvent *ev = &e->xresizerequest;
    Client              *i;

    if ((i = window_to_systray_icon(ev->window))) {
        systray_update_icon_geom(i, ev->width, ev->height);
        systray_update(1);
    }
}

/**
 * 取消映射通知
 */
void unmap_notify(XEvent *e)
{
    Client      *c;
    XUnmapEvent *ev = &e->xunmap;

    if ((c = window_to_client(ev->window))) {
        if (ev->send_event) {
            client_set_state(c, WithdrawnState);
        } else {
            client_unmanage(c, 0);
        }
    } else if (show_systray && (c = window_to_systray_icon(ev->window))) {
        /* KLUDGE! sometimes icons occasionally unmap their windows, but do
         * _not_ destroy them. We map those windows back */
        XMapRaised(display, c->win);
        systray_update(1);
    }
}

/**
 * 更新数字键盘锁按键掩码
 */
void update_numlock_mask(void)
{
    unsigned int     i, j;
    XModifierKeymap *modmap;

    numlockmask = 0;
    modmap      = XGetModifierMapping(display);  // 获取修饰符按键状态
    for (i = 0; i < 8; i++) {
        for (j = 0; j < modmap->max_keypermod; j++) {
            if (modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(display, XK_Num_Lock)) {
                numlockmask = (1 << i);
            }
        }
    }
    XFreeModifiermap(modmap);
}

/**
 * 注册组合键
 */
void grab_keys(void)
{
    update_numlock_mask();
    {
        unsigned int i, j;
        unsigned int modifiers[] = {0, LockMask, numlockmask, numlockmask | LockMask};
        KeyCode      code;

        // 释放全部组合键
        XUngrabKey(display, AnyKey, AnyModifier, root_window);

        // 注册组合键
        for (i = 0; i < keys_count(); i++) {
            if ((code = XKeysymToKeycode(display, keys[i].keysym))) {
                for (j = 0; j < LENGTH(modifiers); j++) {
                    XGrabKey(display, code, keys[i].mod | modifiers[j], root_window, True, GrabModeAsync,
                             GrabModeAsync);
                }
            }
        }
    }
}

/* There's no way to check accesses to destroyed windows, thus those cases are
 * ignored (especially on UnmapNotify's). Other types of errors call Xlibs
 * default error handler, which may call exit. */
int xerror(Display *dpy, XErrorEvent *ee)
{
    if (ee->error_code == BadWindow || (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
        || (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
        || (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
        || (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
        || (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
        || (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
        || (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
        || (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
    {
        return 0;
    }
    fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n", ee->request_code, ee->error_code);
    return xerrorxlib(dpy, ee); /* may call exit */
}

int xerrordummy(Display *dpy, XErrorEvent *ee)
{
    return 0;
}

/* Startup Error handler to check if another window manager
 * is already running. */
int xerrorstart(Display *dpy, XErrorEvent *ee)
{
    die("dwm: another window manager is already running");
    return -1;
}

/**
 * 检查是否存在其他窗口管理器
 */
void check_other_wm(void)
{
    // 注册错误处理函数
    xerrorxlib = XSetErrorHandler(xerrorstart);
    /* this causes an error if some other window manager is running */
    /* 如果其他一些窗口管理器正在运行会导致错误 */
    XSelectInput(display, DefaultRootWindow(display), SubstructureRedirectMask);

    // 刷新输出缓冲区并等待处理完成
    XSync(display, False);
    XSetErrorHandler(xerror);
    XSync(display, False);
}

/**
 * 立即清理僵尸进程
 */
void sigchld(int unused)
{
    if (signal(SIGCHLD, sigchld) == SIG_ERR) {
        die("can't install SIGCHLD handler:");
    }
    // 等待僵尸进程关闭
    while (0 < waitpid(-1, NULL, WNOHANG)) {
    }
}

/**
 *  初始化视觉
 */
void xinit_visual()
{
    XVisualInfo       *infos;
    XRenderPictFormat *fmt;
    int                nitems;
    int                i;

    XVisualInfo tpl   = {.screen = screen, .depth = 32, .class = TrueColor};
    long        masks = VisualScreenMask | VisualDepthMask | VisualClassMask;

    // 获取视觉信息
    infos  = XGetVisualInfo(display, masks, &tpl, &nitems);
    visual = NULL;
    for (i = 0; i < nitems; i++) {
        fmt = XRenderFindVisualFormat(display, infos[i].visual);
        if (fmt->type == PictTypeDirect && fmt->direct.alphaMask) {
            visual    = infos[i].visual;
            depth     = infos[i].depth;
            color_map = XCreateColormap(display, root_window, visual, AllocNone);
            useargb   = 1;
            break;
        }
    }

    XFree(infos);

    if (!visual) {
        visual    = DefaultVisual(display, screen);
        depth     = DefaultDepth(display, screen);
        color_map = DefaultColormap(display, screen);
    }
}

/**
 * 初始化
 */
void setup(void)
{
    /* 立即清理任何僵尸进程 */
    sigchld(0);

    /* init screen */
    screen        = DefaultScreen(display);
    screen_width  = DisplayWidth(display, screen);
    screen_height = DisplayHeight(display, screen);
    root_window   = RootWindow(display, screen);

    xinit_visual();
    drw = drw_create(display, screen, root_window, screen_width, screen_height, visual, depth, color_map);

    // 字符集载入
    if (!drw_fontset_create(drw, fonts, fonts_count())) {
        die("no fonts could be loaded.");
    }

    text_lr_pad = drw->fonts->h;
    bar_height  = drw->fonts->h + user_bar_height;
    monitor_update_geometries();
    bar_side_padding = sidepad;
    bar_ver_padding  = (topbar == 1) ? vertpad : -vertpad;

    /* init atoms */
    Atom utf8string     = XInternAtom(display, "UTF8_STRING", False);
    wmatom[WMProtocols] = XInternAtom(display, "WM_PROTOCOLS", False);
    wmatom[WMDelete]    = XInternAtom(display, "WM_DELETE_WINDOW", False);
    wmatom[WMState]     = XInternAtom(display, "WM_STATE", False);
    wmatom[WMTakeFocus] = XInternAtom(display, "WM_TAKE_FOCUS", False);

    netatom[NetActiveWindow]              = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
    netatom[NetSupported]                 = XInternAtom(display, "_NET_SUPPORTED", False);
    netatom[NetSystemTray]                = XInternAtom(display, "_NET_SYSTEM_TRAY_S0", False);
    netatom[NetSystemTrayOP]              = XInternAtom(display, "_NET_SYSTEM_TRAY_OPCODE", False);
    netatom[NetSystemTrayOrientation]     = XInternAtom(display, "_NET_SYSTEM_TRAY_ORIENTATION", False);
    netatom[NetSystemTrayOrientationHorz] = XInternAtom(display, "_NET_SYSTEM_TRAY_ORIENTATION_HORZ", False);
    netatom[NetSystemTrayVisual]          = XInternAtom(display, "_NET_SYSTEM_TRAY_VISUAL", False);
    netatom[NetWMName]                    = XInternAtom(display, "_NET_WM_NAME", False);
    netatom[NetWMIcon]                    = XInternAtom(display, "_NET_WM_ICON", False);
    netatom[NetWMState]                   = XInternAtom(display, "_NET_WM_STATE", False);
    netatom[NetWMCheck]                   = XInternAtom(display, "_NET_SUPPORTING_WM_CHECK", False);
    netatom[NetWMFullscreen]              = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
    netatom[NetWMWindowType]              = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    netatom[NetWMWindowTypeDock]          = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
    netatom[NetWMWindowTypeDialog]        = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    netatom[NetClientList]                = XInternAtom(display, "_NET_CLIENT_LIST", False);

    xatom[Manager]    = XInternAtom(display, "MANAGER", False);
    xatom[Xembed]     = XInternAtom(display, "_XEMBED", False);
    xatom[XembedInfo] = XInternAtom(display, "_XEMBED_INFO", False);

    /* init 光标 */
    cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
    cursor[CurResize] = drw_cur_create(drw, XC_sizing);
    cursor[CurMove]   = drw_cur_create(drw, XC_fleur);

    /* init appearance */
    scheme                 = ecalloc(colors_count() + 1, sizeof(Clr *));
    scheme[colors_count()] = drw_scm_create(drw, colors[0], alphas[0], 3);
    for (int i = 0; i < colors_count(); i++) {
        scheme[i] = drw_scm_create(drw, colors[i], alphas[i], 3);
    }

    /* 初始化系统托盘 */
    if (show_systray) {
        systray_update(0);
    }

    /* init bars */
    bar_update_bars();
    status_bar_update_status();
    bar_update_pos(select_monitor);

    /* supporting window for NetWMCheck */
    wmcheck_window = XCreateSimpleWindow(display, root_window, 0, 0, 1, 1, 0, 0, 0);
    XChangeProperty(display, wmcheck_window, netatom[NetWMCheck], XA_WINDOW, 32, PropModeReplace,
                    (unsigned char *)&wmcheck_window, 1);
    XChangeProperty(display, wmcheck_window, netatom[NetWMName], utf8string, 8, PropModeReplace, (unsigned char *)"dwm",
                    3);
    XChangeProperty(display, root_window, netatom[NetWMCheck], XA_WINDOW, 32, PropModeReplace,
                    (unsigned char *)&wmcheck_window, 1);
    /* EWMH support per view */
    XChangeProperty(display, root_window, netatom[NetSupported], XA_ATOM, 32, PropModeReplace, (unsigned char *)netatom,
                    NetLast);
    XDeleteProperty(display, root_window, netatom[NetClientList]);
    /* select events */
    XSetWindowAttributes wa;
    wa.cursor     = cursor[CurNormal]->cursor;
    wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask | ButtonPressMask | PointerMotionMask
                  | EnterWindowMask | LeaveWindowMask | StructureNotifyMask | PropertyChangeMask;
    XChangeWindowAttributes(display, root_window, CWEventMask | CWCursor, &wa);
    XSelectInput(display, root_window, wa.event_mask);

    grab_keys();  // 注册组合键
    client_focus(NULL);
}

/**
 * 扫描窗口
 */
void scan(void)
{
    unsigned int      i, num;
    Window            d1, d2, *wins = NULL;
    XWindowAttributes wa;

    if (XQueryTree(display, root_window, &d1, &d2, &wins, &num)) {
        for (i = 0; i < num; i++) {
            if (!XGetWindowAttributes(display, wins[i], &wa) || wa.override_redirect
                || XGetTransientForHint(display, wins[i], &d1))
            {
                continue;
            }
            if (wa.map_state == IsViewable || window_get_state(wins[i]) == IconicState) {
                client_manage(wins[i], &wa);
            }
        }

        for (i = 0; i < num; i++) { /* now the transients */
            if (!XGetWindowAttributes(display, wins[i], &wa)) {
                continue;
            }
            if (XGetTransientForHint(display, wins[i], &d1)
                && (wa.map_state == IsViewable || window_get_state(wins[i]) == IconicState))
            {
                client_manage(wins[i], &wa);
            }
        }

        if (wins) {
            XFree(wins);
        }
    }
}

/**
 * 运行自启动脚本
 */
void runautostart(void)
{
    char       *path;
    char       *xdgdatahome;
    char       *home;
    struct stat sb;

    home = getenv("HOME");
    if (home == NULL)
        return;

    // XDG_DATA_HOME存在则使用$XDG_DATA_HOME/dwm路径,不存在使用$HOME/.local/share/dwm
    xdgdatahome = getenv("XDG_DATA_HOME");
    if (xdgdatahome != NULL && *xdgdatahome != '\0') {
        dwm_script_path = ecalloc(1, strlen(xdgdatahome) + strlen(dwmdir) + 2);

        if (sprintf(dwm_script_path, "%s/%s", xdgdatahome, dwmdir) <= 0) {
            free(dwm_script_path);
            dwm_script_path = NULL;
            return;
        }
    } else {
        dwm_script_path = ecalloc(1, strlen(home) + strlen(localshare) + strlen(dwmdir) + 3);

        if (sprintf(dwm_script_path, "%s/%s/%s", home, localshare, dwmdir) < 0) {
            free(dwm_script_path);
            dwm_script_path = NULL;
            return;
        }
    }

    /* 检查自动启动脚本目录是否存在 */
    if (!(stat(dwm_script_path, &sb) == 0 && S_ISDIR(sb.st_mode))) {
        /* 符合 XDG 的路径不存在或不是目录, 尝试 ~/.dwm */
        char *pathpfx_new = realloc(dwm_script_path, strlen(home) + strlen(dwmdir) + 3);
        if (pathpfx_new == NULL) {
            free(dwm_script_path);
            dwm_script_path = NULL;
            return;
        }
        dwm_script_path = pathpfx_new;

        if (sprintf(dwm_script_path, "%s/.%s", home, dwmdir) <= 0) {
            free(dwm_script_path);
            dwm_script_path = NULL;
            return;
        }
    }

    path = ecalloc(1, strlen(dwm_script_path) + strlen(auto_start_script) + 2);
    if (sprintf(path, "%s/%s", dwm_script_path, auto_start_script) <= 0) {
        free(path);
        free(dwm_script_path);
        dwm_script_path = NULL;
    }

    if (access(path, X_OK) == 0) {
        system(strcat(path, " &"));
    }

    free(path);
}

/**
 * 主循环
 */
void run(void)
{
    XEvent ev;
    /* main event loop */
    XSync(display, False);
    while (running && !XNextEvent(display, &ev)) {
        if (handler[ev.type]) {
            handler[ev.type](&ev); /* call handler */
        }
    }
}

/**
 * 清除资源
 */
void cleanup(void)
{
    Arg      a   = {.ui = ~0};
    Layout   foo = {"", NULL};
    Monitor *m;
    size_t   i;

    view_tag(&a);
    select_monitor->lt[select_monitor->sellt] = &foo;
    for (m = monitor_list; m; m = m->next) {
        while (m->stack) {
            client_unmanage(m->stack, 0);
        }
    }
    XUngrabKey(display, AnyKey, AnyModifier, root_window);
    while (monitor_list) {
        monitor_cleanup(monitor_list);
    }

    systray_cleanup();

    for (i = 0; i < CurLast; i++) {
        drw_cur_free(drw, cursor[i]);
    }
    for (i = 0; i < colors_count() + 1; i++) {
        free(scheme[i]);
    }
    free(scheme);
    XDestroyWindow(display, wmcheck_window);
    drw_free(drw);
    XSync(display, False);
    XSetInputFocus(display, PointerRoot, RevertToPointerRoot, CurrentTime);
    XDeleteProperty(display, root_window, netatom[NetActiveWindow]);
}

int main(int argc, char *argv[])
{
    if (argc == 2 && !strcmp("-v", argv[1])) {
        die("dwm-" VERSION);
    } else if (argc != 1) {
        die("usage: dwm [-v]");
    }

    if (!setlocale(LC_CTYPE, "") || !XSupportsLocale()) {
        fputs("warning: no locale support\n", stderr);
    }

    // 打开默认显示器
    if (!(display = XOpenDisplay(NULL))) {
        die("dwm: cannot open display");
    }

    // 检查是否存在其他窗口管理器
    check_other_wm();

    // 扫描窗口
    setup();

    // 扫描窗口
    scan();

    // 运行自启动脚本
    runautostart();

    // 主循环
    run();

    cleanup();

    XCloseDisplay(display);

    return EXIT_SUCCESS;
}

/**
 * 执行命令
 */
void spawn(const Arg *arg)
{
    select_monitor->tagset[select_monitor->seltags] &= ~scratchtag;
    if (fork() == 0) {
        if (display) {
            close(ConnectionNumber(display));
        }
        setsid();
        execvp(((char **)arg->v)[0], (char **)arg->v);
        die("dwm: execvp '%s' failed:", ((char **)arg->v)[0]);
    }
}

/**
 * 运行app
 */
void app_starter(const Arg *arg)
{
    if (dwm_script_path != NULL) {
        char *app = (char *)arg->v;
        char *path = ecalloc(1, strlen(dwm_script_path) + strlen(app_starter_sh) + strlen(app) + 2);
        if (sprintf(path, "%s/%s %s", dwm_script_path, app_starter_sh, app) <= 0) {
            free(path);
            free(dwm_script_path);
        }
        spawn(&(Arg){.v = (const char *[]){"/bin/sh", "-c", path, NULL}});
        free(path);
    }
}

/**
 * 退出dwm
 */
void quit(const Arg *arg)
{
    // fix: reloading dwm keeps all the hidden clients hidden
    Monitor *m;
    Client  *c;
    for (m = monitor_list; m; m = m->next) {
        if (m) {
            for (c = m->stack; c; c = c->next) {
                if (c && HIDDEN(c)) {
                    window_show(c);
                }
            }
        }
    }

    FILE       *fd = NULL;
    struct stat filestat;

    if ((fd = fopen(lockfile, "r")) && stat(lockfile, &filestat) == 0) {
        fclose(fd);

        if (filestat.st_ctime <= time(NULL) - 2) {
            remove(lockfile);
        }
    }

    if ((fd = fopen(lockfile, "r")) != NULL) {
        fclose(fd);
        remove(lockfile);
        running = 0;
    } else {
        if ((fd = fopen(lockfile, "a")) != NULL) {
            fclose(fd);
        }
    }
}

/**
 * 切换便签薄
 */
void toggle_scratch(const Arg *arg)
{
    Client      *c;
    unsigned int found = 0;

    for (c = select_monitor->clients; c && !(found = c->tags & scratchtag); c = c->next)
        ;
    if (found) {
        unsigned int newtagset = select_monitor->tagset[select_monitor->seltags] ^ scratchtag;
        if (newtagset) {
            select_monitor->tagset[select_monitor->seltags] = newtagset;
            client_focus(NULL);
            layout_arrange(select_monitor);
        }
        if (ISVISIBLE(c)) {
            client_focus(c);
            client_restack(select_monitor);
        }
    } else {
        app_starter(arg);
    }
}
