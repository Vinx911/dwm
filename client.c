
#include "dwm.h"
#include "bar.h"
#include "layout.h"
#include "client.h"
#include "layout.h"
#include "monitor.h"
#include "window.h"
#include "config.h"

#define BUTTONMASK (ButtonPressMask | ButtonReleaseMask)
#define MOUSEMASK (BUTTONMASK | PointerMotionMask)

// 隐藏窗口栈
static int hidden_win_stack_top = -1;
static Client *hidden_win_stack[100];

/**
 * 客户端附加到列表中
 */
void client_attach(Client *c)
{
    if (!new_client_ahead) {
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
 * 客户端从列表中分离
 */
void client_detach(Client *c)
{
    Client **tc;

    for (tc = &c->mon->clients; *tc && *tc != c; tc = &(*tc)->next)
        ;
    *tc = c->next;
}

/**
 * 客户端附加到栈中
 */
void client_attach_stack(Client *c)
{
    c->snext      = c->mon->stack;
    c->mon->stack = c;
}

/**
 * 客户端从栈中分离
 */
void client_detach_stack(Client *c)
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
void client_enqueue(Client *c)
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
void client_enqueue_stack(Client *c)
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
void client_pop(Client *c)
{
    client_detach(c);
    c->next         = c->mon->clients;
    c->mon->clients = c;
    client_focus(c);
    layout_arrange(c->mon);
    client_pointer_focus_win(c);
}

/**
 * 更新客户端列表
 */
void client_update_list()
{
    Client  *c;
    Monitor *m;

    XDeleteProperty(display, root_window, netatom[NetClientList]);
    for (m = monitor_list; m; m = m->next) {
        for (c = m->clients; c; c = c->next) {
            XChangeProperty(display, root_window, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
                            (unsigned char *)&(c->win), 1);
        }
    }
}

/**
 * 重新堆叠窗口
 */
void client_restack(Monitor *m)
{
    Client        *c;
    XEvent         ev;
    XWindowChanges wc;

    bar_draw_bar(m);
    if (!m->select) {
        return;
    }
    if (m->select->isfloating || !m->lt[m->sellt]->arrange) {
        XRaiseWindow(display, m->select->win);
    }
    if (m->lt[m->sellt]->arrange) {
        wc.stack_mode = Below;
        wc.sibling    = m->bar_window;
        for (c = m->stack; c; c = c->snext) {
            if (!c->isfloating && ISVISIBLE(c)) {
                XConfigureWindow(display, c->win, CWSibling | CWStackMode, &wc);
                wc.sibling = c->win;
            }
        }
    }
    XSync(display, False);
    while (XCheckMaskEvent(display, EnterWindowMask, &ev))
        ;
}

void client_pointer_focus_win(Client *c)
{
    if (c) {
        XWarpPointer(display, None, root_window, 0, 0, 0, 0, c->x + c->w / 2, c->y + c->h / 2);
        client_focus(c);
    } else {
        XWarpPointer(display, None, root_window, 0, 0, 0, 0, select_monitor->wx + select_monitor->ww / 3,
                     select_monitor->wy + select_monitor->wh / 2);
    }
}

/**
 * 重设客户端尺寸
 */
void client_resize(Client *c, int x, int y, int w, int h, int interact)
{
    if (client_apply_size_hints(c, &x, &y, &w, &h, interact)) {
        client_resize_client(c, x, y, w, h);
    }
}

/**
 * 重设客户端尺寸
 */
void client_resize_client(Client *c, int x, int y, int w, int h)
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
    if (((client_next_tiled(c->mon->clients) == c && !client_next_tiled(c->next))
         || &monocle == c->mon->lt[c->mon->sellt]->arrange)
        && !c->isfullscreen && !c->isfloating)
    {
        c->w            = wc.width += c->bw * 2;
        c->h            = wc.height += c->bw * 2;
        wc.border_width = 0;
    }
    XConfigureWindow(display, c->win, CWX | CWY | CWWidth | CWHeight | CWBorderWidth, &wc);
    client_configure(c);
    XSync(display, False);
}

/**
 * 应用客户端尺寸hints
 */
int client_apply_size_hints(Client *c, int *x, int *y, int *w, int *h, int interact)
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
    if (resize_hints || c->isfloating || !c->mon->lt[c->mon->sellt]->arrange) {
        if (!c->hintsvalid)
            client_update_size_hints(c);
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

/**
 * 更新wm hint
 */
void client_update_wm_hints(Client *c)
{
    XWMHints *wmh;

    if ((wmh = XGetWMHints(display, c->win))) {
        if (c == select_monitor->select && wmh->flags & XUrgencyHint) {
            wmh->flags &= ~XUrgencyHint;
            XSetWMHints(display, c->win, wmh);
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
 * 更新客户端尺寸提示
 */
void client_update_size_hints(Client *c)
{
    long       msize;
    XSizeHints size;

    if (!XGetWMNormalHints(display, c->win, &size, &msize)) {
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
void client_update_title(Client *c)
{
    if (!window_get_text_prop(c->win, netatom[NetWMName], c->name, sizeof c->name)) {
        window_get_text_prop(c->win, XA_WM_NAME, c->name, sizeof c->name);
    }
    if (c->name[0] == '\0') { /* hack to mark broken clients */
        strcpy(c->name, broken);
    }
}

/**
 * 更新窗口图标
 */
void client_update_icon(Client *c)
{
    client_free_icon(c);
    c->icon = window_get_icon_prop(c->win, &c->icw, &c->ich);
}

/**
 * 释放窗口图标
 */
void client_free_icon(Client *c)
{
    if (c->icon) {
        XRenderFreePicture(display, c->icon);
        c->icon = None;
    }
}

/**
 * 更新窗口类型
 */
void client_update_window_type(Client *c)
{
    Atom state = client_get_atom_prop(c, netatom[NetWMState]);
    Atom wtype = client_get_atom_prop(c, netatom[NetWMWindowType]);

    if (state == netatom[NetWMFullscreen]) {
        client_set_full_screen(c, 1);
    }
    if (wtype == netatom[NetWMWindowTypeDialog]) {
        c->isfloating = 1;
    }
}

/**
 * 显示和隐藏窗口列表
 */
void client_show_hide(Client *c)
{
    if (!c) {
        return;
    }

    if (ISVISIBLE(c)) {
        /* show clients top down */
        XMoveWindow(display, c->win, c->x, c->y);
        if ((!c->mon->lt[c->mon->sellt]->arrange || c->isfloating) && (!c->isfullscreen || c->isfakefullscreen)) {
            client_resize(c, c->x, c->y, c->w, c->h, 0);
        }
        client_show_hide(c->snext);
    } else {
        /* hide clients bottom up */
        client_show_hide(c->snext);
        XMoveWindow(display, c->win, WIDTH(c) * -2, c->y);  // 隐藏窗口移动到屏幕外
    }
}

/**
 * 设置客户端全屏
 */
void client_set_full_screen(Client *c, int full_screen)
{
    if (full_screen && !c->isfullscreen) {
        XChangeProperty(display, c->win, netatom[NetWMState], XA_ATOM, 32, PropModeReplace,
                        (unsigned char *)&netatom[NetWMFullscreen], 1);
        c->isfullscreen = 1;
        if (c->isfakefullscreen) {
            client_resize_client(c, c->x, c->y, c->w, c->h);
            return;
        }
        c->oldstate   = c->isfloating;
        c->oldbw      = c->bw;
        c->bw         = 0;
        c->isfloating = 1;
        client_resize_client(c, c->mon->mx, c->mon->my, c->mon->mw, c->mon->mh);
        XRaiseWindow(display, c->win);
    } else if (!full_screen && c->isfullscreen) {
        XChangeProperty(display, c->win, netatom[NetWMState], XA_ATOM, 32, PropModeReplace, (unsigned char *)0, 0);
        c->isfullscreen = 0;
        if (c->isfakefullscreen) {
            client_resize_client(c, c->x, c->y, c->w, c->h);
            return;
        }
        c->isfloating = c->oldstate;
        c->bw         = c->oldbw;
        c->x          = c->oldx;
        c->y          = c->oldy;
        c->w          = c->oldw;
        c->h          = c->oldh;
        client_resize_client(c, c->x, c->y, c->w, c->h);
        layout_arrange(c->mon);
    }
}

/**
 * 配置客户端
 */
void client_configure(Client *c)
{
    XConfigureEvent ce;

    ce.type              = ConfigureNotify;
    ce.display           = display;
    ce.event             = c->win;
    ce.window            = c->win;
    ce.x                 = c->x;
    ce.y                 = c->y;
    ce.width             = c->w;
    ce.height            = c->h;
    ce.border_width      = c->bw;
    ce.above             = None;
    ce.override_redirect = False;
    XSendEvent(display, c->win, False, StructureNotifyMask, (XEvent *)&ce);
}

/**
 * 设置紧急性
 */
void client_seturgent(Client *c, int urg)
{
    XWMHints *wmh;

    c->isurgent = urg;
    if (!(wmh = XGetWMHints(display, c->win))) {
        return;
    }
    wmh->flags = urg ? (wmh->flags | XUrgencyHint) : (wmh->flags & ~XUrgencyHint);
    XSetWMHints(display, c->win, wmh);
    XFree(wmh);
}

/**
 * 设置客户端状态
 */
void client_set_state(Client *c, long state)
{
    long data[] = {state, None};

    XChangeProperty(display, c->win, wmatom[WMState], wmatom[WMState], 32, PropModeReplace, (unsigned char *)data, 2);
}

/**
 * 获取atom属性
 */
Atom client_get_atom_prop(Client *c, Atom prop)
{
    int            di;
    unsigned long  dl;
    unsigned char *p = NULL;
    Atom           da, atom = None;

    /* FIXME client_get_atom_prop should return the number of items and a pointer to
     * the stored data instead of this workaround */
    Atom req = XA_ATOM;
    if (prop == xatom[XembedInfo])
        req = xatom[XembedInfo];

    if (XGetWindowProperty(display, c->win, prop, 0L, sizeof atom, False, req, &da, &di, &dl, &dl, &p) == Success && p)
    {
        atom = *(Atom *)p;
        if (da == xatom[XembedInfo] && dl == 2)
            atom = ((Atom *)p)[1];
        XFree(p);
    }
    return atom;
}

/**
 * 注册鼠标按键
 */
void client_grab_buttons(Client *c, int focused)
{
    update_numlock_mask();
    {
        unsigned int i, j;
        unsigned int modifiers[] = {0, LockMask, numlockmask, numlockmask | LockMask};
        XUngrabButton(display, AnyButton, AnyModifier, c->win);
        if (!focused) {
            XGrabButton(display, AnyButton, AnyModifier, c->win, False, BUTTONMASK, GrabModeSync, GrabModeSync, None,
                        None);
        }
        for (i = 0; i < buttons_count(); i++) {
            if (buttons[i].click == ClkClientWin) {
                for (j = 0; j < LENGTH(modifiers); j++) {
                    XGrabButton(display, buttons[i].button, buttons[i].mask | modifiers[j], c->win, False, BUTTONMASK,
                                GrabModeAsync, GrabModeSync, None, None);
                }
            }
        }
    }
}

/**
 * 设置客户端焦点
 */
void client_set_focus(Client *c)
{
    if (!c->neverfocus) {
        XSetInputFocus(display, c->win, RevertToPointerRoot, CurrentTime);
        XChangeProperty(display, root_window, netatom[NetActiveWindow], XA_WINDOW, 32, PropModeReplace,
                        (unsigned char *)&(c->win), 1);
    }
    window_send_event(c->win, wmatom[WMTakeFocus], NoEventMask, wmatom[WMTakeFocus], CurrentTime, 0, 0, 0);
}

/**
 * 焦点位置
 */
void client_focus(Client *c)
{
    if (!c || !ISVISIBLE(c)) {
        // 客户端不存在或者不显示时,切换焦点到第一个可显示的客户端
        for (c = select_monitor->stack; c && (!ISVISIBLE(c) || HIDDEN(c)); c = c->snext)
            ;
    }

    if (select_monitor->select && select_monitor->select != c) {
        client_unfocus(select_monitor->select, 0);

        if (select_monitor->hidsel) {  // 之前焦点窗口为隐藏窗口, 焦点取消后继续隐藏
            client_hide(select_monitor->select);
            if (c) {
                layout_arrange(c->mon);  // 隐藏窗口后重新布局
            }
            select_monitor->hidsel = 0;
        }
    }

    if (c) {
        if (c->mon != select_monitor) {
            select_monitor = c->mon;
        }
        if (c->isurgent) {
            client_seturgent(c, 0);
        }
        client_detach_stack(c);
        client_attach_stack(c);  // 重新附加客户端栈
        client_grab_buttons(c, 1);
        XSetWindowBorder(display, c->win, scheme[SchemeSel][ColBorder].pixel);
        client_set_focus(c);
    } else {
        XSetInputFocus(display, root_window, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(display, root_window, netatom[NetActiveWindow]);
    }

    select_monitor->select = c;
    bar_draw_bars();  // 更新Bar
}

/**
 * 取消焦点
 */
void client_unfocus(Client *c, int client_set_focus)
{
    if (!c) {
        return;
    }
    client_grab_buttons(c, 0);
    XSetWindowBorder(display, c->win, scheme[SchemeNorm][ColBorder].pixel);
    if (client_set_focus) {
        XSetInputFocus(display, root_window, RevertToPointerRoot, CurrentTime);
        XDeleteProperty(display, root_window, netatom[NetActiveWindow]);
    }
}

/**
 * 应用规则
 */
void client_apply_rules(Client *c)
{
    const char *class, *instance;
    unsigned int i;
    const Rule  *r;
    Monitor     *m;
    XClassHint   ch = {NULL, NULL};

    /* rule matching */
    c->isfloating = 0;
    c->tags       = 0;
    c->isnoborder = 0;
    XGetClassHint(display, c->win, &ch);
    class    = ch.res_class ? ch.res_class : broken;
    instance = ch.res_name ? ch.res_name : broken;

    for (i = 0; i < rules_count(); i++) {
        r = &rules[i];
        if ((!r->title || strstr(c->name, r->title)) && (!r->class || strstr(class, r->class))
            && (!r->instance || strstr(instance, r->instance)))
        {
            c->isfloating       = r->isfloating;
            c->isfakefullscreen = r->isfakefullscreen;
            c->nooverview       = r->nooverview;
            c->isnoborder = r->noborder;
            c->bw = c->isnoborder ? 0 : border_px;
            c->tags |= r->tags;
            for (m = monitor_list; m && m->num != r->monitor; m = m->next)
                ;
            if (m) {
                c->mon = m;
            }
            // 如果设定了floatposition 且未指定xy，设定窗口位置
            if (r->isfloating && c->x == 0 && c->y == 0) {
                switch (r->floatposition) {
                case 1:
                    c->x = select_monitor->wx + gappx;
                    c->y = select_monitor->wy + gappx;
                    break;  // 左上
                case 2:
                    c->x = select_monitor->wx + (select_monitor->ww - WIDTH(c)) / 2 - gappx;
                    c->y = select_monitor->wy + gappx;
                    break;  // 中上
                case 3:
                    c->x = select_monitor->wx + select_monitor->ww - WIDTH(c) - gappx;
                    c->y = select_monitor->wy + gappx;
                    break;  // 右上
                case 4:
                    c->x = select_monitor->wx + gappx;
                    c->y = select_monitor->wy + (select_monitor->wh - HEIGHT(c)) / 2;
                    break;  // 左中
                case 0:     // 默认0，居中
                case 5:
                    c->x = select_monitor->wx + (select_monitor->ww - WIDTH(c)) / 2;
                    c->y = select_monitor->wy + (select_monitor->wh - HEIGHT(c)) / 2;
                    break;  // 中中
                case 6:
                    c->x = select_monitor->wx + select_monitor->ww - WIDTH(c) - gappx;
                    c->y = select_monitor->wy + (select_monitor->wh - HEIGHT(c)) / 2;
                    break;  // 右中
                case 7:
                    c->x = select_monitor->wx + gappx;
                    c->y = select_monitor->wy + select_monitor->wh - HEIGHT(c) - gappx;
                    break;  // 左下
                case 8:
                    c->x = select_monitor->wx + (select_monitor->ww - WIDTH(c)) / 2;
                    c->y = select_monitor->wy + select_monitor->wh - HEIGHT(c) - gappx;
                    break;  // 中下
                case 9:
                    c->x = select_monitor->wx + select_monitor->ww - WIDTH(c) - gappx;
                    c->y = select_monitor->wy + select_monitor->wh - HEIGHT(c) - gappx;
                    break;  // 右下
                }
            }
            break;  // 有且只会匹配一个第一个符合的rule
        }
    }
    if (ch.res_class) {
        XFree(ch.res_class);
    }
    if (ch.res_name) {
        XFree(ch.res_name);
    }
    c->tags = c->tags & TAGMASK ? c->tags & TAGMASK : c->mon->tagset[c->mon->seltags];
}

/**
 * 管理窗口
 */
void client_manage(Window w, XWindowAttributes *wa)
{
    Client        *c, *t = NULL;
    Window         trans = None;
    XWindowChanges wc;

    c      = ecalloc(1, sizeof(Client));
    c->win = w;
    /* geometry */
    c->x = c->oldx = wa->x;
    c->y = c->oldy = wa->y;
    c->w = c->oldw = wa->width;
    c->h = c->oldh = wa->height;
    c->oldbw       = wa->border_width;


    client_update_icon(c);
    client_update_title(c);
    fprintf(stderr, "name = %s, wa->x = %d, wa->y = %d \n",c->name, wa->x, wa->y );
    if (XGetTransientForHint(display, w, &trans) && (t = window_to_client(trans))) {
        c->mon  = t->mon;
        c->tags = t->tags;
    } else {
        c->mon = select_monitor;
        client_apply_rules(c);
    }

    if (c->x + WIDTH(c) > c->mon->wx + c->mon->ww) {
        c->x = c->mon->wx + c->mon->ww - WIDTH(c);
    }
    if (c->y + HEIGHT(c) > c->mon->wy + c->mon->wh) {
        c->y = c->mon->wy + c->mon->wh - HEIGHT(c);
    }
    c->x  = MAX(c->x, c->mon->wx);
    c->y  = MAX(c->y, c->mon->wy);
    wc.border_width = c->bw;

    select_monitor->tagset[select_monitor->seltags] &= ~scratchtag;
    if (!strcmp(c->name, scratchpadname)) {  // 便笺薄
        c->mon->tagset[c->mon->seltags] |= c->tags = scratchtag;
        c->isfloating                              = True;
        c->nooverview                              = True;
        c->x                                       = c->mon->wx + (c->mon->ww / 2 - WIDTH(c) / 2);
        c->y                                       = c->mon->wy + (c->mon->wh / 2 - HEIGHT(c) / 2);
    }

    XConfigureWindow(display, w, CWBorderWidth, &wc);
    XSetWindowBorder(display, w, scheme[SchemeNorm][ColBorder].pixel);
    client_configure(c); /* propagates border_width, if size doesn't change */
    client_update_window_type(c);
    client_update_size_hints(c);
    client_update_wm_hints(c);
    XSelectInput(display, w, EnterWindowMask | FocusChangeMask | PropertyChangeMask | StructureNotifyMask);
    client_grab_buttons(c, 0);
    if (!c->isfloating) {
        c->isfloating = c->oldstate = trans != None || c->isfixed;
    }
    if (c->isfloating) {
        XRaiseWindow(display, c->win);
    }
    client_attach(c);
    client_attach_stack(c);
    XChangeProperty(display, root_window, netatom[NetClientList], XA_WINDOW, 32, PropModeAppend,
                    (unsigned char *)&(c->win), 1);
    XMoveResizeWindow(display, c->win, c->x + 2 * screen_width, c->y, c->w, c->h); /* some windows require this */
    if (!HIDDEN(c)) {
        client_set_state(c, NormalState);
    }
    if (c->mon == select_monitor) {
        client_unfocus(select_monitor->select, 0);
    }
    c->mon->select = c;
    layout_arrange(c->mon);
    if (!HIDDEN(c)) {
        XMapWindow(display, c->win);
    }
    client_focus(NULL);
}

/**
 * 不再管理窗口
 */
void client_unmanage(Client *c, int destroyed)
{
    Monitor       *m = c->mon;
    XWindowChanges wc;

    client_detach(c);
    client_detach_stack(c);
    client_free_icon(c);
    if (!destroyed) {
        wc.border_width = c->oldbw;
        XGrabServer(display); /* avoid race conditions */
        XSetErrorHandler(xerrordummy);
        XSelectInput(display, c->win, NoEventMask);
        XConfigureWindow(display, c->win, CWBorderWidth, &wc); /* restore border */
        XUngrabButton(display, AnyButton, AnyModifier, c->win);
        client_set_state(c, WithdrawnState);
        XSync(display, False);
        XSetErrorHandler(xerror);
        XUngrabServer(display);
    }
    free(c);
    client_focus(NULL);
    client_update_list();
    layout_arrange(m);
}

/**
 * 将客户端发送到监视器
 */
void client_send_to_monitor(Client *c, Monitor *m)
{
    if (c->mon == m)
        return;
    client_unfocus(c, 1);
    client_detach(c);
    client_detach_stack(c);
    c->mon  = m;
    c->tags = m->tagset[m->seltags]; /* assign tags of target monitor */
    client_attach(c);
    client_attach_stack(c);
    client_focus(NULL);
    layout_arrange(NULL);
}

/**
 * 下一个平铺的客户端
 */
Client *client_next_tiled(Client *c)
{
    for (; c && (c->isfloating || !ISVISIBLE(c) || HIDDEN(c)); c = c->next)
        ;
    return c;
}

/**
 * 显示窗口
 */
void client_show(Client *c)
{
    if (!c || !HIDDEN(c))
        return;
        
    XMapWindow(display, c->win);
    client_set_state(c, NormalState);
    layout_arrange(c->mon);

    hidden_win_stack_top--;
}

/**
 * 隐藏客户端
 */
void client_hide(Client *c)
{
    if (!c || HIDDEN(c))
        return;

    Window                   w = c->win;
    static XWindowAttributes ra, ca;

    // more or less taken directly from blackbox's hide_window() function
    XGrabServer(display);
    XGetWindowAttributes(display, root_window, &ra);
    XGetWindowAttributes(display, w, &ca);
    // prevent UnmapNotify events
    XSelectInput(display, root_window, ra.your_event_mask & ~SubstructureNotifyMask);
    XSelectInput(display, w, ca.your_event_mask & ~StructureNotifyMask);
    XUnmapWindow(display, w);
    client_set_state(c, IconicState);
    XSelectInput(display, root_window, ra.your_event_mask);
    XSelectInput(display, w, ca.your_event_mask);
    XUngrabServer(display);

    hidden_win_stack[++hidden_win_stack_top] = c;
}

/**
 * 隐藏窗口
 */
void hide_client(const Arg *arg)
{
    client_hide(select_monitor->select);
    client_focus(NULL);
    layout_arrange(select_monitor);
}

/**
 * 显示客户端
 */
void show_client(const Arg *arg)
{
    int i = hidden_win_stack_top;
    Client *c;
    while (i > -1) {
        c= hidden_win_stack[i];
        if (HIDDEN(c) && ISVISIBLE(c)) {
            client_show(c);
            client_focus(c);
            client_restack(select_monitor);
            for (int j = i; j < hidden_win_stack_top+1; ++j) {
                hidden_win_stack[j] = hidden_win_stack[j + 1];
            }
            return;
        }
        --i;
    }
}

/**
 * 显示全部客户端
 */
void show_all_client(const Arg *arg)
{
    Client *c              = NULL;
    select_monitor->hidsel = 0;
    for (c = select_monitor->clients; c; c = c->next) {
        if (ISVISIBLE(c)) {
            client_show(c);
        }
    }
    if (!select_monitor->select) {
        for (c = select_monitor->clients; c && !ISVISIBLE(c); c = c->next)
            ;
        if (c) {
            client_focus(c);
        }
    }
    client_restack(select_monitor);
}

/**
 * 杀死客户端
 */
void kill_client(const Arg *arg)
{
    if (!select_monitor->select) {
        return;
    }
    if (!window_send_event(select_monitor->select->win, wmatom[WMDelete], NoEventMask, wmatom[WMDelete], CurrentTime, 0,
                           0, 0))
    {
        XGrabServer(display);
        XSetErrorHandler(xerrordummy);
        XSetCloseDownMode(display, DestroyAll);
        XKillClient(display, select_monitor->select->win);
        XSync(display, False);
        XSetErrorHandler(xerror);
        XUngrabServer(display);
    }
}

/**
 * 强制关闭窗口(处理某些情况下无法销毁的窗口)
 */
void force_kill_client(const Arg *arg)
{
    if (!select_monitor->select) {
        return;
    }

    kill_client(arg);
    client_unmanage(select_monitor->select, 1);
}

/**
 * 旋转窗口栈
 */
void rotate_client_stack(const Arg *arg)
{
    Client *c = NULL, *f;

    if (!select_monitor->select) {
        return;
    }
    f = select_monitor->select;
    if (arg->i > 0) {
        for (c = client_next_tiled(select_monitor->clients); c && client_next_tiled(c->next);
             c = client_next_tiled(c->next))
            ;
        if (c) {
            client_detach(c);
            client_attach(c);
            client_detach_stack(c);
            client_attach_stack(c);
        }
    } else {
        if ((c = client_next_tiled(select_monitor->clients))) {
            client_detach(c);
            client_enqueue(c);
            client_detach_stack(c);
            client_enqueue_stack(c);
        }
    }
    if (c) {
        layout_arrange(select_monitor);
        // client_unfocus(f, 1);
        client_focus(f);
        client_restack(select_monitor);
    }
}

/**
 * 鼠标调整窗口大小
 */
void resize_by_mouse(const Arg *arg)
{
    int      ocx, ocy, nw, nh;
    Client  *c;
    Monitor *m;
    XEvent   ev;
    Time     lasttime = 0;

    if (!(c = select_monitor->select)) {
        return;
    }
    if (c->isfullscreen && !c->isfakefullscreen) { /* no support resizing full_screen windows by mouse */
        return;
    }
    client_restack(select_monitor);
    ocx = c->x;
    ocy = c->y;
    if (XGrabPointer(display, root_window, False, MOUSEMASK, GrabModeAsync, GrabModeAsync, None,
                     cursor[CurResize]->cursor, CurrentTime)
        != GrabSuccess)
    {
        return;
    }
    XWarpPointer(display, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
    do {
        XMaskEvent(display, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
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
                    toggle_floating(NULL);
                }
            }
            if (!select_monitor->lt[select_monitor->sellt]->arrange || c->isfloating) {
                client_resize(c, c->x, c->y, nw, nh, 1);
            }
            break;
        }
    } while (ev.type != ButtonRelease);
    XWarpPointer(display, None, c->win, 0, 0, 0, 0, c->w + c->bw - 1, c->h + c->bw - 1);
    XUngrabPointer(display, CurrentTime);
    while (XCheckMaskEvent(display, EnterWindowMask, &ev))
        ;
    if ((m = monitor_rect_to_monitor(c->x, c->y, c->w, c->h)) != select_monitor) {
        client_send_to_monitor(c, m);
        select_monitor = m;
        client_focus(NULL);
    }
}

/**
 * 鼠标移动窗口
 */
void move_by_mouse(const Arg *arg)
{
    int      x, y, ocx, ocy, nx, ny;
    Client  *c;
    Monitor *m;
    XEvent   ev;
    Time     lasttime = 0;

    if (!(c = select_monitor->select)) {
        return;
    }
    if (c->isfullscreen && !c->isfakefullscreen) { /* no support moving full_screen windows by mouse */
        return;
    }
    client_restack(select_monitor);
    ocx = c->x;
    ocy = c->y;
    if (XGrabPointer(display, root_window, False, MOUSEMASK, GrabModeAsync, GrabModeAsync, None,
                     cursor[CurMove]->cursor, CurrentTime)
        != GrabSuccess)
    {
        return;
    }
    if (!window_get_root_ptr(&x, &y)) {
        return;
    }
    do {
        XMaskEvent(display, MOUSEMASK | ExposureMask | SubstructureRedirectMask, &ev);
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
                toggle_floating(NULL);
            }
            if (!select_monitor->lt[select_monitor->sellt]->arrange || c->isfloating) {
                client_resize(c, nx, ny, c->w, c->h, 1);
            }
            break;
        }
    } while (ev.type != ButtonRelease);
    XUngrabPointer(display, CurrentTime);
    if ((m = monitor_rect_to_monitor(c->x, c->y, c->w, c->h)) != select_monitor) {
        client_send_to_monitor(c, m);
        select_monitor = m;
        client_focus(NULL);
    }
}

/**
 * 将当前聚焦窗口置为主窗口
 */
void zoom(const Arg *arg)
{
    Client *c = select_monitor->select;

    if (c && (c->isfloating || c->isfullscreen)) {
        return;
    }

    if (c == client_next_tiled(select_monitor->clients)) {
        if (!c || !(c = client_next_tiled(c->next))) {
            return;
        }
    }

    client_pop(c);
}

/**
 * 切换窗口焦点
 */
void focus_stack(int inc, int hid)
{
    Client *c = NULL, *i;
    // if no client selected AND exclude hidden client; if client selected but
    // fullscreened
    if ((!select_monitor->select && !hid)
        || (select_monitor->select && select_monitor->select->isfullscreen && lock_full_screen))
    {
        return;
    }
    if (!select_monitor->clients) {
        return;
    }
    if (inc > 0) {
        if (select_monitor->select) {
            for (c = select_monitor->select->next; c && (!ISVISIBLE(c) || (!hid && HIDDEN(c))); c = c->next)
                ;
        }
        if (!c) {
            for (c = select_monitor->clients; c && (!ISVISIBLE(c) || (!hid && HIDDEN(c))); c = c->next)
                ;
        }
    } else {
        if (select_monitor->select) {
            for (i = select_monitor->clients; i != select_monitor->select; i = i->next) {
                if (ISVISIBLE(i) && !(!hid && HIDDEN(i))) {
                    c = i;
                }
            }
        } else {
            c = select_monitor->clients;
        }
        if (!c)
            for (; i; i = i->next) {
                if (ISVISIBLE(i) && !(!hid && HIDDEN(i))) {
                    c = i;
                }
            }
    }
    if (c) {
        client_focus(c);
        client_restack(select_monitor);
        if (HIDDEN(c)) {
            client_show(c);
            c->mon->hidsel = 1;
        }
        client_pointer_focus_win(c);
    }
}

/**
 * 切换显示窗口焦点
 */
void focusstackvis(const Arg *arg)
{
    focus_stack(arg->i, 0);
}

/**
 * 切换隐藏窗口焦点
 */
void focusstackhid(const Arg *arg)
{
    focus_stack(arg->i, 1);
}

/**
 * 切换窗口显示状态
 */
void toggle_window(const Arg *arg)
{
    Client *c = (Client *)arg->v;

    if (c == NULL) {
        return;
    }

    if (c == select_monitor->select) {
        client_hide(c);
        client_focus(NULL);
        layout_arrange(c->mon);
    } else {
        if (HIDDEN(c)) {
            client_show(c);
        }
        client_focus(c);
        client_restack(select_monitor);
    }
}

/**
 * 切换当前客户端浮动
 */
void toggle_floating(const Arg *arg)
{
    if (!select_monitor->select)
        return;

    if (select_monitor->select->isfullscreen
        && !select_monitor->select->isfakefullscreen) /* no support for full_screen windows */
        return;

    select_monitor->select->isfloating = !select_monitor->select->isfloating || select_monitor->select->isfixed;
    if (select_monitor->select->isfloating) {
        // client_resize(select_monitor->select, select_monitor->select->x, select_monitor->select->y,
        // select_monitor->select->w, select_monitor->select->h, 0);
        client_resize(select_monitor->select, select_monitor->wx + select_monitor->ww / 6,
                      select_monitor->wy + select_monitor->wh / 6, select_monitor->ww / 3 * 2,
                      select_monitor->wh / 3 * 2, 0);
    }
    layout_arrange(select_monitor);
    client_pointer_focus_win(select_monitor->select);
}

/**
 * 切换全部浮动
 */
void toggle_all_floating(const Arg *arg)
{
    Client *c            = NULL;
    int     somefloating = 0;

    if (!select_monitor->select || select_monitor->select->isfullscreen) {
        return;
    }

    for (c = select_monitor->clients; c; c = c->next) {
        if (ISVISIBLE(c) && !HIDDEN(c) && c->isfloating) {
            somefloating = 1;
            break;
        }
    }

    if (somefloating) {
        for (c = select_monitor->clients; c; c = c->next) {
            if (ISVISIBLE(c) && !HIDDEN(c)) {
                c->isfloating = 0;
            }
        }
        layout_arrange(select_monitor);
    } else {
        for (c = select_monitor->clients; c; c = c->next) {
            if (ISVISIBLE(c) && !HIDDEN(c)) {
                c->isfloating = 1;
                client_resize(c, c->x + 2 * snap, c->y + 2 * snap, MAX(c->w - 4 * snap, snap),
                              MAX(c->h - 4 * snap, snap), 0);
            }
        }
    }
    client_pointer_focus_win(select_monitor->select);
}

/**
 * 全屏
 */
Layout *last_layout;
void    toggle_full_screen(const Arg *arg)
{
    if (select_monitor->showbar) {
        for (last_layout = (Layout *)layouts; last_layout != select_monitor->lt[select_monitor->sellt]; last_layout++)
            ;
        set_layout(&((Arg){.v = &layouts[2]}));
    } else {
        set_layout(&((Arg){.v = last_layout}));
    }
    toggle_bar(arg);
}

void toggle_fake_full_screen(const Arg *arg)
{
    Client *c = select_monitor->select;
    if (!c)
        return;

    c->isfakefullscreen = !c->isfakefullscreen;

    client_set_full_screen(c, (!c->isfullscreen || c->isfakefullscreen));
}

/**
 * 隐藏其他窗口仅保留该窗口
 */
void hide_other_wins(const Arg *arg)
{
    Client *c = (Client *)arg->v, *tc = NULL;
    for (tc = select_monitor->clients; tc; tc = tc->next) {
        if (tc != c && ISVISIBLE(tc)) {
            client_hide(tc);
        }
    }
    client_show(c);
    client_focus(c);
    layout_arrange(c->mon);
}

int is_single_win(const Arg *arg)
{
    Client *c   = NULL;
    int     cot = 0;

    for (c = select_monitor->clients; c; c = c->next) {
        if (ISVISIBLE(c) && !HIDDEN(c))
            cot++;
        if (cot > 1)
            return 0;
    }
    return 1;
}

/**
 * 切换 只显示一个窗口 / 全部显示
 */
void show_only_or_all(const Arg *arg)
{
    Client *c;
    if (is_single_win(NULL) || !select_monitor->select) {
        for (c = select_monitor->clients; c; c = c->next) {
            if (ISVISIBLE(c)) {
                client_show(c);
            }
        }
    } else {
        hide_other_wins(&(Arg){.v = select_monitor->select});
    }
}

/**
 * 移动窗口
 */
void move_window(const Arg *arg)
{
    Client *c, *tc;
    int     nx, ny;
    int     buttom, top, left, right, tar;
    c = select_monitor->select;
    if (!c || c->isfullscreen) {
        return;
    }

    if (!c->isfloating) {
        toggle_floating(NULL);
    }

    nx = c->x;
    ny = c->y;
    switch (arg->ui) {
    case UP:
        tar = -99999;
        top = c->y;
        ny -= c->mon->wh / 20;
        for (tc = c->mon->clients; tc; tc = tc->next) {
            // 若浮动tc c的顶边会穿过tc的底边
            if (!ISVISIBLE(tc) || !tc->isfloating || tc == c) {
                continue;
            }
            if (c->x + WIDTH(c) < tc->x || c->x > tc->x + WIDTH(tc)) {
                continue;
            }
            buttom = tc->y + HEIGHT(tc) + gappx;
            if (top > buttom && ny < buttom) {
                tar = MAX(tar, buttom);
            };
        }
        ny = tar == -99999 ? ny : tar;
        ny = MAX(ny, c->mon->wy + gappx);
        break;
    case DOWN:
        tar    = 99999;
        buttom = c->y + HEIGHT(c);
        ny += c->mon->wh / 20;
        for (tc = c->mon->clients; tc; tc = tc->next) {
            // 若浮动tc c的底边会穿过tc的顶边
            if (!ISVISIBLE(tc) || !tc->isfloating || tc == c) {
                continue;
            }
            if (c->x + WIDTH(c) < tc->x || c->x > tc->x + WIDTH(tc)) {
                continue;
            }
            top = tc->y - gappx;
            if (buttom < top && (ny + HEIGHT(c)) > top) {
                tar = MIN(tar, top - HEIGHT(c));
            };
        }
        ny = tar == 99999 ? ny : tar;
        ny = MIN(ny, c->mon->wy + c->mon->wh - gappx - HEIGHT(c));
        break;
    case LEFT:
        tar  = -99999;
        left = c->x;
        nx -= c->mon->ww / 40;
        for (tc = c->mon->clients; tc; tc = tc->next) {
            // 若浮动tc c的左边会穿过tc的右边
            if (!ISVISIBLE(tc) || !tc->isfloating || tc == c) {
                continue;
            }
            if (c->y + HEIGHT(c) < tc->y || c->y > tc->y + HEIGHT(tc)) {
                continue;
            }
            right = tc->x + WIDTH(tc) + gappx;
            if (left > right && nx < right) {
                tar = MAX(tar, right);
            };
        }
        nx = tar == -99999 ? nx : tar;
        nx = MAX(nx, c->mon->wx + gappx);
        break;
    case RIGHT:
        tar   = 99999;
        right = c->x + WIDTH(c);
        nx += c->mon->ww / 40;
        for (tc = c->mon->clients; tc; tc = tc->next) {
            // 若浮动tc c的右边会穿过tc的左边
            if (!ISVISIBLE(tc) || !tc->isfloating || tc == c) {
                continue;
            }
            if (c->y + HEIGHT(c) < tc->y || c->y > tc->y + HEIGHT(tc)) {
                continue;
            }
            left = tc->x - gappx;
            if (right < left && (nx + WIDTH(c)) > left) {
                tar = MIN(tar, left - WIDTH(c));
            };
        }
        nx = tar == 99999 ? nx : tar;
        nx = MIN(nx, c->mon->wx + c->mon->ww - gappx - WIDTH(c));
        break;
    }
    client_resize(c, nx, ny, c->w, c->h, 1);
    client_pointer_focus_win(c);
    client_restack(select_monitor);
}


/**
 * 调整窗口
 */
void resize_window(const Arg *arg)
{
    Client *c, *tc;
    int     nh, nw;
    int     buttom, top, left, right, tar;
    c = select_monitor->select;
    if (!c || c->isfullscreen) {
        return;
    }
    if (!c->isfloating) {
        toggle_floating(NULL);
    }
    nw = c->w;
    nh = c->h;
    switch (arg->ui) {
    case H_EXPAND:  // 右
        tar   = 99999;
        right = c->x + WIDTH(c);
        nw += select_monitor->ww / 40;
        for (tc = c->mon->clients; tc; tc = tc->next) {
            // 若浮动tc c的右边会穿过tc的左边
            if (!ISVISIBLE(tc) || !tc->isfloating || tc == c) {
                continue;
            }
            if (c->y + HEIGHT(c) < tc->y || c->y > tc->y + HEIGHT(tc)) {
                continue;
            }
            left = tc->x - gappx;
            if (right < left && (c->x + nw) > left) {
                tar = MIN(tar, left - c->x - 2 * c->bw);
            };
        }
        nw = tar == 99999 ? nw : tar;
        if (c->x + nw + gappx + 2 * c->bw > select_monitor->wx + select_monitor->ww) {
            nw = select_monitor->wx + select_monitor->ww - c->x - gappx - 2 * c->bw;
        }
        break;
    case H_REDUCE:  // 左
        nw -= select_monitor->ww / 40;
        nw = MAX(nw, select_monitor->ww / 10);
        break;
    case V_EXPAND:  // 下
        tar    = -99999;
        buttom = c->y + HEIGHT(c);
        nh += select_monitor->wh / 80;
        for (tc = c->mon->clients; tc; tc = tc->next) {
            // 若浮动tc c的底边会穿过tc的顶边
            if (!ISVISIBLE(tc) || !tc->isfloating || tc == c) {
                continue;
            }
            if (c->x + WIDTH(c) < tc->x || c->x > tc->x + WIDTH(tc)) {
                continue;
            }
            top = tc->y - gappx;
            if (buttom < top && (c->y + nh) > top) {
                tar = MAX(tar, top - c->y - 2 * c->bw);
            };
        }
        nh = tar == -99999 ? nh : tar;
        if (c->y + nh + gappx + 2 * c->bw > select_monitor->wy + select_monitor->wh) {
            nh = select_monitor->wy + select_monitor->wh - c->y - gappx - 2 * c->bw;
        }
        break;
    case V_REDUCE:  // 上
        nh -= select_monitor->wh / 80;
        nh = MAX(nh, select_monitor->wh / 10);
        break;
    }
    client_resize(c, c->x, c->y, nw, nh, 1);
    XWarpPointer(display, None, root_window, 0, 0, 0, 0, c->x + c->w - 2 * c->bw, c->y + c->h - 2 * c->bw);
    client_restack(select_monitor);
}
