
#include "dwm.h"
#include "bar.h"
#include "client.h"
#include "monitor.h"
#include "window.h"
#include "config.h"

#define INTERSECT(x, y, w, h, m)                                   \
    (MAX(0, MIN((x) + (w), (m)->wx + (m)->ww) - MAX((x), (m)->wx)) \
     * MAX(0, MIN((y) + (h), (m)->wy + (m)->wh) - MAX((y), (m)->wy)))

#ifdef XINERAMA
static int is_unique_geometries(XineramaScreenInfo *unique, size_t n, XineramaScreenInfo *info)
{
    while (n--) {
        if (unique[n].x_org == info->x_org && unique[n].y_org == info->y_org && unique[n].width == info->width
            && unique[n].height == info->height)
        {
            return 0;
        }
    }
    return 1;
}
#endif /* XINERAMA */

/**
 * 创建一个监视器
 */
Monitor *monitor_create(void)
{
    Monitor     *m;
    unsigned int i;

    m            = ecalloc(1, sizeof(Monitor));
    m->tagset[0] = m->tagset[1] = 1;
    m->mfact                    = mfact;
    m->nmaster                  = nmaster;
    m->showbar                  = showbar;
    m->topbar                   = topbar;
    m->gappx                    = gappx;
    m->lt[0]                    = &layouts[0];
    m->lt[1]                    = &layouts[1 % layouts_count()];
    strncpy(m->ltsymbol, layouts[0].symbol, sizeof m->ltsymbol);
    m->is_overview                         = 0;
    m->pertag         = ecalloc(1, sizeof(Pertag));
    m->pertag->curtag = m->pertag->prevtag = 1;
    m->pertag->layout = ecalloc(TAGS_COUNT + 1, sizeof(struct LayoutConfig));

    for (i = 0; i <= TAGS_COUNT; i++) {
        m->pertag->layout[i].nmasters = m->nmaster;
        m->pertag->layout[i].mfacts   = m->mfact;

        m->pertag->layout[i].ltidxs[0] = m->lt[0];
        m->pertag->layout[i].ltidxs[1] = m->lt[1];
        m->pertag->layout[i].sellts    = m->sellt;

        m->pertag->layout[i].showbars = m->showbar;
    }

    return m;
}

/**
 * 更新Bar尺寸
 *
 * @return 是否需要重新绘制
 */
int monitor_update_geometries(void)
{
    int dirty = 0;

#ifdef XINERAMA
    if (XineramaIsActive(display)) {
        int                 i, j, n, nn;
        Client             *c;
        Monitor            *m;
        XineramaScreenInfo *info   = XineramaQueryScreens(display, &nn);
        XineramaScreenInfo *unique = NULL;

        for (n = 0, m = monitor_list; m; m = m->next, n++)
            ;
        /* only consider unique geometries as separate screens */
        unique = ecalloc(nn, sizeof(XineramaScreenInfo));
        for (i = 0, j = 0; i < nn; i++) {
            if (is_unique_geometries(unique, j, &info[i])) {
                memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
            }
        }
        XFree(info);
        nn = j;

        /* new monitors if nn > n */
        for (i = n; i < nn; i++) {
            for (m = monitor_list; m && m->next; m = m->next)
                ;
            if (m) {
                m->next = monitor_create();
            } else {
                monitor_list = monitor_create();
            }
        }
        for (i = 0, m = monitor_list; i < nn && m; m = m->next, i++) {
            if (i >= n || unique[i].x_org != m->mx || unique[i].y_org != m->my || unique[i].width != m->mw
                || unique[i].height != m->mh)
            {
                dirty  = 1;
                m->num = i;
                m->mx = m->wx = unique[i].x_org;
                m->my = m->wy = unique[i].y_org;
                m->mw = m->ww = unique[i].width;
                m->mh = m->wh = unique[i].height;
                bar_update_pos(m);
            }
        }
        /* removed monitors if n > nn */
        for (i = nn; i < n; i++) {
            for (m = monitor_list; m && m->next; m = m->next)
                ;
            while ((c = m->clients)) {
                dirty      = 1;
                m->clients = c->next;
                client_detach_stack(c);
                c->mon = monitor_list;
                client_attach(c);
                client_attach_stack(c);
            }
            if (m == select_monitor) {
                select_monitor = monitor_list;
            }
            monitor_cleanup(m);
        }
        free(unique);
    } else
#endif /* XINERAMA */
    {  /* default monitor setup */
        if (!monitor_list) {
            monitor_list = monitor_create();
        }
        if (monitor_list->mw != screen_width || monitor_list->mh != screen_height) {
            dirty    = 1;
            monitor_list->mw = monitor_list->ww = screen_width;
            monitor_list->mh = monitor_list->wh = screen_height;
            bar_update_pos(monitor_list);
        }
    }
    if (dirty) {
        select_monitor = monitor_list;
        select_monitor = window_to_monitor(root_window);
    }
    return dirty;
}

/**
 * 矩形所在的监视器
 */
Monitor *monitor_rect_to_monitor(int x, int y, int w, int h)
{
    Monitor *m, *r   = select_monitor;
    int      a, area = 0;

    for (m = monitor_list; m; m = m->next) {
        if ((a = INTERSECT(x, y, w, h, m)) > area) {
            area = a;
            r    = m;
        }
    }

    return r;
}

/**
 * 指定方向的监视器
 */
Monitor *monitor_dir_to_monitor(int dir)
{
    Monitor *m = NULL;

    if (dir > 0) {
        if (!(m = select_monitor->next)) {
            m = monitor_list;
        }
    } else if (select_monitor == monitor_list) {
        for (m = monitor_list; m->next; m = m->next)
            ;
    } else {
        for (m = monitor_list; m->next != select_monitor; m = m->next)
            ;
    }
    return m;
}

/**
 * 清除释放监视器
 */
void monitor_cleanup(Monitor *mon)
{
    Monitor *m;

    if (mon == monitor_list) {
        monitor_list = monitor_list->next;
    } else {
        for (m = monitor_list; m && m->next != mon; m = m->next)
            ;
        m->next = mon->next;
    }
    XUnmapWindow(display, mon->bar_window);
    XDestroyWindow(display, mon->bar_window);
    free(mon);
}

/**
 * 当前客户端切换监视器
 */
void move_to_monitor(const Arg *arg)
{
    if (!select_monitor->select || !monitor_list->next)
        return;

    client_send_to_monitor(select_monitor->select, monitor_dir_to_monitor(arg->i));
    focus_monitor(&(Arg){.i = +1});
    client_pointer_focus_win(select_monitor->select);
}

/**
 * 光标移动到上一个显示器
 */
void focus_monitor(const Arg *arg)
{
    Monitor *m;

    if (!monitor_list->next) {
        return;
    }
    if ((m = monitor_dir_to_monitor(arg->i)) == select_monitor) {
        return;
    }
    client_unfocus(select_monitor->select, 0);
    select_monitor = m;
    client_focus(NULL);
    client_pointer_focus_win(NULL);
}
