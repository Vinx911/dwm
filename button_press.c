
#include "dwm.h"

/**
 * 鼠标按下
 */
void button_press(XEvent *e)
{
    unsigned int         click = ClkRootWin;
    Arg                  arg   = {0};
    XButtonPressedEvent *ev    = &e->xbutton;

    /* focus monitor if necessary */
    Monitor *monitor = window_to_monitor(ev->window);
    if (monitor != NULL && monitor != select_monitor) {
        unfocus(select_monitor->select, 1);
        select_monitor = monitor;
        focus(NULL);
    }

    if (ev->window == select_monitor->bar_window) {
        unsigned int click_tag = 0;  // 点击的tag
        unsigned int x         = 0;

        if (select_monitor->is_overview) {
            x += TEXTW(overviewtag);
            click_tag = ~0;
            if (ev->x > x) {
                click_tag = TAGS_COUNT;
            }
        } else {
            unsigned int occ = 0;
            for (Client *client = monitor->clients; client; client = client->next) {
                occ |= client->tags == 255 ? 0 : client->tags;
            }

            do {
                /* do not reserve space for vacant tags */
                if (!(occ & 1 << click_tag || monitor->tagset[monitor->seltags] & 1 << click_tag)) {
                    continue;
                }
                x += TEXTW(tags[click_tag]);
            } while (ev->x >= x && ++click_tag < TAGS_COUNT);
        }

        // 布局符号起始x坐标
        int lt_symbol_x = tag_bar_width + lt_symbol_width;

        // 系统托盘宽度
        int systray_width = get_systray_width();
        if (select_monitor == systray_to_monitor(select_monitor)) {
            if (systray_width != 0) {
                systray_width = systray_width + systraypinning + 2;
            }
        }

        // 状态栏宽度
        int status_bar_width = draw_status_bar(select_monitor, status_text);
        // 状态栏起始x坐标
        int status_bar_x = select_monitor->ww - status_bar_width - 2 * sp - systray_width;

        if (click_tag < TAGS_COUNT) {
            click  = ClkTagBar;
            arg.ui = 1 << click_tag;
        } else if (ev->x < lt_symbol_x) {
            click = ClkLtSymbol;
        } else if (ev->x > status_bar_x) {
            click  = ClkStatusText;
            arg.i  = ev->x - status_bar_x;
            arg.ui = ev->button;  // 1 => L，2 => M，3 => R, 5 => U, 6 => D
        } else {
            x += lt_symbol_width;
            Client *client = monitor->clients;

            if (monitor->task_count == 0)
                return;

            if (client) {
                do {
                    if (ISVISIBLE(client)) {
                        x += (1.0 / (double)monitor->task_count) * monitor->task_bar_width;
                    }
                } while (ev->x > x && (client = client->next));

                click = ClkWinTitle;
                arg.v = client;
            }
        }
    } else {
        Client *client = window_to_client(ev->window);
        if (client != NULL) {
            focus(client);
            restack(select_monitor);
            XAllowEvents(dpy, ReplayPointer, CurrentTime);
            click = ClkClientWin;
        }
    }

    for (int i = 0; i < buttons_count(); i++) {
        if (click == buttons[i].click && buttons[i].func && buttons[i].button == ev->button
            && CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state))
        {
            if ((click == ClkTagBar || click == ClkWinTitle || click == ClkStatusText) && buttons[i].arg.i == 0) {
                buttons[i].func(&arg);
            } else {
                buttons[i].func(&buttons[i].arg);
            }
        }
    }
}






















