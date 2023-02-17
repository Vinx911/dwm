#ifndef SYSTRAY_H
#define SYSTRAY_H

#define SYSTEM_TRAY_REQUEST_DOCK 0
#define _NET_SYSTEM_TRAY_ORIENTATION_HORZ 0

/* XEMBED messages */
#define XEMBED_EMBEDDED_NOTIFY 0
#define XEMBED_WINDOW_ACTIVATE 1
#define XEMBED_FOCUS_IN 4
#define XEMBED_MODALITY_ON 10

#define XEMBED_MAPPED (1 << 0)
#define XEMBED_WINDOW_ACTIVATE 1
#define XEMBED_WINDOW_DEACTIVATE 2

#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define XEMBED_EMBEDDED_VERSION (VERSION_MAJOR << 16) | VERSION_MINOR

extern Systray      *systray;
extern unsigned long systrayorientation;

/**
 * 获取系统托盘宽度
 */
unsigned int systray_get_width();

/**
 * 更新系统托盘
 *
 * @param updatebar 是否更新bar
 */
void systray_update(int updatebar);

/**
 * 更新系统托盘图标尺寸
 *
 * @param i 托盘图标
 * @param w 宽度
 * @param h 高度
 */
void systray_update_icon_geom(Client *i, int w, int h);

/**
 * 更新系统托盘图标状态
 *
 * @param i 托盘图标
 * @param ev 事件
 */
void systray_update_icon_state(Client *i, XPropertyEvent *ev);

/**
 * 移除系统托盘图标
 */
void systray_remove_icon(Client *i);

/**
 * 系统托盘所在监视器
 */
Monitor *systray_to_monitor(Monitor *m);

/**
 * 清除系统托盘资源
 */
void systray_cleanup();

/**
 * 系统托盘客户端消息
 */
void systray_client_message(XEvent *e);

/**
 * 切换系统托盘显示
 */
void toggle_systray(const Arg *arg);

#endif  // SYSTRAY_H