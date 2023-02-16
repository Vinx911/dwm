
#include "dwm.h"

/**
 * 客户端附加到列表中
 */
void attach(Client *c)
{
    if (!newclientathead) {
        Client **tc;
        for (tc = &c->mon->clients; *tc; tc = &(*tc)->next)
            ;
        *tc     = c;
        c->next = NULL;
    } else {
        c->next         = c->mon->clients;
        c->mon->clients = c;
    }
}

/**
 * 客户端附加到栈中
 */
void attachstack(Client *c)
{
    c->snext      = c->mon->stack;
    c->mon->stack = c;
}

/**
 * 客户端从列表中分离
 */
void detach(Client *c)
{
    Client **tc;

    for (tc = &c->mon->clients; *tc && *tc != c; tc = &(*tc)->next)
        ;
    *tc = c->next;
}

/**
 * 客户端从栈中分离
 */
void detachstack(Client *c)
{
    Client **tc, *t;

    for (tc = &c->mon->stack; *tc && *tc != c; tc = &(*tc)->snext)
        ;
    *tc = c->snext;

    if (c == c->mon->select) {
        for (t = c->mon->stack; t && !ISVISIBLE(t); t = t->snext)
            ;
        c->mon->select = t;
    }
}

/**
 * 客户端入队
 */
void enqueue(Client *c)
{
    Client *l;
    for (l = c->mon->clients; l && l->next; l = l->next)
        ;
    if (l) {
        l->next = c;
        c->next = NULL;
    }
}

/**
 * 客户端栈中入队
 */
void enqueuestack(Client *c)
{
    Client *l;
    for (l = c->mon->stack; l && l->snext; l = l->snext)
        ;
    if (l) {
        l->snext = c;
        c->snext = NULL;
    }
}


/**
 * 客户端提升到顶部
 */
void pop(Client *c)
{
    detach(c);
    attach(c);
    focus(c);
    arrange(c->mon);
}

/**
 * 杀死客户端
 */
void killclient(const Arg *arg)
{
    if (!select_monitor->select) {
        return;
    }
    if (!sendevent(select_monitor->select->win, wmatom[WMDelete], NoEventMask, wmatom[WMDelete], CurrentTime, 0, 0, 0))
    {
        XGrabServer(dpy);
        XSetErrorHandler(xerrordummy);
        XSetCloseDownMode(dpy, DestroyAll);
        XKillClient(dpy, select_monitor->select->win);
        XSync(dpy, False);
        XSetErrorHandler(xerror);
        XUngrabServer(dpy);
    }
}

/**
 * 重设客户端尺寸
 */
void resize(Client *c, int x, int y, int w, int h, int interact)
{
    if (applysizehints(c, &x, &y, &w, &h, interact)) {
        resizeclient(c, x, y, w, h);
    }
}

/**
 * 重设客户端尺寸
 */
void resizeclient(Client *c, int x, int y, int w, int h)
{
    XWindowChanges wc;

    c->oldx = c->x;
    c->x = wc.x = x;
    c->oldy     = c->y;
    c->y = wc.y = y;
    c->oldw     = c->w;
    c->w = wc.width = w;
    c->oldh         = c->h;
    c->h = wc.height = h;
    wc.border_width  = c->bw;
    if (((nexttiled(c->mon->clients) == c && !nexttiled(c->next)) || &monocle == c->mon->lt[c->mon->sellt]->arrange)
        && !c->isfullscreen && !c->isfloating)
    {
        c->w            = wc.width += c->bw * 2;
        c->h            = wc.height += c->bw * 2;
        wc.border_width = 0;
    }
    XConfigureWindow(dpy, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
    configure(c);
    XSync(dpy, False);
}

/**
 * 设置客户端全屏
 */
void setfullscreen(Client *c, int fullscreen)
{
    if (fullscreen && !c->isfullscreen) {
        XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32, PropModeReplace,
                        (unsigned char *)&netatom[NetWMFullscreen], 1);
        c->isfullscreen = 1;
        if (c->isfakefullscreen) {
            resizeclient(c, c->x, c->y, c->w, c->h);
            return;
        }
        c->oldstate   = c->isfloating;
        c->oldbw      = c->bw;
        c->bw         = 0;
        c->isfloating = 1;
        resizeclient(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
        XRaiseWindow(dpy, c->win);
    } else if (!fullscreen && c->isfullscreen) {
        XChangeProperty(dpy, c->win, netatom[NetWMState], XA_ATOM, 32, PropModeReplace, (unsigned char *)0, 0);
        c->isfullscreen = 0;
        if (c->isfakefullscreen) {
            resizeclient(c, c->x, c->y, c->w, c->h);
            return;
        }
        c->isfloating = c->oldstate;
        c->bw         = c->oldbw;
        c->x          = c->oldx;
        c->y          = c->oldy;
        c->w          = c->oldw;
        c->h          = c->oldh;
        resizeclient(c, c->x, c->y, c->w, c->h);
        arrange(c->mon);
    }
}

/**
 * 显示客户端
 */
void show(const Arg *arg)
{
    if (select_monitor->hidsel) {
        select_monitor->hidsel = 0;
    }
    showwin(select_monitor->select);
}

/**
 * 显示全部客户端
 */
void showall(const Arg *arg)
{
    Client *c              = NULL;
    select_monitor->hidsel = 0;
    for (c = select_monitor->clients; c; c = c->next) {
        if (ISVISIBLE(c)) {
            showwin(c);
        }
    }
    if (!select_monitor->select) {
        for (c = select_monitor->clients; c && !ISVISIBLE(c); c = c->next)
            ;
        if (c) {
            focus(c);
        }
    }
    restack(select_monitor);
}

/**
 * 显示窗口
 */
void showwin(Client *c)
{
    if (!c || !HIDDEN(c))
        return;

    XMapWindow(dpy, c->win);
    setclientstate(c, NormalState);
    arrange(c->mon);
}

/**
 * 显示和隐藏窗口列表
 */
void showhide(Client *c)
{
    if (!c) {
        return;
    }

    if (ISVISIBLE(c)) {
        /* show clients top down */
        XMoveWindow(dpy, c->win, c->x, c->y);
        if ((!c->mon->lt[c->mon->sellt]->arrange || c->isfloating) && (!c->isfullscreen || c->isfakefullscreen)) {
            resize(c, c->x, c->y, c->w, c->h, 0);
        }
        showhide(c->snext);
    } else {
        /* hide clients bottom up */
        showhide(c->snext);
        XMoveWindow(dpy, c->win, WIDTH(c) * -2, c->y);  // 隐藏窗口移动到屏幕外
    }
}



/**
 * 更新客户端列表
 */
void updateclientlist()
{
    Client  *c;
    Monitor *m;

    XDeleteProperty(dpy, root, netatom[NetClientList]);
    for (m = mons; m; m = m->next) {
        for (c = m->clients; c; c = c->next) {
            XChangeProperty(dpy, root, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
                            (unsigned char *)&(c->win), 1);
        }
    }
}


/**
 * 更新客户端尺寸提示
 */
void updatesizehints(Client *c)
{
    long       msize;
    XSizeHints size;

    if (!XGetWMNormalHints(dpy, c->win, &size, &msize)) {
        /* size is uninitialized, ensure that size.flags aren't used */
        size.flags = PSize;
    }

    if (size.flags & PBaseSize) {
        c->basew = size.base_width;
        c->baseh = size.base_height;
    } else if (size.flags & PMinSize) {
        c->basew = size.min_width;
        c->baseh = size.min_height;
    } else {
        c->basew = c->baseh = 0;
    }

    if (size.flags & PResizeInc) {
        c->incw = size.width_inc;
        c->inch = size.height_inc;
    } else {
        c->incw = c->inch = 0;
    }

    if (size.flags & PMaxSize) {
        c->maxw = size.max_width;
        c->maxh = size.max_height;
    } else {
        c->maxw = c->maxh = 0;
    }

    if (size.flags & PMinSize) {
        c->minw = size.min_width;
        c->minh = size.min_height;
    } else if (size.flags & PBaseSize) {
        c->minw = size.base_width;
        c->minh = size.base_height;
    } else {
        c->minw = c->minh = 0;
    }

    if (size.flags & PAspect) {
        c->mina = (float)size.min_aspect.y / size.min_aspect.x;
        c->maxa = (float)size.max_aspect.x / size.max_aspect.y;
    } else {
        c->maxa = c->mina = 0.0;
    }

    c->isfixed    = (c->maxw && c->maxh && c->maxw == c->minw && c->maxh == c->minh);
    c->hintsvalid = 1;
}

/**
 * 更新窗口标题
 */
void updatetitle(Client *c)
{
    if (!gettextprop(c->win, netatom[NetWMName], c->name, sizeof c->name)) {
        gettextprop(c->win, XA_WM_NAME, c->name, sizeof c->name);
    }
    if (c->name[0] == '\0') { /* hack to mark broken clients */
        strcpy(c->name, broken);
    }
}

/**
 * 更新窗口图标
 */
void updateicon(Client *c)
{
    freeicon(c);
    c->icon = geticonprop(c->win, &c->icw, &c->ich);
}



/**
 * 释放窗口图标
 */
void freeicon(Client *c)
{
    if (c->icon) {
        XRenderFreePicture(dpy, c->icon);
        c->icon = None;
    }
}

/**
 * 更新窗口类型
 */
void updatewindowtype(Client *c)
{
    Atom state = getatomprop(c, netatom[NetWMState]);
    Atom wtype = getatomprop(c, netatom[NetWMWindowType]);

    if (state == netatom[NetWMFullscreen]) {
        setfullscreen(c, 1);
    }
    if (wtype == netatom[NetWMWindowTypeDialog]) {
        c->isfloating = 1;
    }
}

/**
 * 更新wm hint
 */
void updatewmhints(Client *c)
{
    XWMHints *wmh;

    if ((wmh = XGetWMHints(dpy, c->win))) {
        if (c == select_monitor->select && wmh->flags & XUrgencyHint) {
            wmh->flags &= ~XUrgencyHint;
            XSetWMHints(dpy, c->win, wmh);
        } else {
            c->isurgent = (wmh->flags & XUrgencyHint) ? 1 : 0;
        }

        if (wmh->flags & InputHint) {
            c->neverfocus = !wmh->input;
        } else {
            c->neverfocus = 0;
        }
        XFree(wmh);
    }
}


/**
 * 切换窗口显示状态
 */
void togglewin(const Arg *arg)
{
    Client *c = (Client *)arg->v;

    if (c == NULL) {
        return;
    }

    if (c == select_monitor->select) {
        hidewin(c);
        focus(NULL);
        arrange(c->mon);
    } else {
        if (HIDDEN(c)) {
            showwin(c);
        }
        focus(c);
        restack(select_monitor);
    }
}


/**
 * 切换当前客户端浮动
 */
void togglefloating(const Arg *arg)
{
    if (!select_monitor->select)
        return;

    if (select_monitor->select->isfullscreen
        && !select_monitor->select->isfakefullscreen) /* no support for fullscreen windows */
        return;

    select_monitor->select->isfloating = !select_monitor->select->isfloating || select_monitor->select->isfixed;
    if (select_monitor->select->isfloating) {
        // resize(select_monitor->select, select_monitor->select->x, select_monitor->select->y,
        // select_monitor->select->w, select_monitor->select->h, 0);
        resize(select_monitor->select, select_monitor->wx + select_monitor->ww / 6,
               select_monitor->wy + select_monitor->wh / 6, select_monitor->ww / 3 * 2, select_monitor->wh / 3 * 2, 0);
    }
    arrange(select_monitor);
    pointerfocuswin(select_monitor->select);
}


/**
 * 全屏
 */
Layout *last_layout;
void    fullscreen(const Arg *arg)
{
    if (select_monitor->showbar) {
        for (last_layout = (Layout *)layouts; last_layout != select_monitor->lt[select_monitor->sellt]; last_layout++)
            ;
        setlayout(&((Arg){.v = &layouts[2]}));
    } else {
        setlayout(&((Arg){.v = last_layout}));
    }
    togglebar(arg);
}

void togglefakefullscreen(const Arg *arg)
{
    Client *c = select_monitor->select;
    if (!c)
        return;

    c->isfakefullscreen = !c->isfakefullscreen;

    setfullscreen(c, (!c->isfullscreen || c->isfakefullscreen));
}



/**
 * 设置客户端焦点
 */
void setfocus(Client *c)
{
    if (!c->neverfocus) {
        XSetInputFocus(dpy, c->win, RevertToPointerRoot, CurrentTime);
        XChangeProperty(dpy, root, netatom[NetActiveWindow], XA_WINDOW, 32, PropModeReplace, (unsigned char *)&(c->win),
                        1);
    }
    sendevent(c->win, wmatom[WMTakeFocus], NoEventMask, wmatom[WMTakeFocus], CurrentTime, 0, 0, 0);
}


/**
 * 将客户端发送到监视器
 */
void sendmon(Client *c, Monitor *m)
{
    if (c->mon == m)
        return;
    unfocus(c, 1);
    detach(c);
    detachstack(c);
    c->mon  = m;
    c->tags = m->tagset[m->seltags]; /* assign tags of target monitor */
    attach(c);
    attachstack(c);
    focus(NULL);
    arrange(NULL);
}

/**
 * 设置客户端状态
 */
void setclientstate(Client *c, long state)
{
    long data[] = {state, None};

    XChangeProperty(dpy, c->win, wmatom[WMState], wmatom[WMState], 32, PropModeReplace, (unsigned char *)data, 2);
}


/**
 * 旋转窗口栈
 */
void rotatestack(const Arg *arg)
{
    Client *c = NULL, *f;

    if (!select_monitor->select) {
        return;
    }
    f = select_monitor->select;
    if (arg->i > 0) {
        for (c = nexttiled(select_monitor->clients); c && nexttiled(c->next); c = nexttiled(c->next))
            ;
        if (c) {
            detach(c);
            attach(c);
            detachstack(c);
            attachstack(c);
        }
    } else {
        if ((c = nexttiled(select_monitor->clients))) {
            detach(c);
            enqueue(c);
            detachstack(c);
            enqueuestack(c);
        }
    }
    if (c) {
        arrange(select_monitor);
        // unfocus(f, 1);
        focus(f);
        restack(select_monitor);
    }
}


/**
 * 鼠标调整窗口大小
 */
void resizemouse(const Arg *arg)
{
    int      ocx, ocy, nw, nh;
    Client  *c;
    Monitor *m;
    XEvent   ev;
    Time     lasttime = 0;

    if (!(c = select_monitor->select)) {
        return;
    }
    if (c->isfullscreen && !c->isfakefullscreen) { /* no support resizing fullscreen windows by mouse */
        return;
    }
    restack(select_monitor);
    ocx = c->x;
    ocy = c->y;
    if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, cursor[CurResize]->cursor,
                     CurrentTime)
        != GrabSuccess)
    {
        return;
    }
    XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
    do {
        XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
        switch (ev.type) {
        case ConfigureRequest:
        case Expose:
        case MapRequest:
            handler[ev.type](&ev);
            break;
        case MotionNotify:
            if ((ev.xmotion.time - lasttime) <= (1000 / 60))
                continue;
            lasttime = ev.xmotion.time;

            nw = MAX(ev.xmotion.x - ocx - 2 * c->bw + 1, 1);
            nh = MAX(ev.xmotion.y - ocy - 2 * c->bw + 1, 1);
            if (c->mon->wx + nw >= select_monitor->wx && c->mon->wx + nw <= select_monitor->wx + select_monitor->ww
                && c->mon->wy + nh >= select_monitor->wy && c->mon->wy + nh <= select_monitor->wy + select_monitor->wh)
            {
                if (!c->isfloating && select_monitor->lt[select_monitor->sellt]->arrange
                    && (abs(nw - c->w) > snap || abs(nh - c->h) > snap))
                {
                    togglefloating(NULL);
                }
            }
            if (!select_monitor->lt[select_monitor->sellt]->arrange || c->isfloating) {
                resize(c, c->x, c->y, nw, nh, 1);
            }
            break;
        }
    } while (ev.type != ButtonRelease);
    XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
    XUngrabPointer(dpy, CurrentTime);
    while (XCheckMaskEvent(dpy, EnterWindowMask, &ev))
        ;
    if ((m = recttomon(c->x, c->y, c->w, c->h)) != select_monitor) {
        sendmon(c, m);
        select_monitor = m;
        focus(NULL);
    }
}


/**
 * 下一个平铺的客户端
 */
Client *nexttiled(Client *c)
{
    for (; c && (c->isfloating || !ISVISIBLE(c) || HIDDEN(c)); c = c->next)
        ;
    return c;
}


/**
 * 鼠标移动窗口
 */
void movemouse(const Arg *arg)
{
    int      x, y, ocx, ocy, nx, ny;
    Client  *c;
    Monitor *m;
    XEvent   ev;
    Time     lasttime = 0;

    if (!(c = select_monitor->select)) {
        return;
    }
    if (c->isfullscreen && !c->isfakefullscreen) { /* no support moving fullscreen windows by mouse */
        return;
    }
    restack(select_monitor);
    ocx = c->x;
    ocy = c->y;
    if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync, None, cursor[CurMove]->cursor,
                     CurrentTime)
        != GrabSuccess)
    {
        return;
    }
    if (!getrootptr(&x, &y)) {
        return;
    }
    do {
        XMaskEvent(dpy, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
        switch (ev.type) {
        case ConfigureRequest:
        case Expose:
        case MapRequest:
            handler[ev.type](&ev);
            break;
        case MotionNotify:
            if ((ev.xmotion.time - lasttime) <= (1000 / 60)) {
                continue;
            }
            lasttime = ev.xmotion.time;

            nx = ocx + (ev.xmotion.x - x);
            ny = ocy + (ev.xmotion.y - y);
            if (abs(select_monitor->wx - nx) < snap) {
                nx = select_monitor->wx;
            } else if (abs((select_monitor->wx + select_monitor->ww) - (nx + WIDTH(c))) < snap) {
                nx = select_monitor->wx + select_monitor->ww - WIDTH(c);
            }
            if (abs(select_monitor->wy - ny) < snap) {
                ny = select_monitor->wy;
            } else if (abs((select_monitor->wy + select_monitor->wh) - (ny + HEIGHT(c))) < snap) {
                ny = select_monitor->wy + select_monitor->wh - HEIGHT(c);
            }
            if (!c->isfloating && select_monitor->lt[select_monitor->sellt]->arrange
                && (abs(nx - c->x) > snap || abs(ny - c->y) > snap))
            {
                togglefloating(NULL);
            }
            if (!select_monitor->lt[select_monitor->sellt]->arrange || c->isfloating) {
                resize(c, nx, ny, c->w, c->h, 1);
            }
            break;
        }
    } while (ev.type != ButtonRelease);
    XUngrabPointer(dpy, CurrentTime);
    if ((m = recttomon(c->x, c->y, c->w, c->h)) != select_monitor) {
        sendmon(c, m);
        select_monitor = m;
        focus(NULL);
    }
}




Atom getatomprop(Client *c, Atom prop)
{
    int            di;
    unsigned long  dl;
    unsigned char *p = NULL;
    Atom           da, atom = None;

    /* FIXME getatomprop should return the number of items and a pointer to
     * the stored data instead of this workaround */
    Atom req = XA_ATOM;
    if (prop == xatom[XembedInfo])
        req = xatom[XembedInfo];

    if (XGetWindowProperty(dpy, c->win, prop, 0L, sizeof atom, False, req, &da, &di, &dl, &dl, &p) == Success && p) {
        atom = *(Atom *)p;
        if (da == xatom[XembedInfo] && dl == 2)
            atom = ((Atom *)p)[1];
        XFree(p);
    }
    return atom;
}


/**
 * 配置客户端
 */
void configure(Client *c)
{
    XConfigureEvent ce;

    ce.type              = ConfigureNotify;
    ce.display           = dpy;
    ce.event             = c->win;
    ce.window            = c->win;
    ce.x                 = c->x;
    ce.y                 = c->y;
    ce.width             = c->w;
    ce.height            = c->h;
    ce.border_width      = c->bw;
    ce.above             = None;
    ce.override_redirect = False;
    XSendEvent(dpy, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}


/**
 * 应用客户端尺寸hints
 */
int applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact)
{
    int      baseismin;
    Monitor *m = c->mon;

    /* set minimum possible */
    *w = MAX(1, *w);
    *h = MAX(1, *h);
    if (interact) {
        if (*x > screen_width)
            *x = screen_width - WIDTH(c);
        if (*y > screen_height)
            *y = screen_height - HEIGHT(c);
        if (*x + *w + 2 * c->bw < 0)
            *x = 0;
        if (*y + *h + 2 * c->bw < 0)
            *y = 0;
    } else {
        if (*x >= m->wx + m->ww)
            *x = m->wx + m->ww - WIDTH(c);
        if (*y >= m->wy + m->wh)
            *y = m->wy + m->wh - HEIGHT(c);
        if (*x + *w + 2 * c->bw <= m->wx)
            *x = m->wx;
        if (*y + *h + 2 * c->bw <= m->wy)
            *y = m->wy;
    }
    if (*h < bar_height)
        *h = bar_height;
    if (*w < bar_height)
        *w = bar_height;
    if (resizehints || c->isfloating || !c->mon->lt[c->mon->sellt]->arrange) {
        if (!c->hintsvalid)
            updatesizehints(c);
        /* see last two sentences in ICCCM 4.1.2.3 */
        baseismin = c->basew == c->minw && c->baseh == c->minh;
        if (!baseismin) { /* temporarily remove base dimensions */
            *w -= c->basew;
            *h -= c->baseh;
        }
        /* adjust for aspect limits */
        if (c->mina > 0 && c->maxa > 0) {
            if (c->maxa < (float)*w / *h)
                *w = *h * c->maxa + 0.5;
            else if (c->mina < (float)*h / *w)
                *h = *w * c->mina + 0.5;
        }
        if (baseismin) { /* increment calculation requires this */
            *w -= c->basew;
            *h -= c->baseh;
        }
        /* adjust for increment value */
        if (c->incw)
            *w -= *w % c->incw;
        if (c->inch)
            *h -= *h % c->inch;
        /* restore base dimensions */
        *w = MAX(*w + c->basew, c->minw);
        *h = MAX(*h + c->baseh, c->minh);
        if (c->maxw)
            *w = MIN(*w, c->maxw);
        if (c->maxh)
            *h = MIN(*h, c->maxh);
    }
    return *x != c->x || *y != c->y || *w != c->w || *h != c->h;
}
