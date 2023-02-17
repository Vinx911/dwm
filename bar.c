
#include "dwm.h"
#include "client.h"
#include "monitor.h"
#include "window.h"
#include "layout.h"
#include "status_bar.h"
#include "systray.h"
#include "config.h"

/**
 * 更新Bar位置
 */
void bar_update_pos(Monitor *m)
{
    m->wy = m->my;
    m->wh = m->mh;
    if (m->showbar) {
        m->wh = m->wh - vertpad - bar_height;
        m->by = m->topbar ? m->wy : m->wy + m->wh + vertpad;  // 这里实际上是屏幕尺寸的y坐标
        m->wy = m->topbar ? m->wy + bar_height + bar_ver_padding : m->wy;  // 窗口区域y坐标是屏幕尺寸的y坐标加上Bar的高度
    } else {
        m->by = -bar_height - bar_ver_padding;  // 不显示, 负值在屏幕外
    }
}

/**
 * 更新Bar
 */
void bar_update_bars(void)
{
    Monitor             *m;
    XSetWindowAttributes wa = {.override_redirect = True,
                               .background_pixel  = 0,
                               .border_pixel      = 0,
                               .colormap          = color_map,
                               .event_mask        = ButtonPressMask | ExposureMask};
    XClassHint           ch = {"dwm", "dwm"};
    for (m = monitor_list; m; m = m->next) {
        if (m->bar_window) {
            continue;
        }
        m->bar_window = XCreateWindow(display, root_window, m->wx + bar_side_padding, m->by + bar_ver_padding, m->ww - 2 * bar_side_padding, bar_height, 0, depth,
                                      InputOutput, visual,
                                      CWOverrideRedirect | CWBackPixel | CWBorderPixel | CWColormap | CWEventMask, &wa);
        XDefineCursor(display, m->bar_window, cursor[CurNormal]->cursor);
        if (show_systray && m == systray_to_monitor(m)) {
            XMapRaised(display, systray->win);
        }
        XMapRaised(display, m->bar_window);
        XSetClassHint(display, m->bar_window, &ch);
    }
}

/**
 * 绘制Bar
 */
void bar_draw_bar(Monitor *m)
{
    int          x, w, system_tray_width = 0, n = 0, scm;
    unsigned int i, occ = 0, urg = 0;
    Client      *c;
    int          boxw = 2;

    if (!m->showbar) {
        return;
    }

    drw_rect(drw, 0, 0, m->ww, bar_height, 1, 1);

    if (show_systray && m == systray_to_monitor(m)) {
        system_tray_width = systray_get_width();
        drw_setscheme(drw, scheme[SchemeNorm]);
        drw_rect(drw, m->ww - system_tray_width - 2 * bar_side_padding, 0, system_tray_width, bar_height, 1, 1);
    }

    statsu_bar_width = status_bar_draw(m, status_text);

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
        drw_text(drw, x, 0, w, bar_height, text_lr_pad / 2, overviewtag, 0);
        drw_setscheme(drw, scheme[SchemeUnderline]);
        drw_rect(drw, x, bar_height - boxw, w + text_lr_pad, boxw, 1, 0);
        x += w;
    } else {
        for (i = 0; i < TAGS_COUNT; i++) {
            /* do not draw vacant tags */
            if (!(occ & 1 << i || m->tagset[m->seltags] & 1 << i)) {
                continue;
            }

            w = TEXTW(tags[i]);
            drw_setscheme(drw, scheme[m->tagset[m->seltags] & 1 << i ? SchemeSel : SchemeNorm]);
            drw_text(drw, x, 0, w, bar_height, text_lr_pad / 2, tags[i], urg & 1 << i);
            if (m->tagset[m->seltags] & 1 << i) {
                drw_setscheme(drw, scheme[SchemeUnderline]);
                drw_rect(drw, x, bar_height - boxw, w + text_lr_pad, boxw, 1, 0);
            }
            x += w;
        }
    }

    w               = TEXTW(m->ltsymbol);
    tag_bar_width   = x;
    lt_symbol_width = w;
    drw_setscheme(drw, scheme[SchemeNorm]);
    x = drw_text(drw, x, 0, w, bar_height, text_lr_pad / 2, m->ltsymbol, 0);

    if ((w = m->ww - statsu_bar_width - system_tray_width - x - 2 * bar_side_padding) > bar_height) {
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

                drw_text(drw, x, 0, tabw, bar_height, text_lr_pad / 2 + (c->icon ? c->icw + winiconspacing : 0),
                         c->name, 0);
                if (c->icon) {
                    drw_pic(drw, x + text_lr_pad / 2, (bar_height - c->ich) / 2, c->icw, c->ich, c->icon);
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
void bar_draw_bars(void)
{
    Monitor *m;

    for (m = monitor_list; m; m = m->next) {
        bar_draw_bar(m);
    }

    if (show_systray && !systraypinning) {
        systray_update(0);
    }
}

/**
 * 切换bar显示状态
 */
void toggle_bar(const Arg *arg)
{
    select_monitor->showbar = select_monitor->pertag->layout[select_monitor->pertag->curtag].showbars =
        !select_monitor->showbar;
    bar_update_pos(select_monitor);
    XMoveResizeWindow(display, select_monitor->bar_window, select_monitor->wx + bar_side_padding, select_monitor->by + bar_ver_padding,
                      select_monitor->ww - 2 * bar_side_padding, bar_height);
    if (show_systray) {
        XWindowChanges wc;
        if (!select_monitor->showbar) {
            wc.y = -bar_height;
        } else if (select_monitor->showbar) {
            wc.y = bar_ver_padding;
            if (!select_monitor->topbar)
                wc.y = select_monitor->mh - bar_height + bar_ver_padding;
        }
        XConfigureWindow(display, systray->win, CWY, &wc);
    }
    layout_arrange(select_monitor);
}
