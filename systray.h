#ifndef SYSTRAY_H
#define SYSTRAY_H

extern Systray      *systray;
extern unsigned long systrayorientation;

/**
 * 获取系统托盘宽度
 */
unsigned int get_systray_width();

/**
 * 更新系统托盘
 *
 * @param updatebar 是否更新bar
 */
void update_systray(int updatebar);

/**
 * 更新系统托盘图标尺寸
 *
 * @param i 托盘图标
 * @param w 宽度
 * @param h 高度
 */
void update_systray_icon_geom(Client *i, int w, int h);

/**
 * 更新系统托盘图标状态
 *
 * @param i 托盘图标
 * @param ev 事件
 */
void update_systray_icon_state(Client *i, XPropertyEvent *ev);

/**
 * 移除系统托盘图标
 */
void remove_systray_icon(Client *i);

/**
 * 系统托盘所在监视器
 */
Monitor *systray_to_monitor(Monitor *m);

/**
 * 窗口的系统托盘图标
 */
Client *window_to_systray_icon(Window w);

/**
 * 切换系统托盘显示
 */
void toggle_systray(const Arg *arg);

/**
 * 清除系统托盘资源
 */
void cleanup_systray();

/**
 * 系统托盘客户端消息
 */
void systray_client_message(XEvent *e);

#endif  // SYSTRAY_H