#ifndef WINDOW_H
#define WINDOW_H

/**
 * 窗口所属的客户端
 */
Client *window_to_client(Window w);

/**
 * 窗口所在的监视器
 */
Monitor *window_to_monitor(Window w);

/**
 * 窗口的系统托盘图标
 */
Client *window_to_systray_icon(Window w);

/**
 * 获取窗口状态
 */
long window_get_state(Window w);

/**
 * 获取文本属性
 */
int window_get_text_prop(Window w, Atom atom, char *text, unsigned int size);

/**
 * 获取图标
 */
Picture window_get_icon_prop(Window win, unsigned int *picw, unsigned int *pich);

/**
 * 获取根点
 */
int window_get_root_ptr(int *x, int *y);

/**
 * 隐藏客户端
 */
void window_hide(Client *c);

/**
 * 发送事件
 */
int window_send_event(Window w, Atom proto, int m, long d0, long d1, long d2, long d3, long d4);

/**
 * 隐藏窗口
 */
void hide_window(const Arg *arg);


#endif  // WINDOW_H