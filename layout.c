
#include "dwm.h"

/**
 * 排列窗口
 */
void arrange(Monitor *m)
{
    if (m) {
        showhide(m->stack);
    } else {
        for (m = mons; m; m = m->next) {
            showhide(m->stack);
        }
    }

    if (m) {
        arrangemon(m);
        restack(m);
    } else {
        for (m = mons; m; m = m->next) {
            arrangemon(m);
        }
    }
}

/**
 * 排列尺寸计算
 */
void arrangemon(Monitor *m)
{
    if (m->is_overview) {
        strncpy(m->ltsymbol, overviewlayout.symbol, sizeof m->ltsymbol);
        overviewlayout.arrange(m);
    } else {
        strncpy(m->ltsymbol, m->lt[m->sellt]->symbol, sizeof m->ltsymbol);
        if (m->lt[m->sellt]->arrange) {
            m->lt[m->sellt]->arrange(m);
        }
    }
}


void magicgrid(Monitor *m)
{
    grid(m, gappx);
}

void overview(Monitor *m)
{
    grid(m, overviewgappx);
}

void grid(Monitor *m, uint gappx)
{
    unsigned int i, n;
    unsigned int cx, cy, cw, ch;
    unsigned int dx;
    unsigned int cols, rows, overcols;
    Client      *c;

    for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++)
        ;
    if (n == 0) {
        return;
    }
    if (n == 1) {
        c  = nexttiled(m->clients);
        cw = (m->ww - 2 * gappx) * 0.7;
        ch = (m->wh - 2 * gappx) * 0.65;
        resize(c, m->mx + (m->mw - cw) / 2 + gappx, m->my + (m->mh - ch) / 2 + gappx, cw - 2 * c->bw, ch - 2 * c->bw,
               0);
        return;
    }
    if (n == 2) {
        c  = nexttiled(m->clients);
        cw = (m->ww - 2 * gappx - gappx) / 2;
        ch = (m->wh - 2 * gappx) * 0.65;
        resize(c, m->mx + gappx, m->my + (m->mh - ch) / 2 + gappx, cw - 2 * c->bw, ch - 2 * c->bw, 0);
        resize(nexttiled(c->next), m->mx + cw + gappx + gappx, m->my + (m->mh - ch) / 2 + gappx, cw - 2 * c->bw,
               ch - 2 * c->bw, 0);
        return;
    }

    for (cols = 0; cols <= n / 2; cols++) {
        if (cols * cols >= n)
            break;
    }
    rows = (cols && (cols - 1) * cols >= n) ? cols - 1 : cols;
    ch   = (m->wh - 2 * gappx - (rows - 1) * gappx) / rows;
    cw   = (m->ww - 2 * gappx - (cols - 1) * gappx) / cols;

    overcols = n % cols;
    if (overcols) {
        dx = (m->ww - overcols * cw - (overcols - 1) * gappx) / 2 - gappx;
    }
    for (i = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
        cx = m->wx + (i % cols) * (cw + gappx);
        cy = m->wy + (i / cols) * (ch + gappx);
        if (overcols && i >= n - overcols) {
            cx += dx;
        }
        resize(c, cx + gappx, cy + gappx, cw - 2 * c->bw, ch - 2 * c->bw, 0);
    }
}


/**
 * 单窗口布局
 */
void monocle(Monitor *m)
{
    unsigned int n   = 0;
    unsigned int gap = m->gappx;

    Client *c;

    for (c = m->clients; c; c = c->next) {
        if (ISVISIBLE(c)) {
            n++;
        }
    }
    if (monocleshowcount && n > 0) { /* override layout symbol */
        snprintf(m->ltsymbol, sizeof m->ltsymbol, "[%d]", n);
    }
    for (c = nexttiled(m->clients); c; c = nexttiled(c->next)) {
        resize(c, m->wx + gap, m->wy + gap, m->ww - 2 * c->bw - 2 * gap, m->wh - 2 * c->bw - 2 * gap, 0);
    }
}
/**
 * 切换monocle显示窗口个数
 */
void togglemonocle(const Arg *arg)
{
    if (select_monitor->lt[select_monitor->sellt] == &layouts[2]) {
        monocleshowcount ^= 1;
        arrange(select_monitor);
        drawbar(select_monitor);
    }
}


/**
 * 平铺布局
 */
void tile(Monitor *m)
{
    int     i, n, h, mw, my, ty;
    Client *c;

    // 计算窗口个数
    for (n = 0, c = nexttiled(m->clients); c; c = nexttiled(c->next), n++)
        ;

    if (n == 0) {
        return;
    }

    if (n > m->nmaster) {
        mw = m->nmaster ? m->ww * m->mfact : 0;
    } else {
        mw = m->ww - m->gappx;
    }

    for (i = 0, my = ty = m->gappx, c = nexttiled(m->clients); c; c = nexttiled(c->next), i++) {
        if (i < m->nmaster) {
            h = (m->wh - my) / (MIN(n, m->nmaster) - i) - m->gappx;
            resize(c, m->wx + m->gappx, m->wy + my, mw - (2 * c->bw) - m->gappx, h - (2 * c->bw), 0);
            if (my + HEIGHT(c) < m->wh) {
                my += HEIGHT(c) + m->gappx;
            }
        } else {
            int rx = m->wx + mw + m->gappx;
            int ry = m->wy + ty;
            int rw = m->ww - mw - (2 * c->bw) - 2 * m->gappx;
            int rh = (m->wh - ty) / (n - i) - m->gappx - (2 * c->bw);
            if (rh < minclientheight) {
                rh = minclientheight;
            }

            resize(c, rx, ry, rw, rh, 0);
            if (ty + HEIGHT(c) + m->gappx + minclientheight < m->wh) {
                ty += HEIGHT(c) + m->gappx;
            }
        }
    }
}




/**
 * 设置布局
 */
void setlayout(const Arg *arg)
{
    if (!arg || !arg->v || arg->v != select_monitor->lt[select_monitor->sellt]) {
        select_monitor->sellt = select_monitor->pertag->layout[select_monitor->pertag->curtag].sellts ^= 1;
    }
    if (arg && arg->v) {
        select_monitor->lt[select_monitor->sellt] =
            select_monitor->pertag->layout[select_monitor->pertag->curtag].ltidxs[select_monitor->sellt] = (Layout *)arg->v;
    }
    strncpy(select_monitor->ltsymbol, select_monitor->lt[select_monitor->sellt]->symbol,
            sizeof select_monitor->ltsymbol);
    if (select_monitor->select) {
        arrange(select_monitor);
    } else {
        drawbar(select_monitor);
    }
}

/**
 * 重新堆叠窗口
 */
void restack(Monitor *m)
{
    Client        *c;
    XEvent         ev;
    XWindowChanges wc;

    drawbar(m);
    if (!m->select) {
        return;
    }
    if (m->select->isfloating || !m->lt[m->sellt]->arrange) {
        XRaiseWindow(dpy, m->select->win);
    }
    if (m->lt[m->sellt]->arrange) {
        wc.stack_mode = Below;
        wc.sibling    = m->bar_window;
        for (c = m->stack; c; c = c->snext) {
            if (!c->isfloating && ISVISIBLE(c)) {
                XConfigureWindow(dpy, c->win, CWSibling | CWStackMode, &wc);
                wc.sibling = c->win;
            }
        }
    }
    XSync(dpy, False);
    while (XCheckMaskEvent(dpy, EnterWindowMask, &ev))
        ;
}
