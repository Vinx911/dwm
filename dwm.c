
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

/* variables */
const char autostartblocksh[] = "autostart_blocking.sh";
const char autostartsh[]      = "autostart.sh";
const char broken[]           = "broken";       /* 无法获取到窗口标题时显示文本 */
const char dwmdir[]           = "dwm";          /* dwm目录 */
const char localshare[]       = ".local/share"; /* .local/share */
char       status_text[1024];                   /* 状态栏文本 */
int        screen;                              /* 默认屏幕 */
int        screen_width;                        /* 默认屏幕的宽 */
int        screen_height;                       /* 默认屏幕的高 */
int        bar_height;                          /* bar 高度 */
int        tag_bar_width;                       /* 标签栏宽度 */
int        lt_symbol_width;                     /* 布局符号宽度 */
int        statsu_bar_width;                    /* 状态栏宽度 */
int        lrpad;                               /* 文本左右填充的总和 */
int        vp;                                  /* vertical padding for bar */
int        sp;                                  /* side padding for bar */
int (*xerrorxlib)(Display *, XErrorEvent *);
unsigned int numlockmask             = 0; /* 数字键盘锁按键掩码 */
unsigned int monocleshowcount        = 0; /* monocle显示窗口个数 */
void (*handler[LASTEvent])(XEvent *) = {
    // 事件数组
    [ButtonPress]      = button_press,
    [ClientMessage]    = clientmessage,
    [ConfigureRequest] = configurerequest,
    [ConfigureNotify]  = configurenotify,
    [DestroyNotify]    = destroynotify,
    [EnterNotify]      = enternotify,
    [Expose]           = expose,
    [FocusIn]          = focusin,
    [KeyPress]         = keypress,
    [MappingNotify]    = mappingnotify,
    [MapRequest]       = maprequest,
    [MotionNotify]     = motionnotify,
    [PropertyNotify]   = propertynotify,
    [ResizeRequest]    = resizerequest,
    [UnmapNotify]      = unmapnotify,
};
Atom     wmatom[WMLast], netatom[NetLast], xatom[XLast];
int      running = 1;
Cur     *cursor[CurLast]; /* 光标 */
Clr    **scheme;          /* 配色 */
Display *dpy;             /* 默认显示器 */
Drw     *drw;
Monitor *mons;           /* 监视器列表 */
Monitor *select_monitor; /* 当前选择的监视器 */
Window   root;           /* 根窗口 */
Window   wmcheckwin;

int      useargb = 0; /* 使用argb */
Visual  *visual;      /* 视觉 */
int      depth;       /* 颜色位深 */
Colormap cmap;        /* 颜色映射 */

/* configuration, allows nested code to access above variables */
// #include "config.h"



static unsigned int scratchtag = 1 << TAGS_COUNT; /* 便签簿标签 */

/* compile-time check if all tags fit into an unsigned int bit array. */
// struct NumTags
// {
//     char limitexceeded[TAGS_COUNT > 31 ? -1 : 1];
// };

/* function implementations */
/**
 * 应用规则
 */
void applyrules(Client *c)
{
    const char *class, *instance;
    unsigned int i;
    const Rule  *r;
    Monitor     *m;
    XClassHint   ch = {NULL, NULL};

    /* rule matching */
    c->isfloating = 0;
    c->tags       = 0;
    XGetClassHint(dpy, c->win, &ch);
    class    = ch.res_class ? ch.res_class : broken;
    instance = ch.res_name ? ch.res_name : broken;

    for (i = 0; i < rules_count(); i++) {
        r = &rules[i];
        if ((!r->title || strstr(c->name, r->title)) && (!r->class || strstr(class, r->class))
            && (!r->instance || strstr(instance, r->instance)))
        {
            c->isfloating       = r->isfloating;
            c->isfakefullscreen = r->isfakefullscreen;
            c->nooverview       = r->nooverview;
            c->tags |= r->tags;
            for (m = mons; m && m->num != r->monitor; m = m->next)
                ;
            if (m) {
                c->mon = m;
            }
            // 如果设定了floatposition 且未指定xy，设定窗口位置
            if (r->isfloating && c->x == 0 && c->y == 0) {
                switch (r->floatposition) {
                case 1:
                    c->x = select_monitor->wx + gappx;
                    c->y = select_monitor->wy + gappx;
                    break;  // 左上
                case 2:
                    c->x = select_monitor->wx + (select_monitor->ww - WIDTH(c)) / 2 - gappx;
                    c->y = select_monitor->wy + gappx;
                    break;  // 中上
                case 3:
                    c->x = select_monitor->wx + select_monitor->ww - WIDTH(c) - gappx;
                    c->y = select_monitor->wy + gappx;
                    break;  // 右上
                case 4:
                    c->x = select_monitor->wx + gappx;
                    c->y = select_monitor->wy + (select_monitor->wh - HEIGHT(c)) / 2;
                    break;  // 左中
                case 0:     // 默认0，居中
                case 5:
                    c->x = select_monitor->wx + (select_monitor->ww - WIDTH(c)) / 2;
                    c->y = select_monitor->wy + (select_monitor->wh - HEIGHT(c)) / 2;
                    break;  // 中中
                case 6:
                    c->x = select_monitor->wx + select_monitor->ww - WIDTH(c) - gappx;
                    c->y = select_monitor->wy + (select_monitor->wh - HEIGHT(c)) / 2;
                    break;  // 右中
                case 7:
                    c->x = select_monitor->wx + gappx;
                    c->y = select_monitor->wy + select_monitor->wh - HEIGHT(c) - gappx;
                    break;  // 左下
                case 8:
                    c->x = select_monitor->wx + (select_monitor->ww - WIDTH(c)) / 2;
                    c->y = select_monitor->wy + select_monitor->wh - HEIGHT(c) - gappx;
                    break;  // 中下
                case 9:
                    c->x = select_monitor->wx + select_monitor->ww - WIDTH(c) - gappx;
                    c->y = select_monitor->wy + select_monitor->wh - HEIGHT(c) - gappx;
                    break;  // 右下
                }
            }
            break;  // 有且只会匹配一个第一个符合的rule
        }
    }
    if (ch.res_class) {
        XFree(ch.res_class);
    }
    if (ch.res_name) {
        XFree(ch.res_name);
    }
    c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : c->mon->tagset[c->mon->seltags];
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
    XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureRedirectMask);

    // 刷新输出缓冲区并等待处理完成
    XSync(dpy, False);
    XSetErrorHandler(xerror);
    XSync(dpy, False);
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

    view(&a);
    select_monitor->lt[select_monitor->sellt] = &foo;
    for (m = mons; m; m = m->next) {
        while (m->stack) {
            unmanage(m->stack, 0);
        }
    }
    XUngrabKey(dpy, AnyKey, AnyModifier, root);
    while (mons) {
        cleanup_monitor(mons);
    }

    cleanup_systray();

    for (i = 0; i < CurLast; i++) {
        drw_cur_free(drw, cursor[i]);
    }
    for (i = 0; i < colors_count() + 1; i++) {
        free(scheme[i]);
    }
    free(scheme);
    XDestroyWindow(dpy, wmcheckwin);
    drw_free(drw);
    XSync(dpy, False);
    XSetInputFocus(dpy, PointerRoot, RevertToPointerRoot, CurrentTime);
    XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
}


/**
 * 客户端消息
 */
void clientmessage(XEvent *e)
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
            setfullscreen(c, (cme->data.l[0] == 1     /* _NET_WM_STATE_ADD    */
                              || (cme->data.l[0] == 2 /* _NET_WM_STATE_TOGGLE */
                                  && (!c->isfullscreen || c->isfakefullscreen))));
        }
    } else if (cme->message_type == netatom[NetActiveWindow]) {
        if (c != select_monitor->select && !c->isurgent) {
            seturgent(c, 1);
        }

        if (c == select_monitor->select) {
            return;
        }
        // 若不是当前显示器 则跳转到对应显示器
        if (c->mon != select_monitor) {
            focusmon(&(Arg){.i = +1});
        }
        // 若不适当前tag 则跳转到对应tag
        if (!ISVISIBLE(c)) {
            view(&(Arg){.ui = c->tags});
        }
        // 选中窗口
        focus(c);
        pointerfocuswin(c);
    }
}


/**
 * 配置通知
 */
void configurenotify(XEvent *e)
{
    Monitor         *m;
    Client          *c;
    XConfigureEvent *ev = &e->xconfigure;
    int              dirty;

    /* TODO: updategeom handling sucks, needs to be simplified */
    if (ev->window == root) {
        dirty         = (screen_width != ev->width || screen_height != ev->height);
        screen_width  = ev->width;
        screen_height = ev->height;
        if (updategeom() || dirty) {
            drw_resize(drw, screen_width, bar_height);
            updatebars();
            for (m = mons; m; m = m->next) {
                for (c = m->clients; c; c = c->next) {
                    if (c->isfullscreen && !c->isfakefullscreen) {
                        resizeclient(c, m->mx, m->my, m->mw, m->mh);
                    }
                }
                XMoveResizeWindow(dpy, m->bar_window, m->wx + sp, m->by + vp, m->ww - 2 * sp, bar_height);
            }
            focus(NULL);
            arrange(NULL);
        }
    }
}

/**
 * 配置请求
 */
void configurerequest(XEvent *e)
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
                configure(c);
            }
            if (ISVISIBLE(c)) {
                XMoveResizeWindow(dpy, c->win, c->x, c->y, c->w, c->h);
            }
        } else {
            configure(c);
        }
    } else {
        wc.x            = ev->x;
        wc.y            = ev->y;
        wc.width        = ev->width;
        wc.height       = ev->height;
        wc.border_width = ev->border_width;
        wc.sibling      = ev->above;
        wc.stack_mode   = ev->detail;
        XConfigureWindow(dpy, ev->window, ev->value_mask, &wc);
    }
    XSync(dpy, False);
}

/**
 * 销毁通知
 */
void destroynotify(XEvent *e)
{
    Client              *c;
    XDestroyWindowEvent *ev = &e->xdestroywindow;

    if ((c = window_to_client(ev->window))) {
        unmanage(c, 1);
    } else if (show_systray && (c = window_to_systray_icon(ev->window))) {
        remove_systray_icon(c);
        update_systray(1);
    }
}




/**
 * 进入通知
 */
void enternotify(XEvent *e)
{
    Client         *c;
    Monitor        *m;
    XCrossingEvent *ev = &e->xcrossing;

    if ((ev->mode != NotifyNormal || ev->detail == NotifyInferior) && ev->window != root) {
        return;
    }
    c = window_to_client(ev->window);
    m = c ? c->mon : window_to_monitor(ev->window);
    if (m != select_monitor) {
        unfocus(select_monitor->select, 1);
        select_monitor = m;
    } else if (!c || c == select_monitor->select) {
        return;
    }
    focus(c);
}

void expose(XEvent *e)
{
    Monitor      *m;
    XExposeEvent *ev = &e->xexpose;

    if (ev->count == 0 && (m = window_to_monitor(ev->window))) {
        drawbar(m);

        if (show_systray && m == systray_to_monitor(m)) {
            update_systray(0);
        }
    }
}

/**
 * 焦点位置
 */
void focus(Client *c)
{
    if (!c || !ISVISIBLE(c)) {
        // 客户端不存在或者不显示时,切换焦点到第一个可显示的客户端
        for (c = select_monitor->stack; c && (!ISVISIBLE(c) || HIDDEN(c)); c = c->snext)
            ;
    }

    if (select_monitor->select && select_monitor->select != c) {
        unfocus(select_monitor->select, 0);

        if (select_monitor->hidsel) {  // 之前焦点窗口为隐藏窗口, 焦点取消后继续隐藏
            hidewin(select_monitor->select);
            if (c) {
                arrange(c->mon);  // 隐藏窗口后重新布局
            }
            select_monitor->hidsel = 0;
        }
    }

    if (c) {
        if (c->mon != select_monitor) {
            select_monitor = c->mon;
        }
        if (c->isurgent) {
            seturgent(c, 0);
        }
        detachstack(c);
        attachstack(c);  // 重新附加客户端栈
        grabbuttons(c, 1);
        XSetWindowBorder(dpy, c->win, scheme[SchemeSel][ColBorder].pixel);
        setfocus(c);
    } else {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
    }

    select_monitor->select = c;
    drawbars();  // 更新Bar
}

/* there are some broken focus acquiring clients needing extra handling */
void focusin(XEvent *e)
{
    XFocusChangeEvent *ev = &e->xfocus;

    if (select_monitor->select && ev->window != select_monitor->select->win)
        setfocus(select_monitor->select);
}

void focusmon(const Arg *arg)
{
    Monitor *m;

    if (!mons->next) {
        return;
    }
    if ((m = dirtomon(arg->i)) == select_monitor) {
        return;
    }
    unfocus(select_monitor->select, 0);
    select_monitor = m;
    focus(NULL);
    pointerfocuswin(NULL);
}

/**
 * 切换显示窗口焦点
 */
void focusstackvis(const Arg *arg)
{
    focusstack(arg->i, 0);
}

/**
 * 切换隐藏窗口焦点
 */
void focusstackhid(const Arg *arg)
{
    focusstack(arg->i, 1);
}

void focusstack(int inc, int hid)
{
    Client *c = NULL, *i;
    // if no client selected AND exclude hidden client; if client selected but
    // fullscreened
    if ((!select_monitor->select && !hid)
        || (select_monitor->select && select_monitor->select->isfullscreen && lockfullscreen))
    {
        return;
    }
    if (!select_monitor->clients) {
        return;
    }
    if (inc > 0) {
        if (select_monitor->select) {
            for (c = select_monitor->select->next; c && (!ISVISIBLE(c) || (!hid && HIDDEN(c))); c = c->next)
                ;
        }
        if (!c) {
            for (c = select_monitor->clients; c && (!ISVISIBLE(c) || (!hid && HIDDEN(c))); c = c->next)
                ;
        }
    } else {
        if (select_monitor->select) {
            for (i = select_monitor->clients; i != select_monitor->select; i = i->next) {
                if (ISVISIBLE(i) && !(!hid && HIDDEN(i))) {
                    c = i;
                }
            }
        } else {
            c = select_monitor->clients;
        }
        if (!c)
            for (; i; i = i->next) {
                if (ISVISIBLE(i) && !(!hid && HIDDEN(i))) {
                    c = i;
                }
            }
    }
    if (c) {
        focus(c);
        restack(select_monitor);
        if (HIDDEN(c)) {
            showwin(c);
            c->mon->hidsel = 1;
        }
        pointerfocuswin(c);
    }
}



/**
 * 获取根点
 */
int getrootptr(int *x, int *y)
{
    int          di;
    unsigned int dui;
    Window       dummy;

    return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

/**
 * 注册鼠标按键
 */
void grabbuttons(Client *c, int focused)
{
    updatenumlockmask();
    {
        unsigned int i, j;
        unsigned int modifiers[] = {0, LockMask, numlockmask, numlockmask | LockMask};
        XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
        if (!focused) {
            XGrabButton(dpy, AnyButton, AnyModifier, c->win, False, BUTTONMASK, GrabModeSync, GrabModeSync, None, None);
        }
        for (i = 0; i < buttons_count(); i++) {
            if (buttons[i].click == ClkClientWin) {
                for (j = 0; j < LENGTH(modifiers); j++) {
                    XGrabButton(dpy, buttons[i].button, buttons[i].mask | modifiers[j], c->win, False, BUTTONMASK,
                                GrabModeAsync, GrabModeSync, None, None);
                }
            }
        }
    }
}

/**
 * 注册组合键
 */
void grabkeys(void)
{
    updatenumlockmask();
    {
        unsigned int i, j;
        unsigned int modifiers[] = {0, LockMask, numlockmask, numlockmask | LockMask};
        KeyCode      code;

        // 释放全部组合键
        XUngrabKey(dpy, AnyKey, AnyModifier, root);

        // 注册组合键
        for (i = 0; i < keys_count(); i++) {
            if ((code = XKeysymToKeycode(dpy, keys[i].keysym))) {
                for (j = 0; j < LENGTH(modifiers); j++) {
                    XGrabKey(dpy, code, keys[i].mod | modifiers[j], root, True, GrabModeAsync, GrabModeAsync);
                }
            }
        }
    }
}


/**
 * 增加/减少主窗口个数
 */
void incnmaster(const Arg *arg)
{
    int nmaster = select_monitor->nmaster + arg->i;
    if (select_monitor->task_count <= 1) {
        nmaster = 1;
    } else if (nmaster >= 3) {
        nmaster = 1;
    }
    select_monitor->nmaster = select_monitor->pertag->layout[select_monitor->pertag->curtag].nmasters = MAX(nmaster, 1);
    arrange(select_monitor);
}


/**
 * 按键按下事件
 */
void keypress(XEvent *e)
{
    unsigned int i;
    KeySym       keysym;
    XKeyEvent   *ev;

    ev     = &e->xkey;
    keysym = XKeycodeToKeysym(dpy, (KeyCode)ev->keycode, 0);
    for (i = 0; i < keys_count(); i++) {
        if (keysym == keys[i].keysym && CLEANMASK(keys[i].mod) == CLEANMASK(ev->state) && keys[i].func) {
            keys[i].func(&(keys[i].arg));
        }
    }
}


/**
 * 管理窗口
 */
void manage(Window w, XWindowAttributes *wa)
{
    Client        *c, *t = NULL;
    Window         trans = None;
    XWindowChanges wc;

    c      = ecalloc(1, sizeof(Client));
    c->win = w;
    /* geometry */
    c->x = c->oldx = wa->x;
    c->y = c->oldy = wa->y;
    c->w = c->oldw = wa->width;
    c->h = c->oldh = wa->height;
    c->oldbw       = wa->border_width;

    updateicon(c);
    updatetitle(c);
    if (XGetTransientForHint(dpy, w, &trans) && (t = window_to_client(trans))) {
        c->mon  = t->mon;
        c->tags = t->tags;
    } else {
        c->mon = select_monitor;
        applyrules(c);
    }

    if (c->x + WIDTH(c) > c->mon->wx + c->mon->ww) {
        c->x = c->mon->wx + c->mon->ww - WIDTH(c);
    }
    if (c->y + HEIGHT(c) > c->mon->wy + c->mon->wh) {
        c->y = c->mon->wy + c->mon->wh - HEIGHT(c);
    }
    c->x  = MAX(c->x, c->mon->wx);
    c->y  = MAX(c->y, c->mon->wy);
    c->bw = borderpx;

    select_monitor->tagset[select_monitor->seltags] &= ~scratchtag;
    if (!strcmp(c->name, scratchpadname)) {  // 便笺薄
        c->mon->tagset[c->mon->seltags] |= c->tags = scratchtag;
        c->isfloating                              = True;
        c->nooverview                              = True;
        c->x                                       = c->mon->wx + (c->mon->ww / 2 - WIDTH(c) / 2);
        c->y                                       = c->mon->wy + (c->mon->wh / 2 - HEIGHT(c) / 2);
    }

    wc.border_width = c->bw;
    XConfigureWindow(dpy, w, CWBorderWidth, &wc);
    XSetWindowBorder(dpy, w, scheme[SchemeNorm][ColBorder].pixel);
    configure(c); /* propagates border_width, if size doesn't change */
    updatewindowtype(c);
    updatesizehints(c);
    updatewmhints(c);
    XSelectInput(dpy, w, EnterWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask);
    grabbuttons(c, 0);
    if (!c->isfloating) {
        c->isfloating = c->oldstate = trans != None || c->isfixed;
    }
    if (c->isfloating) {
        XRaiseWindow(dpy, c->win);
    }
    attach(c);
    attachstack(c);
    XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend, (unsigned char *)&(c->win), 1);
    XMoveResizeWindow(dpy, c->win, c->x + 2 * screen_width, c->y, c->w, c->h); /* some windows require this */
    if (!HIDDEN(c)) {
        setclientstate(c, NormalState);
    }
    if (c->mon == select_monitor) {
        unfocus(select_monitor->select, 0);
    }
    c->mon->select = c;
    arrange(c->mon);
    if (!HIDDEN(c)) {
        XMapWindow(dpy, c->win);
    }
    focus(NULL);
}

/**
 * 映射通知
 */
void mappingnotify(XEvent *e)
{
    XMappingEvent *ev = &e->xmapping;

    XRefreshKeyboardMapping(ev);
    if (ev->request == MappingKeyboard) {
        grabkeys();
    }
}

/**
 * 映射请求
 */
void maprequest(XEvent *e)
{
    static XWindowAttributes wa;
    XMapRequestEvent        *ev = &e->xmaprequest;

    Client *i;
    if (show_systray && (i = window_to_systray_icon(ev->window))) {
        sendevent(i->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_WINDOW_ACTIVATE, 0, systray->win,
                  XEMBED_EMBEDDED_VERSION);
        update_systray(1);
    }

    if (!XGetWindowAttributes(dpy, ev->window, &wa) || wa.override_redirect) {
        return;
    }
    if (!window_to_client(ev->window)) {
        manage(ev->window, &wa);
    }
}


/**
 * 移动通知
 */
void motionnotify(XEvent *e)
{
    static Monitor *mon = NULL;
    Monitor        *m;
    XMotionEvent   *ev = &e->xmotion;

    if (ev->window != root) {
        return;
    }
    if ((m = recttomon(ev->x_root, ev->y_root, 1, 1)) != mon && mon) {
        unfocus(select_monitor->select, 1);
        select_monitor = m;
        focus(NULL);
    }
    mon = m;
}



void pointerfocuswin(Client *c)
{
    if (c) {
        XWarpPointer(dpy, None, root, 0, 0, 0, 0, c->x + c->w / 2, c->y + c->h / 2);
        focus(c);
    } else {
        XWarpPointer(dpy, None, root, 0, 0, 0, 0, select_monitor->wx + select_monitor->ww / 3,
                     select_monitor->wy + select_monitor->wh / 2);
    }
}

/**
 * 属性通知
 */
void propertynotify(XEvent *e)
{
    Client         *c;
    Window          trans;
    XPropertyEvent *ev = &e->xproperty;

    if (show_systray && (c = window_to_systray_icon(ev->window))) {
        if (ev->atom == XA_WM_NORMAL_HINTS) {
            updatesizehints(c);
            update_systray_icon_geom(c, c->w, c->h);
        } else {
            update_systray_icon_state(c, ev);
        }
        update_systray(1);
    }

    if ((ev->window == root) && (ev->atom == XA_WM_NAME)) {
        update_status();
    } else if (ev->state == PropertyDelete) {
        return; /* ignore */
    } else if ((c = window_to_client(ev->window))) {
        switch (ev->atom) {
        default:
            break;
        case XA_WM_TRANSIENT_FOR:
            if (!c->isfloating && (XGetTransientForHint(dpy, c->win, &trans))
                && (c->isfloating = (window_to_client(trans)) != NULL))
            {
                arrange(c->mon);
            }
            break;
        case XA_WM_NORMAL_HINTS:
            c->hintsvalid = 0;
            break;
        case XA_WM_HINTS:
            updatewmhints(c);
            drawbars();
            break;
        }
        if (ev->atom == XA_WM_NAME || ev->atom == netatom[NetWMName]) {
            updatetitle(c);
            if (c == c->mon->select) {
                drawbar(c->mon);
            }
        } else if (ev->atom == netatom[NetWMIcon]) {
            updateicon(c);
            if (c == c->mon->select) {
                drawbar(c->mon);
            }
        }
        if (ev->atom == netatom[NetWMWindowType]) {
            updatewindowtype(c);
        }
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
    for (m = mons; m; m = m->next) {
        if (m) {
            for (c = m->stack; c; c = c->next) {
                if (c && HIDDEN(c)) {
                    showwin(c);
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
 * 调整大小请求
 */
void resizerequest(XEvent *e)
{
    XResizeRequestEvent *ev = &e->xresizerequest;
    Client              *i;

    if ((i = window_to_systray_icon(ev->window))) {
        update_systray_icon_geom(i, ev->width, ev->height);
        update_systray(1);
    }
}



/**
 * 主循环
 */
void run(void)
{
    XEvent ev;
    /* main event loop */
    XSync(dpy, False);
    while (running && !XNextEvent(dpy, &ev)) {
        if (handler[ev.type]) {
            handler[ev.type](&ev); /* call handler */
        }
    }
}

/**
 * 运行自启动脚本
 */
void runautostart(void)
{
    char       *pathpfx;
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
        pathpfx = ecalloc(1, strlen(xdgdatahome) + strlen(dwmdir) + 2);

        if (sprintf(pathpfx, "%s/%s", xdgdatahome, dwmdir) <= 0) {
            free(pathpfx);
            return;
        }
    } else {
        pathpfx = ecalloc(1, strlen(home) + strlen(localshare) + strlen(dwmdir) + 3);

        if (sprintf(pathpfx, "%s/%s/%s", home, localshare, dwmdir) < 0) {
            free(pathpfx);
            return;
        }
    }

    /* 检查自动启动脚本目录是否存在 */
    if (!(stat(pathpfx, &sb) == 0 && S_ISDIR(sb.st_mode))) {
        /* 符合 XDG 的路径不存在或不是目录, 尝试 ~/.dwm */
        char *pathpfx_new = realloc(pathpfx, strlen(home) + strlen(dwmdir) + 3);
        if (pathpfx_new == NULL) {
            free(pathpfx);
            return;
        }
        pathpfx = pathpfx_new;

        if (sprintf(pathpfx, "%s/.%s", home, dwmdir) <= 0) {
            free(pathpfx);
            return;
        }
    }

    /* try the blocking script first */
    path = ecalloc(1, strlen(pathpfx) + strlen(autostartblocksh) + 2);
    if (sprintf(path, "%s/%s", pathpfx, autostartblocksh) <= 0) {
        free(path);
        free(pathpfx);
    }

    if (access(path, X_OK) == 0) {
        system(path);
    }

    /* now the non-blocking script */
    if (sprintf(path, "%s/%s", pathpfx, autostartsh) <= 0) {
        free(path);
        free(pathpfx);
    }

    if (access(path, X_OK) == 0) {
        system(strcat(path, " &"));
    }

    free(pathpfx);
    free(path);
}

/**
 * 扫描窗口
 */
void scan(void)
{
    unsigned int      i, num;
    Window            d1, d2, *wins = NULL;
    XWindowAttributes wa;

    if (XQueryTree(dpy, root, &d1, &d2, &wins, &num)) {
        for (i = 0; i < num; i++) {
            if (!XGetWindowAttributes(dpy, wins[i], &wa) || wa.override_redirect
                || XGetTransientForHint(dpy, wins[i], &d1))
            {
                continue;
            }
            if (wa.map_state == IsViewable || getstate(wins[i]) == IconicState) {
                manage(wins[i], &wa);
            }
        }

        for (i = 0; i < num; i++) { /* now the transients */
            if (!XGetWindowAttributes(dpy, wins[i], &wa)) {
                continue;
            }
            if (XGetTransientForHint(dpy, wins[i], &d1)
                && (wa.map_state == IsViewable || getstate(wins[i]) == IconicState))
            {
                manage(wins[i], &wa);
            }
        }

        if (wins) {
            XFree(wins);
        }
    }
}


/**
 * 发送事件
 */
int sendevent(Window w, Atom proto, int mask, long d0, long d1, long d2, long d3, long d4)
{
    int    n;
    Atom  *protocols, mt;
    int    exists = 0;
    XEvent ev;

    if (proto == wmatom[WMTakeFocus] || proto == wmatom[WMDelete]) {
        mt = wmatom[WMProtocols];
        if (XGetWMProtocols(dpy, w, &protocols, &n)) {
            while (!exists && n--)
                exists = protocols[n] == proto;
            XFree(protocols);
        }
    } else {
        exists = True;
        mt     = proto;
    }
    if (exists) {
        ev.type                 = ClientMessage;
        ev.xclient.window       = w;
        ev.xclient.message_type = mt;
        ev.xclient.format       = 32;
        ev.xclient.data.l[0]    = d0;
        ev.xclient.data.l[1]    = d1;
        ev.xclient.data.l[2]    = d2;
        ev.xclient.data.l[3]    = d3;
        ev.xclient.data.l[4]    = d4;
        XSendEvent(dpy, w, False, mask, &ev);
    }
    return exists;
}

/**
 * 设置窗口间距
 */
void setgaps(const Arg *arg)
{
    if ((arg->i == 0) || (select_monitor->gappx + arg->i < 0))
        select_monitor->gappx = 0;
    else
        select_monitor->gappx += arg->i;
    arrange(select_monitor);
}


/**
 * 设置主窗口尺寸因子
 */
/* arg > 1.0 will set mfact absolutely */
void setmfact(const Arg *arg)
{
    float f;

    if (!arg || !select_monitor->lt[select_monitor->sellt]->arrange) {
        return;
    }
    f = arg->f < 1.0 ? arg->f + select_monitor->mfact : arg->f - 1.0;
    if (f < 0.05 || f > 0.95) {
        return;
    }
    select_monitor->mfact = select_monitor->pertag->layout[select_monitor->pertag->curtag].mfacts = f;
    arrange(select_monitor);
}

/**
 * 初始化
 */
void setup(void)
{
    int                  i;
    XSetWindowAttributes wa;
    Atom                 utf8string;

    /* 立即清理任何僵尸进程 */
    sigchld(0);

    /* init screen */
    screen        = DefaultScreen(dpy);
    screen_width  = DisplayWidth(dpy, screen);
    screen_height = DisplayHeight(dpy, screen);
    root          = RootWindow(dpy, screen);

    xinitvisual();
    drw = drw_create(dpy, screen, root, screen_width, screen_height, visual, depth, cmap);

    // 字符集载入
    if (!drw_fontset_create(drw, fonts, fonts_count())) {
        die("no fonts could be loaded.");
    }

    lrpad      = drw->fonts->h;
    bar_height = drw->fonts->h + userbarheight;
    updategeom();
    sp = sidepad;
    vp = (topbar == 1) ? vertpad : -vertpad;

    /* init atoms */
    utf8string          = XInternAtom(dpy, "UTF8_STRING", False);
    wmatom[WMProtocols] = XInternAtom(dpy, "WM_PROTOCOLS", False);
    wmatom[WMDelete]    = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    wmatom[WMState]     = XInternAtom(dpy, "WM_STATE", False);
    wmatom[WMTakeFocus] = XInternAtom(dpy, "WM_TAKE_FOCUS", False);

    netatom[NetActiveWindow]              = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
    netatom[NetSupported]                 = XInternAtom(dpy, "_NET_SUPPORTED", False);
    netatom[NetSystemTray]                = XInternAtom(dpy, "_NET_SYSTEM_TRAY_S0", False);
    netatom[NetSystemTrayOP]              = XInternAtom(dpy, "_NET_SYSTEM_TRAY_OPCODE", False);
    netatom[NetSystemTrayOrientation]     = XInternAtom(dpy, "_NET_SYSTEM_TRAY_ORIENTATION", False);
    netatom[NetSystemTrayOrientationHorz] = XInternAtom(dpy, "_NET_SYSTEM_TRAY_ORIENTATION_HORZ", False);
    netatom[NetSystemTrayVisual]          = XInternAtom(dpy, "_NET_SYSTEM_TRAY_VISUAL", False);
    netatom[NetWMName]                    = XInternAtom(dpy, "_NET_WM_NAME", False);
    netatom[NetWMIcon]                    = XInternAtom(dpy, "_NET_WM_ICON", False);
    netatom[NetWMState]                   = XInternAtom(dpy, "_NET_WM_STATE", False);
    netatom[NetWMCheck]                   = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
    netatom[NetWMFullscreen]              = XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
    netatom[NetWMWindowType]              = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
    netatom[NetWMWindowTypeDock]          = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DOCK", False);
    netatom[NetWMWindowTypeDialog]        = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
    netatom[NetClientList]                = XInternAtom(dpy, "_NET_CLIENT_LIST", False);

    xatom[Manager]    = XInternAtom(dpy, "MANAGER", False);
    xatom[Xembed]     = XInternAtom(dpy, "_XEMBED", False);
    xatom[XembedInfo] = XInternAtom(dpy, "_XEMBED_INFO", False);

    /* init 光标 */
    cursor[CurNormal] = drw_cur_create(drw, XC_left_ptr);
    cursor[CurResize] = drw_cur_create(drw, XC_sizing);
    cursor[CurMove]   = drw_cur_create(drw, XC_fleur);

    /* init appearance */
    scheme                 = ecalloc(colors_count() + 1, sizeof(Clr *));
    scheme[colors_count()] = drw_scm_create(drw, colors[0], alphas[0], 3);
    for (i = 0; i < colors_count(); i++) {
        scheme[i] = drw_scm_create(drw, colors[i], alphas[i], 3);
    }

    /* 初始化系统托盘 */
    if (show_systray) {
        update_systray(0);
    }

    /* init bars */
    updatebars();
    update_status();
    updatebarpos(select_monitor);

    /* supporting window for NetWMCheck */
    wmcheckwin = XCreateSimpleWindow(dpy, root, 0, 0, 1, 1, 0, 0, 0);
    XChangeProperty(dpy, wmcheckwin, netatom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&wmcheckwin,
                    1);
    XChangeProperty(dpy, wmcheckwin, netatom[NetWMName], utf8string, 8, PropModeReplace, (unsigned char *)"dwm", 3);
    XChangeProperty(dpy, root, netatom[NetWMCheck], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&wmcheckwin, 1);
    /* EWMH support per view */
    XChangeProperty(dpy, root, netatom[NetSupported], XA_ATOM, 32, PropModeReplace, (unsigned char *)netatom, NetLast);
    XDeleteProperty(dpy, root, netatom[NetClientList]);
    /* select events */
    wa.cursor     = cursor[CurNormal]->cursor;
    wa.event_mask = SubstructureRedirectMask | SubstructureNotifyMask | ButtonPressMask | PointerMotionMask
                  | EnterWindowMask | LeaveWindowMask | StructureNotifyMask | PropertyChangeMask;
    XChangeWindowAttributes(dpy, root, CWEventMask | CWCursor, &wa);
    XSelectInput(dpy, root, wa.event_mask);

    grabkeys();  // 注册组合键
    focus(NULL);
}

/**
 * 设置紧急性
 */
void seturgent(Client *c, int urg)
{
    XWMHints *wmh;

    c->isurgent = urg;
    if (!(wmh = XGetWMHints(dpy, c->win))) {
        return;
    }
    wmh->flags = urg ? (wmh->flags | XUrgencyHint) : (wmh->flags & ~XUrgencyHint);
    XSetWMHints(dpy, c->win, wmh);
    XFree(wmh);
}

/**
 * 立即清理僵尸进程
 */
void sigchld(int unused)
{
    if (signal(SIGCHLD, sigchld) == SIG_ERR)
        die("can't install SIGCHLD handler:");
    while (0 < waitpid(-1, NULL, WNOHANG))
        ;
}

/**
 * 执行命令
 */
void spawn(const Arg *arg)
{
    select_monitor->tagset[select_monitor->seltags] &= ~scratchtag;
    if (fork() == 0) {
        if (dpy) {
            close(ConnectionNumber(dpy));
        }
        setsid();
        execvp(((char **)arg->v)[0], (char **)arg->v);
        die("dwm: execvp '%s' failed:", ((char **)arg->v)[0]);
    }
}


/**
 * 切换全部浮动
 */
void toggleallfloating(const Arg *arg)
{
    Client *c            = NULL;
    int     somefloating = 0;

    if (!select_monitor->select || select_monitor->select->isfullscreen) {
        return;
    }

    for (c = select_monitor->clients; c; c = c->next) {
        if (ISVISIBLE(c) && !HIDDEN(c) && c->isfloating) {
            somefloating = 1;
            break;
        }
    }

    if (somefloating) {
        for (c = select_monitor->clients; c; c = c->next) {
            if (ISVISIBLE(c) && !HIDDEN(c)) {
                c->isfloating = 0;
            }
        }
        arrange(select_monitor);
    } else {
        for (c = select_monitor->clients; c; c = c->next) {
            if (ISVISIBLE(c) && !HIDDEN(c)) {
                c->isfloating = 1;
                resize(c, c->x + 2 * snap, c->y + 2 * snap, MAX(c->w - 4 * snap, snap), MAX(c->h - 4 * snap, snap), 0);
            }
        }
    }
    pointerfocuswin(select_monitor->select);
}



/**
 * 切换便签薄
 */
void togglescratch(const Arg *arg)
{
    Client      *c;
    unsigned int found = 0;

    for (c = select_monitor->clients; c && !(found = c->tags & scratchtag); c = c->next)
        ;
    if (found) {
        unsigned int newtagset = select_monitor->tagset[select_monitor->seltags] ^ scratchtag;
        if (newtagset) {
            select_monitor->tagset[select_monitor->seltags] = newtagset;
            focus(NULL);
            arrange(select_monitor);
        }
        if (ISVISIBLE(c)) {
            focus(c);
            restack(select_monitor);
        }
    } else {
        spawn(arg);
    }
}


/**
 * 取消焦点
 */
void unfocus(Client *c, int setfocus)
{
    if (!c) {
        return;
    }
    grabbuttons(c, 0);
    XSetWindowBorder(dpy, c->win, scheme[SchemeNorm][ColBorder].pixel);
    if (setfocus) {
        XSetInputFocus(dpy, root, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(dpy, root, netatom[NetActiveWindow]);
    }
}

/**
 * 不再管理窗口
 */
void unmanage(Client *c, int destroyed)
{
    Monitor       *m = c->mon;
    XWindowChanges wc;

    detach(c);
    detachstack(c);
    freeicon(c);
    if (!destroyed) {
        wc.border_width = c->oldbw;
        XGrabServer(dpy); /* avoid race conditions */
        XSetErrorHandler(xerrordummy);
        XSelectInput(dpy, c->win, NoEventMask);
        XConfigureWindow(dpy, c->win, CWBorderWidth, &wc); /* restore border */
        XUngrabButton(dpy, AnyButton, AnyModifier, c->win);
        setclientstate(c, WithdrawnState);
        XSync(dpy, False);
        XSetErrorHandler(xerror);
        XUngrabServer(dpy);
    }
    free(c);
    focus(NULL);
    updateclientlist();
    arrange(m);
}

/**
 * 取消映射通知
 */
void unmapnotify(XEvent *e)
{
    Client      *c;
    XUnmapEvent *ev = &e->xunmap;

    if ((c = window_to_client(ev->window))) {
        if (ev->send_event) {
            setclientstate(c, WithdrawnState);
        } else {
            unmanage(c, 0);
        }
    } else if (show_systray && (c = window_to_systray_icon(ev->window))) {
        /* KLUDGE! sometimes icons occasionally unmap their windows, but do
         * _not_ destroy them. We map those windows back */
        XMapRaised(dpy, c->win);
        update_systray(1);
    }
}


/**
 * 更新数字键盘锁按键掩码
 */
void updatenumlockmask(void)
{
    unsigned int     i, j;
    XModifierKeymap *modmap;

    numlockmask = 0;
    modmap      = XGetModifierMapping(dpy);  // 获取修饰符按键状态
    for (i = 0; i < 8; i++) {
        for (j = 0; j < modmap->max_keypermod; j++) {
            if (modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(dpy, XK_Num_Lock)) {
                numlockmask = (1 << i);
            }
        }
    }
    XFreeModifiermap(modmap);
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

/* 初始化视觉 */
void xinitvisual()
{
    XVisualInfo       *infos;
    XRenderPictFormat *fmt;
    int                nitems;
    int                i;

    XVisualInfo tpl   = {.screen = screen, .depth = 32, .class = TrueColor};
    long        masks = VisualScreenMask | VisualDepthMask | VisualClassMask;

    // 获取视觉信息
    infos  = XGetVisualInfo(dpy, masks, &tpl, &nitems);
    visual = NULL;
    for (i = 0; i < nitems; i++) {
        fmt = XRenderFindVisualFormat(dpy, infos[i].visual);
        if (fmt->type == PictTypeDirect && fmt->direct.alphaMask) {
            visual  = infos[i].visual;
            depth   = infos[i].depth;
            cmap    = XCreateColormap(dpy, root, visual, AllocNone);
            useargb = 1;
            break;
        }
    }

    XFree(infos);

    if (!visual) {
        visual = DefaultVisual(dpy, screen);
        depth  = DefaultDepth(dpy, screen);
        cmap   = DefaultColormap(dpy, screen);
    }
}

/**
 * 放大,提升到栈顶
 */
void zoom(const Arg *arg)
{
    Client *c = select_monitor->select;

    if (!select_monitor->lt[select_monitor->sellt]->arrange || !c || c->isfloating) {
        return;
    }
    if (c == nexttiled(select_monitor->clients) && !(c = nexttiled(c->next))) {
        return;
    }

    pop(c);
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
    if (!(dpy = XOpenDisplay(NULL))) {
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
    XCloseDisplay(dpy);
    return EXIT_SUCCESS;
}
