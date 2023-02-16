
#include "dwm.h"
#include "client.h"
#include "monitor.h"
#include "layout.h"
#include "window.h"
#include "config.h"

/**
 * 窗口所属的客户端
 */
Client *window_to_client(Window w)
{
    Client  *c;
    Monitor *m;

    for (m = mons; m; m = m->next) {
        for (c = m->clients; c; c = c->next) {
            if (c->win == w) {
                return c;
            }
        }
    }
    return NULL;
}

/**
 * 窗口所在的监视器
 */
Monitor *window_to_monitor(Window w)
{
    int      x, y;
    Client  *c;
    Monitor *m;

    if (w == root && getrootptr(&x, &y)) {
        return recttomon(x, y, 1, 1);
    }

    for (m = mons; m; m = m->next) {
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
 * 获取窗口状态
 */
long getstate(Window w)
{
    int            format;
    long           result = -1;
    unsigned char *p      = NULL;
    unsigned long  n, extra;
    Atom           real;

    if (XGetWindowProperty(dpy, w, wmatom[WMState], 0L, 2L, False, wmatom[WMState], &real, &format, &n, &extra,
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
int gettextprop(Window w, Atom atom, char *text, unsigned int size)
{
    char        **list = NULL;
    int           n;
    XTextProperty name;

    if (!text || size == 0) {
        return 0;
    }
    text[0] = '\0';
    if (!XGetTextProperty(dpy, w, &name, atom) || !name.nitems) {
        return 0;
    }
    if (name.encoding == XA_STRING) {
        strncpy(text, (char *)name.value, size - 1);
    } else if (XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success && n > 0 && *list) {
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
Picture geticonprop(Window win, unsigned int *picw, unsigned int *pich)
{
    int           format;
    unsigned long n, extra, *p = NULL;
    Atom          real;

    if (XGetWindowProperty(dpy, win, netatom[NetWMIcon], 0L, LONG_MAX, False, AnyPropertyType, &real, &format, &n,
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
 * 隐藏窗口
 */
void hide(const Arg *arg)
{
    hidewin(select_monitor->select);
    focus(NULL);
    arrange(select_monitor);
}

/**
 * 隐藏客户端
 */
void hidewin(Client *c)
{
    if (!c || HIDDEN(c))
        return;

    Window                   w = c->win;
    static XWindowAttributes ra, ca;

    // more or less taken directly from blackbox's hide() function
    XGrabServer(dpy);
    XGetWindowAttributes(dpy, root, &ra);
    XGetWindowAttributes(dpy, w, &ca);
    // prevent UnmapNotify events
    XSelectInput(dpy, root, ra.your_event_mask & ~SubstructureNotifyMask);
    XSelectInput(dpy, w, ca.your_event_mask & ~StructureNotifyMask);
    XUnmapWindow(dpy, w);
    setclientstate(c, IconicState);
    XSelectInput(dpy, root, ra.your_event_mask);
    XSelectInput(dpy, w, ca.your_event_mask);
    XUngrabServer(dpy);
}