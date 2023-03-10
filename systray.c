
#include "dwm.h"
#include "bar.h"
#include "client.h"
#include "layout.h"
#include "status_bar.h"
#include "systray.h"
#include "window.h"
#include "config.h"

Systray      *systray            = NULL;
unsigned long systrayorientation = _NET_SYSTEM_TRAY_ORIENTATION_HORZ;

/**
 * 获取系统托盘宽度
 */
unsigned int systray_get_width()
{
    unsigned int w = 0;
    Client      *icon;
    if (show_systray) {
        for (icon = systray->icons; icon; w += icon->w + systray_spacing, icon = icon->next)
            ;
    }
    return w ? w + systray_spacing : 0;
}

/**
 * 更新系统托盘
 *
 * @param update_bar 是否更新bar
 */
void systray_update(int update_bar)
{
    XSetWindowAttributes wa;
    XWindowChanges       wc;
    Client              *icon;
    Monitor             *m = systray_to_monitor(NULL);
    unsigned int         x = m->mx + m->mw;
    unsigned int         w = 1, xpad = 0, ypad = 0;
    xpad = bar_side_padding;
    ypad = bar_ver_padding;

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
        wa.colormap          = color_map;
        systray->win = XCreateWindow(display, root_window, x - xpad, m->by + ypad, w, bar_height, 0, depth, InputOutput, visual,
                                     CWOverrideRedirect | CWBackPixel | CWBorderPixel | CWColormap | CWEventMask, &wa);
        XSelectInput(display, systray->win, SubstructureNotifyMask);
        XChangeProperty(display, systray->win, netatom[NetSystemTrayOrientation], XA_CARDINAL, 32, PropModeReplace,
                        (unsigned char *)&systrayorientation, 1);
        XChangeProperty(display, systray->win, netatom[NetSystemTrayVisual], XA_VISUALID, 32, PropModeReplace,
                        (unsigned char *)&visual->visualid, 1);
        XChangeProperty(display, systray->win, netatom[NetWMWindowType], XA_ATOM, 32, PropModeReplace,
                        (unsigned char *)&netatom[NetWMWindowTypeDock], 1);
        XMapRaised(display, systray->win);
        XSetSelectionOwner(display, netatom[NetSystemTray], systray->win, CurrentTime);
        if (XGetSelectionOwner(display, netatom[NetSystemTray]) == systray->win) {
            window_send_event(root_window, xatom[Manager], StructureNotifyMask, CurrentTime, netatom[NetSystemTray], systray->win, 0,
                      0);
            XSync(display, False);
        } else {
            fprintf(stderr, "dwm: unable to obtain system tray.\n");
            free(systray);
            systray = NULL;
            return;
        }
    }

    for (w = 0, icon = systray->icons; icon; icon = icon->next) {
        wa.background_pixel = 0;
        XChangeWindowAttributes(display, icon->win, CWBackPixel, &wa);
        XMapRaised(display, icon->win);
        w += systray_spacing;
        icon->x = w;
        XMoveResizeWindow(display, icon->win, icon->x, (bar_height - icon->h) / 2, icon->w, icon->h);
        w += icon->w;
        if (icon->mon != m)
            icon->mon = m;
    }
    w = w ? w + systray_spacing : 1;
    x -= w;
    XMoveResizeWindow(display, systray->win, x - xpad, m->by + ypad, w, bar_height);
    wc.x          = x - xpad;
    wc.y          = m->by + ypad;
    wc.width      = w;
    wc.height     = bar_height;
    wc.stack_mode = Above;
    wc.sibling    = m->bar_window;
    XConfigureWindow(display, systray->win, CWX | CWY | CWWidth | CWHeight | CWSibling | CWStackMode, &wc);
    XMapWindow(display, systray->win);
    XMapSubwindows(display, systray->win);
    XSync(display, False);

    if (update_bar) {
        bar_draw_bar(m);
    }
}

/**
 * 更新系统托盘图标尺寸
 *
 * @param icon 托盘图标
 * @param w 宽度
 * @param h 高度
 */
void systray_update_icon_geom(Client *icon, int w, int h)
{
    if (icon) {
        icon->h = systray_icon_size;
        if (w == h) {
            icon->w = systray_icon_size;
        } else if (h == systray_icon_size) {
            icon->w = w;
        } else {
            icon->w = (int)((float)systray_icon_size * ((float)w / (float)h));
        }

        client_apply_size_hints(icon, &(icon->x), &(icon->y), &(icon->w), &(icon->h), False);
        /* force icons into the systray dimensions if they don't want to */
        if (icon->h > systray_icon_size) {
            if (icon->w == icon->h) {
                icon->w = systray_icon_size;
            } else {
                icon->w = (int)((float)systray_icon_size * ((float)icon->w / (float)icon->h));
            }
            icon->h = systray_icon_size;
        }

        if (icon->w > 2 * systray_icon_size) {
            icon->w = systray_icon_size;
        }
    }
}

/**
 * 更新系统托盘图标状态
 *
 * @param icon 托盘图标
 * @param ev 事件
 */
void systray_update_icon_state(Client *icon, XPropertyEvent *ev)
{
    long flags;
    int  code = 0;

    if (!show_systray || !icon || ev->atom != xatom[XembedInfo] || !(flags = client_get_atom_prop(icon, xatom[XembedInfo])))
        return;

    if (flags & XEMBED_MAPPED && !icon->tags) {
        icon->tags = 1;
        code    = XEMBED_WINDOW_ACTIVATE;
        XMapRaised(display, icon->win);
        client_set_state(icon, NormalState);
    } else if (!(flags & XEMBED_MAPPED) && icon->tags) {
        icon->tags = 0;
        code    = XEMBED_WINDOW_DEACTIVATE;
        XUnmapWindow(display, icon->win);
        client_set_state(icon, WithdrawnState);
    } else {
        return;
    }
    window_send_event(icon->win, xatom[Xembed], StructureNotifyMask, CurrentTime, code, 0, systray->win, XEMBED_EMBEDDED_VERSION);
}

/**
 * 移除系统托盘图标
 */
void systray_remove_icon(Client *icon)
{
    Client **ii;

    if (!show_systray || !icon) {
        return;
    }

    for (ii = &systray->icons; *ii && *ii != icon; ii = &(*ii)->next)
        ;

    if (ii) {
        *ii = icon->next;
    }
    free(icon);
}

/**
 * 系统托盘所在监视器
 */
Monitor *systray_to_monitor(Monitor *m)
{
    Monitor *t;
    int      i, n;
    if (!systray_pinning) {
        if (!m) {
            return select_monitor;
        }
        return m == select_monitor ? m : NULL;
    }

    for (n = 1, t = monitor_list; t && t->next; n++, t = t->next)
        ;

    for (i = 1, t = monitor_list; t && t->next && i < systray_pinning; i++, t = t->next)
        ;

    if (systray_pinning_fail_first && n < systray_pinning) {
        return monitor_list;
    }
    return t;
}

/**
 * 清除系统托盘资源
 */
void systray_cleanup()
{
    if (show_systray) {
        while (systray->icons) {
            systray_remove_icon(systray->icons);
        }
        XUnmapWindow(display, systray->win);
        XDestroyWindow(display, systray->win);
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
        XGetWindowAttributes(display, c->win, &wa);
        c->x = c->oldx = c->y = c->oldy = 0;
        c->w = c->oldw = wa.width;
        c->h = c->oldh = wa.height;
        c->oldbw       = wa.border_width;
        c->bw          = 0;
        c->isfloating  = True;
        /* reuse tags field as mapped status */
        c->tags = 1;
        client_update_size_hints(c);
        systray_update_icon_geom(c, wa.width, wa.height);
        XAddToSaveSet(display, c->win);
        XSelectInput(display, c->win, StructureNotifyMask | PropertyChangeMask | ResizeRedirectMask);
        XReparentWindow(display, c->win, systray->win, 0, 0);

        XClassHint ch = {"dwmsystray", "dwmsystray"};
        XSetClassHint(display, c->win, &ch);

        /* use parents background color */
        swa.background_pixel = scheme[SchemeNorm][ColBg].pixel;
        XChangeWindowAttributes(display, c->win, CWBackPixel, &swa);
        window_send_event(c->win, netatom[Xembed], StructureNotifyMask, CurrentTime, XEMBED_EMBEDDED_NOTIFY, 0, systray->win,
                  XEMBED_EMBEDDED_VERSION);
        XSync(display, False);
        client_set_state(c, NormalState);
        systray_update(1);
    }
}

/**
 * 切换系统托盘显示
 */
void toggle_systray(const Arg *arg)
{
    if (show_systray) {
        show_systray = 0;
        XUnmapWindow(display, systray->win);
    } else {
        show_systray = 1;
    }

    systray_update(1);
    status_bar_update_status();
}
