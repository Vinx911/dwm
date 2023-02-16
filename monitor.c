
#include "dwm.h"
#include "client.h"
#include "monitor.h"
#include "config.h"



/**
 * 当前客户端切换监视器
 */
void tagmon(const Arg *arg)
{
    if (!select_monitor->select || !mons->next)
        return;

    sendmon(select_monitor->select, dirtomon(arg->i));
    focusmon(&(Arg){.i = +1});
    pointerfocuswin(select_monitor->select);
}



/**
 * 矩形所在的监视器
 */
Monitor *recttomon(int x, int y, int w, int h)
{
    Monitor *m, *r   = select_monitor;
    int      a, area = 0;

    for (m = mons; m; m = m->next) {
        if ((a = INTERSECT(x, y, w, h, m)) > area) {
            area = a;
            r    = m;
        }
    }

    return r;
}



/**
 * 创建一个监视器
 */
Monitor *createmon(void)
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
    m->pertag->layout = ecalloc(TAGS_COUNT, sizeof(struct LayoutConfig));

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
 * 指定方向的监视器
 */
Monitor *dirtomon(int dir)
{
    Monitor *m = NULL;

    if (dir > 0) {
        if (!(m = select_monitor->next)) {
            m = mons;
        }
    } else if (select_monitor == mons) {
        for (m = mons; m->next; m = m->next)
            ;
    } else {
        for (m = mons; m->next != select_monitor; m = m->next)
            ;
    }
    return m;
}

/**
 * 清除释放监视器
 */
void cleanup_monitor(Monitor *mon)
{
    Monitor *m;

    if (mon == mons) {
        mons = mons->next;
    } else {
        for (m = mons; m && m->next != mon; m = m->next)
            ;
        m->next = mon->next;
    }
    XUnmapWindow(dpy, mon->bar_window);
    XDestroyWindow(dpy, mon->bar_window);
    free(mon);
}
