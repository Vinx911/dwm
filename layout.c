
#include "dwm.h"
#include "bar.h"
#include "client.h"
#include "monitor.h"
#include "layout.h"
#include "config.h"

static unsigned int monocleshowcount = 0; /* monocle显示窗口个数 */

/**
 * 排列窗口
 */
void layout_arrange(Monitor *m)
{
    if (m) {
        client_show_hide(m->stack);
    } else {
        for (m = monitor_list; m; m = m->next) {
            client_show_hide(m->stack);
        }
    }

    if (m) {
        layout_arrange_monitor(m);
        client_restack(m);
    } else {
        for (m = monitor_list; m; m = m->next) {
            layout_arrange_monitor(m);
        }
    }
}

/**
 * 排列监视器
 */
void layout_arrange_monitor(Monitor *m)
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

/**
 * 平铺布局
 */
void tile(Monitor *m)
{
    int     i, n, h, mw, my, ty;
    Client *c;

    // 计算窗口个数
    for (n = 0, c = client_next_tiled(m->clients); c; c = client_next_tiled(c->next), n++)
        ;

    if (n == 0) {
        return;
    }

    if (n > m->nmaster) {
        mw = m->nmaster ? m->ww * m->mfact : 0;
    } else {
        mw = m->ww - m->gappx;
    }

    for (i = 0, my = ty = m->gappx, c = client_next_tiled(m->clients); c; c = client_next_tiled(c->next), i++) {
        if (i < m->nmaster) {
            h = (m->wh - my) / (MIN(n, m->nmaster) - i) - m->gappx;
            client_resize(c, m->wx + m->gappx, m->wy + my, mw - (2 * c->bw) - m->gappx, h - (2 * c->bw), 0);
            if (my + HEIGHT(c) < m->wh) {
                my += HEIGHT(c) + m->gappx;
            }
        } else {
            int rx = m->wx + mw + m->gappx;
            int ry = m->wy + ty;
            int rw = m->ww - mw - (2 * c->bw) - 2 * m->gappx;
            int rh = (m->wh - ty) / (n - i) - m->gappx - (2 * c->bw);
            if (rh < min_client_height) {
                rh = min_client_height;
            }

            client_resize(c, rx, ry, rw, rh, 0);
            if (ty + HEIGHT(c) + m->gappx + min_client_height < m->wh) {
                ty += HEIGHT(c) + m->gappx;
            }
        }
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
    for (c = client_next_tiled(m->clients); c; c = client_next_tiled(c->next)) {
        client_resize(c, m->wx + gap, m->wy + gap, m->ww - 2 * c->bw - 2 * gap, m->wh - 2 * c->bw - 2 * gap, 0);
    }
}

void grid(Monitor *m, uint gappx)
{
    unsigned int i, n;
    unsigned int cx, cy, cw, ch;
    unsigned int dx;
    unsigned int cols, rows, overcols;
    Client      *c;

    for (n = 0, c = client_next_tiled(m->clients); c; c = client_next_tiled(c->next), n++)
        ;
    if (n == 0) {
        return;
    }
    if (n == 1) {
        c  = client_next_tiled(m->clients);
        cw = (m->ww - 2 * gappx) * 0.7;
        ch = (m->wh - 2 * gappx) * 0.65;
        client_resize(c, m->mx + (m->mw - cw) / 2 + gappx, m->my + (m->mh - ch) / 2 + gappx, cw - 2 * c->bw, ch - 2 * c->bw,
               0);
        return;
    }
    if (n == 2) {
        c  = client_next_tiled(m->clients);
        cw = (m->ww - 2 * gappx - gappx) / 2;
        ch = (m->wh - 2 * gappx) * 0.65;
        client_resize(c, m->mx + gappx, m->my + (m->mh - ch) / 2 + gappx, cw - 2 * c->bw, ch - 2 * c->bw, 0);
        client_resize(client_next_tiled(c->next), m->mx + cw + gappx + gappx, m->my + (m->mh - ch) / 2 + gappx, cw - 2 * c->bw,
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
    for (i = 0, c = client_next_tiled(m->clients); c; c = client_next_tiled(c->next), i++) {
        cx = m->wx + (i % cols) * (cw + gappx);
        cy = m->wy + (i / cols) * (ch + gappx);
        if (overcols && i >= n - overcols) {
            cx += dx;
        }
        client_resize(c, cx + gappx, cy + gappx, cw - 2 * c->bw, ch - 2 * c->bw, 0);
    }
}

void magic_grid(Monitor *m)
{
    grid(m, gappx);
}

void overview(Monitor *m)
{
    grid(m, overview_gappx);
}

/**
 * 设置布局
 */
void set_layout(const Arg *arg)
{
    if (!arg || !arg->v || arg->v != select_monitor->lt[select_monitor->sellt]) {
        select_monitor->sellt = select_monitor->pertag->layout[select_monitor->pertag->curtag].sellts ^= 1;
    }
    if (arg && arg->v) {
        select_monitor->lt[select_monitor->sellt] = select_monitor->pertag->layout[select_monitor->pertag->curtag]
                                                        .ltidxs[select_monitor->sellt] = (Layout *)arg->v;
    }
    strncpy(select_monitor->ltsymbol, select_monitor->lt[select_monitor->sellt]->symbol,
            sizeof select_monitor->ltsymbol);
    if (select_monitor->select) {
        layout_arrange(select_monitor);
    } else {
        bar_draw_bar(select_monitor);
    }
}

/**
 * 切换monocle显示窗口个数
 */
void toggle_monocle(const Arg *arg)
{
    if (select_monitor->lt[select_monitor->sellt] == &layouts[2]) {
        monocleshowcount ^= 1;
        layout_arrange(select_monitor);
        bar_draw_bar(select_monitor);
    }
}

/**
 * 增加/减少主窗口个数
 */
void inc_nmaster(const Arg *arg)
{
    int nmaster = select_monitor->nmaster + arg->i;
    if (select_monitor->task_count <= 1) {
        nmaster = 1;
    } else if (nmaster >= 3) {
        nmaster = 1;
    }
    select_monitor->nmaster = select_monitor->pertag->layout[select_monitor->pertag->curtag].nmasters = MAX(nmaster, 1);
    layout_arrange(select_monitor);
}

/**
 * 设置主窗口尺寸因子
 */
void set_mfact(const Arg *arg)
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
    layout_arrange(select_monitor);
}

/**
 * 设置窗口间距
 */
void set_gaps(const Arg *arg)
{
    if ((arg->i == 0) || (select_monitor->gappx + arg->i < 0))
        select_monitor->gappx = 0;
    else
        select_monitor->gappx += arg->i;
    layout_arrange(select_monitor);
}
