
#include "dwm.h"

Systray      *systray            = NULL;
unsigned long systrayorientation = _NET_SYSTEM_TRAY_ORIENTATION_HORZ;

/**
 * 获取系统托盘宽度
 */
unsigned int get_systray_width()
{
    unsigned int w = 0;
    Client      *i;
    if (show_systray) {
        for (i = systray->icons; i; w += i->w + systrayspacing, i = i->next)
            ;
    }
    return w ? w + systrayspacing : 0;
}

/**
 * 更新系统托盘
 *
 * @param update_bar 是否更新bar
 */
void update_systray(int update_bar)
{
    XSetWindowAttributes wa;
    XWindowChanges       wc;
    Client              *i;
    Monitor             *m = systray_to_monitor(NULL);
    unsigned int         x = m->mx + m->mw;
    unsigned int         w = 1, xpad = 0, ypad = 0;
    xpad = sp;
    ypad = vp;

    if (!show_systray) {
        return;
    }

    // 系统托盘未初始化
    if (!systray) {
        /* init systray */
        if (!(systray = (Systray *)calloc(1, sizeof(Systray)))) {
            die("fatal: could not malloc() %u bytes\n", sizeof(Systray));
        }

        wa.override_redirect = True;
        wa.event_mask        = ButtonPressMask | ExposureMask;
        wa.background_pixel  = 0;
        wa.border_pixel      = 0;
        wa.colormap          = cmap;
        systray->win = XCreateWindow(dpy, root, x - xpad, m->by + ypad, w, bar_height, 0, depth, InputOutput, visual,
                                     CWOverrideRedirect | CWBackPixel | CWBorderPixel | CWColormap | CWEventMask, &wa);
        XSelectInput(dpy, systray->win, SubstructureNotifyMask);
        XChangeProperty(dpy, systray->win, netatom[NetSystemTrayOrientation], XA_CARDINAL, 32, PropModeReplace,
                        (unsigned char *)&systrayorientation, 1);
        XChangeProperty(dpy, systray->win, netatom[NetSystemTrayVisual], XA_VISUALID, 32, PropModeReplace,
                        (unsigned char *)&visual->visualid, 1);
        XChangeProperty(dpy, systray->win, netatom[NetWMWindowType], XA_ATOM, 32, PropModeReplace,
                        (unsigned char *)&netatom[NetWMWindowTypeDock], 1);
        XMapRaised(dpy, systray->win);
        XSetSelectionOwner(dpy, netatom[NetSystemTray], systray->win, CurrentTime);
        if (XGetSelectionOwner(dpy, netatom[NetSystemTray]) == systray->win) {
            sendevent(root, xatom[Manager], StructureNotifyMask, CurrentTime, netatom[NetSystemTray], systray->win, 0,
                      0);
            XSync(dpy, False);
        } else {
            fprintf(stderr, "dwm: unable to obtain system tray.\n");
            free(systray);
            systray = NULL;
            return;
        }
    }

    for (w = 0, i = systray->icons; i; i = i->next) {
        wa.background_pixel = 0;
        ;
        XChangeWindowAttributes(dpy, i->win, CWBackPixel, &wa);
        XMapRaised(dpy, i->win);
        w += systrayspacing;
        i->x = w;
        XMoveResizeWindow(dpy, i->win, i->x, (bar_height - i->h) / 2, i->w, i->h);
        w += i->w;
        if (i->mon != m)
            i->mon = m;
    }
    w = w ? w + systrayspacing : 1;
    x -= w;
    XMoveResizeWindow(dpy, systray->win, x - xpad, m->by + ypad, w, bar_height);
    wc.x          = x - xpad;
    wc.y          = m->by + ypad;
    wc.width      = w;
    wc.height     = bar_height;
    wc.stack_mode = Above;
    wc.sibling    = m->bar_window;
    XConfigureWindow(dpy, systray->win, CWX | CWY | CWWidth | CWHeight | CWSibling | CWStackMode, &wc);
    XMapWindow(dpy, systray->win);
    XMapSubwindows(dpy, systray->win);
    XSync(dpy, False);

    if (update_bar) {
        drawbar(m);
    }
}

/**
 * 更新系统托盘图标尺寸
 *
 * @param i 托盘图标
 * @param w 宽度
 * @param h 高度
 */
void update_systray_icon_geom(Client *i, int w, int h)
{
    if (i) {
        i->h = systrayiconsize;
        if (w == h) {
            i->w = systrayiconsize;
        } else if (h == systrayiconsize) {
            i->w = w;
        } else {
            i->w = (int)((float)systrayiconsize * ((float)w / (float)h));
        }

        applysizehints(i, &(i->x), &(i->y), &(i->w), &(i->h), False);
        /* force icons into the systray dimensions if they don't want to */
        if (i->h > systrayiconsize) {
            if (i->w == i->h) {
                i->w = systrayiconsize;
            } else {
                i->w = (int)((float)systrayiconsize * ((float)i->w / (float)i->h));
            }
            i->h = systrayiconsize;
        }

        if (i->w > 2 * systrayiconsize) {
            i->w = systrayiconsize;
        }
    }
}

/**
 * 更新系统托盘图标状态
 *
 * @param i 托盘图标
 * @param ev 事件
 */
void update_systray_icon_state(Client *i, XPropertyEvent *ev)
{
    long flags;
    int  code = 0;

    if (!show_systray || !i || ev->atom != xatom[XembedInfo] || !(flags = getatomprop(i, xatom[XembedInfo])))
        return;

    if (flags & XEMBED_MAPPED && !i->tags) {
        i->tags = 1;
        code    = XEMBED_WINDOW_ACTIVATE;
        XMapRaised(dpy, i->win);
        setclientstate(i, NormalState);
    } else if (!(flags & XEMBED_MAPPED) && i->tags) {
        i->tags = 0;
        code    = XEMBED_WINDOW_DEACTIVATE;
        XUnmapWindow(dpy, i->win);
        setclientstate(i, WithdrawnState);
    } else {
        return;
    }
    sendevent(i->win, xatom[Xembed], StructureNotifyMask, CurrentTime, code, 0, systray->win, XEMBED_EMBEDDED_VERSION);
}

/**
 * 移除系统托盘图标
 */
void remove_systray_icon(Client *i)
{
    Client **ii;

    if (!show_systray || !i) {
        return;
    }

    for (ii = &systray->icons; *ii && *ii != i; ii = &(*ii)->next)
        ;

    if (ii) {
        *ii = i->next;
    }
    free(i);
}

/**
 * 系统托盘所在监视器
 */
Monitor *systray_to_monitor(Monitor *m)
{
    Monitor *t;
    int      i, n;
    if (!systraypinning) {
        if (!m) {
            return select_monitor;
        }
        return m == select_monitor ? m : NULL;
    }

    for (n = 1, t = mons; t && t->next; n++, t = t->next)
        ;

    for (i = 1, t = mons; t && t->next && i < systraypinning; i++, t = t->next)
        ;

    if (systraypinningfailfirst && n < systraypinning) {
        return mons;
    }
    return t;
}

/**
 * 窗口的系统托盘图标
 */
Client *window_to_systray_icon(Window w)
{
    Client *i = NULL;

    if (!show_systray || !w) {
        return i;
    }
    for (i = systray->icons; i && i->win != w; i = i->next)
        ;
    return i;
}

/**
 * 切换系统托盘显示
 */
void toggle_systray(const Arg *arg)
{
    if (show_systray) {
        show_systray = 0;
        XUnmapWindow(dpy, systray->win);
    } else {
        show_systray = 1;
    }

    update_systray(1);
    update_status();
}

/**
 * 清除系统托盘资源
 */
void cleanup_systray()
{
    if (show_systray) {
        while (systray->icons) {
            remove_systray_icon(systray->icons);
        }
        XUnmapWindow(dpy, systray->win);
        XDestroyWindow(dpy, systray->win);
        free(systray);
    }
}

/**
 * 系统托盘客户端消息
 */
void systray_client_message(XEvent *e)
{
    XWindowAttributes    wa;
    XSetWindowAttributes swa;
    XClientMessageEvent *cme = &e->xclient;
    Client              *c   = window_to_client(cme->window);

    /* add systray icons */
    if (cme->data.l[1] == SYSTEM_TRAY_REQUEST_DOCK) {
        if (!(c = (Client *)calloc(1, sizeof(Client)))) {
            die("fatal: could not malloc() %u bytes\n", sizeof(Client));
        }
        if (!(c->win = cme->data.l[2])) {
            free(c);
            return;
        }

        c->mon         = select_monitor;
        c->next        = systray->icons;
        systray->icons = c;
        XGetWindowAttributes(dpy, c->win, &wa);
        c->x = c->oldx = c->y = c->oldy = 0;
        c->w = c->oldw = wa.width;
        c->h = c->oldh = wa.height;
        c->oldbw       = wa.border_width;
        c->bw          = 0;
        c->isfloating  = True;
        /* reuse tags field as mapped status */
        c->tags = 1;
        updatesizehints(c);
        update_systray_icon_geom(c, wa.width, wa.height);
        XAddToSaveSet(dpy, c->win);
        XSelectInput(dpy, c->win, StructureNotifyMask | PropertyChangeMask | ResizeRedirectMask);
        XReparentWindow(dpy, c->win, systray->win, 0, 0);

        XClassHint ch = {"dwmsystray", "dwmsystray"};
        XSetClassHint(dpy, c->win, &ch);

        /* use parents background color */
        swa.background_pixel = scheme[SchemeNorm][ColBg].pixel;
        XChangeWindowAttributes(dpy, c->win, CWBackPixel, &swa);
        sendevent(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_EMBEDDED_NOTIFY, 0, systray->win,
                  XEMBED_EMBEDDED_VERSION);
        XSync(dpy, False);
        setclientstate(c, NormalState);
        update_systray(1);
    }
}