

#include "dwm.h"

/**
 * 更新Bar
 */
void updatebars(void)
{
    Monitor             *m;
    XSetWindowAttributes wa = {.override_redirect = True,
                               .background_pixel  = 0,
                               .border_pixel      = 0,
                               .colormap          = cmap,
                               .event_mask        = ButtonPressMask | ExposureMask};
    XClassHint           ch = {"dwm", "dwm"};
    for (m = mons; m; m = m->next) {
        if (m->bar_window) {
            continue;
        }
        m->bar_window = XCreateWindow(dpy, root, m->wx + sp, m->by + vp, m->ww - 2 * sp, bar_height, 0, depth,
                                      InputOutput, visual,
                                      CWOverrideRedirect | CWBackPixel | CWBorderPixel | CWColormap | CWEventMask, &wa);
        XDefineCursor(dpy, m->bar_window, cursor[CurNormal]->cursor);
        if (show_systray && m == systray_to_monitor(m)) {
            XMapRaised(dpy, systray->win);
        }
        XMapRaised(dpy, m->bar_window);
        XSetClassHint(dpy, m->bar_window, &ch);
    }
}

/**
 * 更新Bar位置
 */
void updatebarpos(Monitor *m)
{
    m->wy = m->my;
    m->wh = m->mh;
    if (m->showbar) {
        m->wh = m->wh - vertpad - bar_height;
        m->by = m->topbar ? m->wy : m->wy + m->wh + vertpad;  // 这里实际上是屏幕尺寸的y坐标
        m->wy = m->topbar ? m->wy + bar_height + vp : m->wy;  // 窗口区域y坐标是屏幕尺寸的y坐标加上Bar的高度
    } else
        m->by = -bar_height - vp;  // 不显示, 负值在屏幕外
}

#ifdef XINERAMA
static int isuniquegeom(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info)
{
    while (n--)
        if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org && unique[n].width == info->width
            && unique[n].height == info->height)
            return 0;
    return 1;
}
#endif /* XINERAMA */

/**
 * 更新Bar尺寸
 *
 * @return 是否需要重新绘制
 */
int updategeom(void)
{
    int dirty = 0;

#ifdef XINERAMA
    if (XineramaIsActive(dpy)) {
        int                 i, j, n, nn;
        Client             *c;
        Monitor            *m;
        XineramaScreenInfo *info   = XineramaQueryScreens(dpy, &nn);
        XineramaScreenInfo *unique = NULL;

        for (n = 0, m = mons; m; m = m->next, n++)
            ;
        /* only consider unique geometries as separate screens */
        unique = ecalloc(nn, sizeof(XineramaScreenInfo));
        for (i = 0, j = 0; i < nn; i++)
            if (isuniquegeom(unique, j, &info[i]))
                memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
        XFree(info);
        nn = j;

        /* new monitors if nn > n */
        for (i = n; i < nn; i++) {
            for (m = mons; m && m->next; m = m->next)
                ;
            if (m)
                m->next = createmon();
            else
                mons = createmon();
        }
        for (i = 0, m = mons; i < nn && m; m = m->next, i++)
            if (i >= n || unique[i].x_org != m->mx || unique[i].y_org != m->my || unique[i].width != m->mw
                || unique[i].height != m->mh)
            {
                dirty  = 1;
                m->num = i;
                m->mx = m->wx = unique[i].x_org;
                m->my = m->wy = unique[i].y_org;
                m->mw = m->ww = unique[i].width;
                m->mh = m->wh = unique[i].height;
                updatebarpos(m);
            }
        /* removed monitors if n > nn */
        for (i = nn; i < n; i++) {
            for (m = mons; m && m->next; m = m->next)
                ;
            while ((c = m->clients)) {
                dirty      = 1;
                m->clients = c->next;
                detachstack(c);
                c->mon = mons;
                attach(c);
                attachstack(c);
            }
            if (m == select_monitor)
                select_monitor = mons;
            cleanup_monitor(m);
        }
        free(unique);
    } else
#endif /* XINERAMA */
    {  /* default monitor setup */
        if (!mons) {
            mons = createmon();
        }
        if (mons->mw != screen_width || mons->mh != screen_height) {
            dirty    = 1;
            mons->mw = mons->ww = screen_width;
            mons->mh = mons->wh = screen_height;
            updatebarpos(mons);
        }
    }
    if (dirty) {
        select_monitor = mons;
        select_monitor = window_to_monitor(root);
    }
    return dirty;
}


/**
 * 切换bar显示状态
 */
void togglebar(const Arg *arg)
{
    select_monitor->showbar = select_monitor->pertag->layout[select_monitor->pertag->curtag].showbars =
        !select_monitor->showbar;
    updatebarpos(select_monitor);
    XMoveResizeWindow(dpy, select_monitor->bar_window, select_monitor->wx + sp, select_monitor->by + vp,
                      select_monitor->ww - 2 * sp, bar_height);
    if (show_systray) {
        XWindowChanges wc;
        if (!select_monitor->showbar) {
            wc.y = -bar_height;
        } else if (select_monitor->showbar) {
            wc.y = vp;
            if (!select_monitor->topbar)
                wc.y = select_monitor->mh - bar_height + vp;
        }
        XConfigureWindow(dpy, systray->win, CWY, &wc);
    }
    arrange(select_monitor);
}



/**
 * 绘制Bar
 */
void drawbar(Monitor *m)
{
    int          x, w, system_tray_width = 0, n = 0, scm;
    unsigned int i, occ = 0, urg = 0;
    Client      *c;
    int          boxw = 2;

    if (!m->showbar) {
        return;
    }

    if (show_systray && m == systray_to_monitor(m)) {
        system_tray_width = get_systray_width();
        drw_setscheme(drw, scheme[SchemeNorm]);
        drw_rect(drw, m->ww - system_tray_width, 0, system_tray_width, bar_height, 1, 1);
    }

    statsu_bar_width = draw_status_bar(m, status_text);

    for (c = m->clients; c; c = c->next) {
        if (ISVISIBLE(c)) {
            n++;
        }
        occ |= c->tags == 255 ? 0 : c->tags;
        if (c->isurgent) {
            urg |= c->tags;
        }
    }
    x = 0;

    // 代表为overview tag状态
    if (m->is_overview) {
        w = TEXTW(overviewtag);
        drw_setscheme(drw, scheme[SchemeSel]);
        drw_text(drw, x, 0, w, bar_height, lrpad / 2, overviewtag, 0);
        drw_setscheme(drw, scheme[SchemeUnderline]);
        drw_rect(drw, x, bar_height - boxw, w + lrpad, boxw, 1, 0);
        x += w;
    } else {
        for (i = 0; i < TAGS_COUNT; i++) {
            /* do not draw vacant tags */
            if (!(occ & 1 << i || m->tagset[m->seltags] & 1 << i)) {
                continue;
            }

            w = TEXTW(tags[i]);
            drw_setscheme(drw, scheme[m->tagset[m->seltags] & 1 << i ? SchemeSel : SchemeNorm]);
            drw_text(drw, x, 0, w, bar_height, lrpad / 2, tags[i], urg & 1 << i);
            if (m->tagset[m->seltags] & 1 << i) {
                drw_setscheme(drw, scheme[SchemeUnderline]);
                drw_rect(drw, x, bar_height - boxw, w + lrpad, boxw, 1, 0);
            }
            x += w;
        }
    }

    w               = TEXTW(m->ltsymbol);
    tag_bar_width   = x;
    lt_symbol_width = w;
    drw_setscheme(drw, scheme[SchemeNorm]);
    x = drw_text(drw, x, 0, w, bar_height, lrpad / 2, m->ltsymbol, 0);

    if ((w = m->ww - statsu_bar_width - system_tray_width - x - 2 * sp) > bar_height) {
        if (n > 0) {
            int remainder = w % n;
            int tabw      = (1.0 / (double)n) * w + 1;
            for (c = m->clients; c; c = c->next) {
                if (!ISVISIBLE(c)) {
                    continue;
                }
                if (m->select == c) {
                    scm = SchemeHid;
                } else if (HIDDEN(c)) {
                    scm = SchemeHid;
                } else {
                    scm = SchemeNorm;
                }
                drw_setscheme(drw, scheme[scm]);

                if (remainder >= 0) {
                    if (remainder == 0) {
                        tabw--;
                    }
                    remainder--;
                }

                drw_text(drw, x, 0, tabw, bar_height, lrpad / 2 + (c->icon ? c->icw + winiconspacing : 0), c->name, 0);
                if (c->icon) {
                    drw_pic(drw, x + lrpad / 2, (bar_height - c->ich) / 2, c->icw, c->ich, c->icon);
                }

                x += tabw;
            }
        } else {
            drw_setscheme(drw, scheme[SchemeNorm]);
            drw_rect(drw, x, 0, w, bar_height, 1, 1);
        }
    }
    m->task_count     = n;
    m->task_bar_width = w;
    drw_map(drw, m->bar_window, 0, 0, m->ww, bar_height);
}

/**
 * 绘制Bar
 */
void drawbars(void)
{
    Monitor *m;

    for (m = mons; m; m = m->next) {
        drawbar(m);
    }

    if (show_systray && !systraypinning) {
        update_systray(0);
    }
}
