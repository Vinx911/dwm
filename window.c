
#include "dwm.h"
#include "client.h"
#include "monitor.h"
#include "layout.h"
#include "systray.h"
#include "window.h"
#include "config.h"

/**
 * 窗口所属的客户端
 */
Client *window_to_client(Window w)
{
    Client  *c;
    Monitor *m;

    for (m = monitor_list; m; m = m->next) {
        for (c = m->clients; c; c = c->next) {
            if (c->win == w) {
                return c;
            }
        }
    }
    return NULL;
}


/**
 * 获取根点
 */
int window_get_root_ptr(int *x, int *y)
{
    int          di;
    unsigned int dui;
    Window       dummy;

    return XQueryPointer(display, root_window, &dummy, &dummy, x, y, &di, &di, &dui);
}

/**
 * 窗口所在的监视器
 */
Monitor *window_to_monitor(Window w)
{
    int      x, y;
    Client  *c;
    Monitor *m;

    if (w == root_window && window_get_root_ptr(&x, &y)) {
        return monitor_rect_to_monitor(x, y, 1, 1);
    }

    for (m = monitor_list; m; m = m->next) {
        if (w == m->bar_window) {
            return m;
        }
    }

    if ((c = window_to_client(w))) {
        return c->mon;
    }

    return select_monitor;
}


/**
 * 窗口的系统托盘图标
 */
Client *window_to_systray_icon(Window w)
{
    Client *i = NULL;

    if (!show_systray || !w) {
        return i;
    }
    for (i = systray->icons; i && i->win != w; i = i->next)
        ;
    return i;
}


/**
 * 获取窗口状态
 */
long window_get_state(Window w)
{
    int            format;
    long           result = -1;
    unsigned char *p      = NULL;
    unsigned long  n, extra;
    Atom           real;

    if (XGetWindowProperty(display, w, wmatom[WMState], 0L, 2L, False, wmatom[WMState], &real, &format, &n, &extra,
                           (unsigned char **)&p)
        != Success)
    {
        return -1;
    }
    if (n != 0) {
        result = *p;
    }
    XFree(p);
    return result;
}

/**
 * 获取文本属性
 */
int window_get_text_prop(Window w, Atom atom, char *text, unsigned int size)
{
    char        **list = NULL;
    int           n;
    XTextProperty name;

    if (!text || size == 0) {
        return 0;
    }
    text[0] = '\0';
    if (!XGetTextProperty(display, w, &name, atom) || !name.nitems) {
        return 0;
    }
    if (name.encoding == XA_STRING) {
        strncpy(text, (char *)name.value, size - 1);
    } else if (XmbTextPropertyToTextList(display, &name, &list, &n) >= Success && n > 0 && *list) {
        strncpy(text, *list, size - 1);
        XFreeStringList(list);
    }
    text[size - 1] = '\0';
    XFree(name.value);
    return 1;
}

static uint32_t prealpha(uint32_t p)
{
    uint8_t  a  = p >> 24u;
    uint32_t rb = (a * (p & 0xFF00FFu)) >> 8u;
    uint32_t g  = (a * (p & 0x00FF00u)) >> 8u;
    return (rb & 0xFF00FFu) | (g & 0x00FF00u) | (a << 24u);
}

/**
 * 获取图标
 */
Picture window_get_icon_prop(Window win, unsigned int *picw, unsigned int *pich)
{
    int           format;
    unsigned long n, extra, *p = NULL;
    Atom          real;

    if (XGetWindowProperty(display, win, netatom[NetWMIcon], 0L, LONG_MAX, False, AnyPropertyType, &real, &format, &n,
                           &extra, (unsigned char **)&p)
        != Success)
        return None;
    if (n == 0 || format != 32) {
        XFree(p);
        return None;
    }

    unsigned long *bstp = NULL;
    uint32_t       w, h, sz;
    {
        unsigned long       *i;
        const unsigned long *end  = p + n;
        uint32_t             bstd = UINT32_MAX, d, m;
        for (i = p; i < end - 1; i += sz) {
            if ((w = *i++) >= 16384 || (h = *i++) >= 16384) {
                XFree(p);
                return None;
            }
            if ((sz = w * h) > end - i)
                break;
            if ((m = w > h ? w : h) >= winiconsize && (d = m - winiconsize) < bstd) {
                bstd = d;
                bstp = i;
            }
        }
        if (!bstp) {
            for (i = p; i < end - 1; i += sz) {
                if ((w = *i++) >= 16384 || (h = *i++) >= 16384) {
                    XFree(p);
                    return None;
                }
                if ((sz = w * h) > end - i)
                    break;
                if ((d = winiconsize - (w > h ? w : h)) < bstd) {
                    bstd = d;
                    bstp = i;
                }
            }
        }
        if (!bstp) {
            XFree(p);
            return None;
        }
    }

    if ((w = *(bstp - 2)) == 0 || (h = *(bstp - 1)) == 0) {
        XFree(p);
        return None;
    }

    uint32_t icw, ich;
    if (w <= h) {
        ich = winiconsize;
        icw = w * winiconsize / h;
        if (icw == 0)
            icw = 1;
    } else {
        icw = winiconsize;
        ich = h * winiconsize / w;
        if (ich == 0)
            ich = 1;
    }
    *picw = icw;
    *pich = ich;

    uint32_t i, *bstp32 = (uint32_t *)bstp;
    for (sz = w * h, i = 0; i < sz; ++i)
        bstp32[i] = prealpha(bstp[i]);

    Picture ret = drw_picture_create_resized(drw, (char *)bstp, w, h, icw, ich);
    XFree(p);

    return ret;
}

/**
 * 隐藏客户端
 */
void window_hide(Client *c)
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
}

/**
 * 发送事件
 */
int window_send_event(Window w, Atom proto, int mask, long d0, long d1, long d2, long d3, long d4)
{
    int    n;
    Atom  *protocols, mt;
    int    exists = 0;
    XEvent ev;

    if (proto == wmatom[WMTakeFocus] || proto == wmatom[WMDelete]) {
        mt = wmatom[WMProtocols];
        if (XGetWMProtocols(display, w, &protocols, &n)) {
            while (!exists && n--)
                exists = protocols[n] == proto;
            XFree(protocols);
        }
    } else {
        exists = True;
        mt     = proto;
    }
    if (exists) {
        ev.type                 = ClientMessage;
        ev.xclient.window       = w;
        ev.xclient.message_type = mt;
        ev.xclient.format       = 32;
        ev.xclient.data.l[0]    = d0;
        ev.xclient.data.l[1]    = d1;
        ev.xclient.data.l[2]    = d2;
        ev.xclient.data.l[3]    = d3;
        ev.xclient.data.l[4]    = d4;
        XSendEvent(display, w, False, mask, &ev);
    }
    return exists;
}

/**
 * 隐藏窗口
 */
void hide_window(const Arg *arg)
{
    window_hide(select_monitor->select);
    client_focus(NULL);
    layout_arrange(select_monitor);
}
