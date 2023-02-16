
#include "dwm.h"
#include "bar.h"
#include "systray.h"
#include "window.h"
#include "config.h"

/**
 * 绘制状态栏
 */
int draw_status_bar(Monitor *monitor, char *stext)
{
    int   width  = 0, i, w, x, len;
    short isCode = 0;
    char *text;
    char *p;
    char  buf8[8], buf5[5];
    uint  textsalpha;

    // 系统托盘宽度
    int systray_width = 0;
    if (show_systray && monitor == systray_to_monitor(monitor)) {
        systray_width = get_systray_width();
    }

    len = strlen(stext) + 1;
    if (!(text = (char *)malloc(sizeof(char) * len))) {
        die("malloc");
    }
    p = text;
    memcpy(text, stext, len);

    // 计算状态文本的宽度
    w = 0;
    i = -1;
    while (text[++i]) {
        if (text[i] == '^') {
            if (!isCode) {
                isCode  = 1;
                text[i] = '\0';
                w += TEXTW(text) - text_lr_pad;
                text[i] = '^';
                if (text[++i] == 'f') {
                    w += atoi(text + ++i);
                }
            } else {
                isCode = 0;
                text   = text + i + 1;
                i      = -1;
            }
        }
    }

    if (!isCode) {
        w += TEXTW(text) - text_lr_pad;
    } else {
        isCode = 0;
    }

    text = p;

    x = monitor->ww - w - systray_width - 2 * sp
      - (systray_width ? systrayspadding : 0);  // 托盘存在时 额外多-一个systrayspadding

    drw_setscheme(drw, scheme[colors_count()]);
    drw->scheme[ColFg] = scheme[SchemeNorm][ColFg];
    drw->scheme[ColBg] = scheme[SchemeNorm][ColBg];
    drw_rect(drw, x, 0, w, bar_height, 1, 1);
    x++;

    /* process status text */
    i = -1;
    while (text[++i]) {
        if (text[i] == '^' && !isCode) {
            isCode = 1;

            text[i] = '\0';
            w       = TEXTW(text) - text_lr_pad;
            drw_text(drw, x, 0, w, bar_height, 0, text, 0);
            width += w;

            x += w;

            /* process code */
            while (text[++i] != '^') {
                if (text[i] == 'c') {
                    memcpy(buf8, (char *)text + i + 1, 7);
                    buf8[7] = '\0';
                    i += 7;

                    textsalpha = alphas[SchemeStatusText][ColFg];
                    if (text[i + 1] != '^') {
                        memcpy(buf5, (char *)text + i + 1, 4);
                        buf5[4] = '\0';
                        i += 4;
                        sscanf(buf5, "%x", &textsalpha);
                    }
                    drw_clr_create(drw, &drw->scheme[ColFg], buf8, textsalpha);
                } else if (text[i] == 'b') {
                    memcpy(buf8, (char *)text + i + 1, 7);
                    buf8[7] = '\0';
                    i += 7;

                    textsalpha = alphas[SchemeStatusText][ColBg];
                    if (text[i + 1] != '^') {
                        memcpy(buf5, (char *)text + i + 1, 4);
                        buf5[4] = '\0';
                        i += 4;
                        sscanf(buf5, "%x", &textsalpha);
                    }
                    drw_clr_create(drw, &drw->scheme[ColBg], buf8, textsalpha);
                } else if (text[i] == 's') {
                    while (text[i + 1] != '^')
                        i++;
                } else if (text[i] == 'd') {
                    drw->scheme[ColFg] = scheme[SchemeNorm][ColFg];
                    drw->scheme[ColBg] = scheme[SchemeNorm][ColBg];
                } else if (text[i] == 'r') {
                    int rx = atoi(text + ++i);
                    while (text[++i] != ',')
                        ;
                    int ry = atoi(text + ++i);
                    while (text[++i] != ',')
                        ;
                    int rw = atoi(text + ++i);
                    while (text[++i] != ',')
                        ;
                    int rh = atoi(text + ++i);

                    drw_rect(drw, rx + x, ry, rw, rh, 1, 0);
                } else if (text[i] == 'f') {
                    x += atoi(text + ++i);
                }
            }

            text   = text + i + 1;
            i      = -1;
            isCode = 0;
        }
    }

    if (!isCode) {
        w = TEXTW(text) - text_lr_pad;
        drw_text(drw, x, 0, w, bar_height, 0, text, 0);
        width += w;
    }

    drw_setscheme(drw, scheme[SchemeNorm]);
    free(p);

    return width - 2;
}

/**
 * 点击状态栏时执行的func
 * 传入参数为 i  => 鼠标点击的位置相对于左边界的距离
 * 传入参数为 ui => 鼠标按键, 1 => 左键, 2 => 中键, 3 => 右键, 4 => 滚轮向上, 5=> 滚轮向下
 */
void click_status_bar(const Arg *arg)
{
    // 用于避免过于频繁的点击和滚轮行为
    static long lastclickstatusbartime = 0;
    if (!arg->i && arg->i < 0)
        return;

    // 用于避免过于频繁的点击和滚轮行为
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long now = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    if (now - lastclickstatusbartime < 100)
        return;
    lastclickstatusbartime = now;

    int   offset   = -1;
    int   status_w = 0;
    int   iscode = 0, issignal = 0, signalindex = 0;
    char  signal[20];
    char  text[100];
    char *button = "L";
    int   limit, max = sizeof(status_text);

    while (status_text[++offset] != '\0') {
        // 左侧^
        if (status_text[offset] == '^' && !iscode) {
            iscode = 1;
            offset++;
            if (status_text[offset] == 's') {  // 查询到s->signal
                issignal    = 1;
                signalindex = 0;
                memset(signal, '\0', sizeof(signal));
            } else {
                issignal = 0;
            }
            continue;
        }

        // 右侧^
        if (status_text[offset] == '^' && iscode) {
            iscode   = 0;
            issignal = 0;
            continue;
        }

        if (issignal) {  // 逐位读取signal
            signal[signalindex++] = status_text[offset];
        }

        // 是普通文本
        if (!iscode) {
            // 查找到下一个^ 或 游标到达最后
            limit = 0;
            while (status_text[offset + ++limit] != '^' && offset + limit < max)
                ;
            if (offset + limit == max)
                break;

            memset(text, '\0', sizeof(text));
            strncpy(text, status_text + offset, limit);
            offset += --limit;
            status_w += TEXTW(text) - text_lr_pad;
            if (status_w > arg->i)
                break;
        }
    }

    switch (arg->ui) {
    case Button1:
        button = "L";
        break;
    case Button2:
        button = "M";
        break;
    case Button3:
        button = "R";
        break;
    case Button4:
        button = "U";
        break;
    case Button5:
        button = "D";
        break;
    }

    memset(text, '\0', sizeof(text));
    sprintf(text, "%s %s %s &", statusbarscript, signal, button);
    system(text);
}


/**
 * 更新状态栏
 */
void update_status(void)
{
    Monitor *m;
    if (!gettextprop(root, XA_WM_NAME, status_text, sizeof(status_text))) {
        strcpy(status_text, "dwm-" VERSION);
    }

    for (m = mons; m; m = m->next) {
        drawbar(m);
    }
}
