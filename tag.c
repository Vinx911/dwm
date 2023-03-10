
#include "dwm.h"
#include "bar.h"
#include "tag.h"
#include "client.h"
#include "layout.h"
#include "window.h"
#include "config.h"

/**
 * 显示tag
 */
void view_tag(const Arg *arg)
{
    int          i;
    unsigned int tmptag;
    Client      *c;
    int          n = 0;

    select_monitor->seltags ^= 1; /* toggle select tagset */
    if (arg->ui & TAGMASK) {
        select_monitor->tagset[select_monitor->seltags] = arg->ui & TAGMASK;
        select_monitor->pertag->prevtag                 = select_monitor->pertag->curtag;

        if (arg->ui == ~0) {
            select_monitor->pertag->curtag = 0;
        } else {
            for (i = 0; !(arg->ui & 1 << i); i++)
                ;
            select_monitor->pertag->curtag = i + 1;
        }
    } else {  // 显示上一个tag
        tmptag                          = select_monitor->pertag->prevtag;
        select_monitor->pertag->prevtag = select_monitor->pertag->curtag;
        select_monitor->pertag->curtag  = tmptag;
    }

    select_monitor->nmaster = select_monitor->pertag->layout[select_monitor->pertag->curtag].nmasters;
    select_monitor->mfact   = select_monitor->pertag->layout[select_monitor->pertag->curtag].mfacts;
    select_monitor->sellt   = select_monitor->pertag->layout[select_monitor->pertag->curtag].sellts;
    select_monitor->lt[select_monitor->sellt] =
        select_monitor->pertag->layout[select_monitor->pertag->curtag].ltidxs[select_monitor->sellt];
    select_monitor->lt[select_monitor->sellt ^ 1] =
        select_monitor->pertag->layout[select_monitor->pertag->curtag].ltidxs[select_monitor->sellt ^ 1];

    if (select_monitor->showbar != select_monitor->pertag->layout[select_monitor->pertag->curtag].showbars) {
        toggle_bar(NULL);
    }

    client_focus(NULL);
    layout_arrange(select_monitor);

    // 若当前tag无窗口 且附加了v参数 则执行
    if (arg->v) {
        for (c = select_monitor->clients; c; c = c->next)
            if (c->tags & arg->ui && !HIDDEN(c))
                n++;
        if (n == 0) {
            app_starter(arg);
        }
    }
}

/**
 * 切换当前客户端tag
 */
void move_to_tag(const Arg *arg)
{
    if (select_monitor->select && arg->ui & TAGMASK) {
        select_monitor->select->tags = arg->ui & TAGMASK;
        client_focus(NULL);
        layout_arrange(select_monitor);
        view_tag(&(Arg){.ui = arg->ui});
    } else {
        view_tag(arg);
    }
}

/**
 * 切换当前客户端所属tag, 可以添加一个新的tag
 */
void append_to_tag(const Arg *arg)
{
    unsigned int newtags;

    if (!select_monitor->select) {
        return;
    }
    newtags = select_monitor->select->tags ^ (arg->ui & TAGMASK);
    if (newtags) {
        select_monitor->select->tags = newtags;
        client_focus(NULL);
        layout_arrange(select_monitor);
    }
}

/**
 * 切换tag显示状态
 */
void toggle_tag_view(const Arg *arg)
{
    unsigned int newtagset = select_monitor->tagset[select_monitor->seltags] ^ (arg->ui & TAGMASK);
    int          i;

    if (newtagset) {
        select_monitor->tagset[select_monitor->seltags] = newtagset;

        if (newtagset == ~0) {
            select_monitor->pertag->prevtag = select_monitor->pertag->curtag;
            select_monitor->pertag->curtag  = 0;
        }

        /* test if the user did not select the same tag */
        if (!(newtagset & 1 << (select_monitor->pertag->curtag - 1))) {
            select_monitor->pertag->prevtag = select_monitor->pertag->curtag;
            for (i = 0; !(newtagset & 1 << i); i++)
                ;
            select_monitor->pertag->curtag = i + 1;
        }

        /* apply settings for this view */
        select_monitor->nmaster = select_monitor->pertag->layout[select_monitor->pertag->curtag].nmasters;
        select_monitor->mfact   = select_monitor->pertag->layout[select_monitor->pertag->curtag].mfacts;
        select_monitor->sellt   = select_monitor->pertag->layout[select_monitor->pertag->curtag].sellts;
        select_monitor->lt[select_monitor->sellt] =
            select_monitor->pertag->layout[select_monitor->pertag->curtag].ltidxs[select_monitor->sellt];
        select_monitor->lt[select_monitor->sellt ^ 1] =
            select_monitor->pertag->layout[select_monitor->pertag->curtag].ltidxs[select_monitor->sellt ^ 1];

        if (select_monitor->showbar != select_monitor->pertag->layout[select_monitor->pertag->curtag].showbars)
            toggle_bar(NULL);

        client_focus(NULL);
        layout_arrange(select_monitor);
    }
}

/**
 * 显示所有tag 或 跳转到聚焦窗口的tag
 */
void toggle_overview(const Arg *arg)
{
    uint target = select_monitor->select ? select_monitor->select->tags : select_monitor->seltags;
    select_monitor->is_overview ^= 1;
    view_tag(&(Arg){.ui = target});
    client_pointer_focus_win(select_monitor->select);
}
