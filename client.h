#ifndef CLIENT_H
#define CLIENT_H

#define ISVISIBLE(C) (((C->mon->is_overview && !C->nooverview) || C->tags & C->mon->tagset[C->mon->seltags]))
#define HIDDEN(C) ((window_get_state(C->win) == IconicState))
#define WIDTH(X) ((X)->w + 2 * (X)->bw)
#define HEIGHT(X) ((X)->h + 2 * (X)->bw)

/**
 * 客户端附加到列表中
 */
void client_attach(Client *c);

/**
 * 客户端从列表中分离
 */
void client_detach(Client *c);

/**
 * 客户端附加到栈中
 */
void client_attach_stack(Client *c);

/**
 * 客户端从栈中分离
 */
void client_detach_stack(Client *c);

/**
 * 客户端入队
 */
void client_enqueue(Client *c);

/**
 * 客户端栈中入队
 */
void client_enqueue_stack(Client *c);

/**
 * 客户端提升到顶部
 */
void client_pop(Client *c);

/**
 * 更新客户端列表
 */
void client_update_list();

/**
 * 重新堆叠窗口
 */
void client_restack(Monitor *m);

void client_pointer_focus_win(Client *c);

/**
 * 重设客户端尺寸
 */
void client_resize(Client *c, int x, int y, int w, int h, int interact);

/**
 * 重设客户端尺寸
 */
void client_resize_client(Client *c, int x, int y, int w, int h);

/**
 * 应用客户端尺寸hints
 */
int client_apply_size_hints(Client *c, int *x, int *y, int *w, int *h, int interact);

/**
 * 更新wm hint
 */
void client_update_wm_hints(Client *c);

/**
 * 更新客户端尺寸提示
 */
void client_update_size_hints(Client *c);

/**
 * 更新窗口标题
 */
void client_update_title(Client *c);

/**
 * 更新窗口图标
 */
void client_update_icon(Client *c);

/**
 * 释放窗口图标
 */
void client_free_icon(Client *c);

/**
 * 更新窗口类型
 */
void client_update_window_type(Client *c);

/**
 * 显示和隐藏窗口列表
 */
void client_show_hide(Client *c);

/**
 * 设置客户端全屏
 */
void client_set_full_screen(Client *c, int full_screen);

/**
 * 配置客户端
 */
void client_configure(Client *c);

/**
 * 设置紧急性
 */
void client_seturgent(Client *c, int urg);

/**
 * 设置客户端状态
 */
void client_set_state(Client *c, long state);

/**
 * 获取atom属性
 */
Atom client_get_atom_prop(Client *c, Atom prop);

/**
 * 注册鼠标按键
 */
void client_grab_buttons(Client *c, int focused);

/**
 * 设置客户端焦点
 */
void client_set_focus(Client *c);

/**
 * 客户端焦点
 */
void client_focus(Client *c);

/**
 * 取消客户端焦点
 */
void client_unfocus(Client *c, int client_set_focus);

/**
 * 管理窗口
 */
void client_manage(Window w, XWindowAttributes *wa);

/**
 * 不再管理窗口
 */
void client_unmanage(Client *c, int destroyed);

/**
 * 将客户端发送到监视器
 */
void client_send_to_monitor(Client *c, Monitor *m);

/**
 * 下一个平铺的客户端
 */
Client *client_next_tiled(Client *c);

/**
 * 显示客户端
 */
void show_client(const Arg *arg);

/**
 * 显示全部客户端
 */
void show_all_client(const Arg *arg);

/**
 * 杀死客户端
 */
void kill_client(const Arg *arg);

/**
 * 强制关闭窗口(处理某些情况下无法销毁的窗口)
 */
void force_kill_client(const Arg *arg);

/**
 * 旋转窗口栈
 */
void rotate_client_stack(const Arg *arg);

/**
 * 鼠标调整窗口大小
 */
void resize_by_mouse(const Arg *arg);

/**
 * 鼠标移动窗口
 */
void move_by_mouse(const Arg *arg);

/**
 * 将当前聚焦窗口置为主窗口
 */
void zoom(const Arg *arg);

/**
 * 切换窗口焦点
 */
void focus_stack(int inc, int vis);

/**
 * 切换显示窗口焦点
 */
void focusstackvis(const Arg *arg);

/**
 * 切换隐藏窗口焦点
 */
void focusstackhid(const Arg *arg);

/**
 * 切换窗口显示状态
 */
void toggle_window(const Arg *arg);

/**
 * 切换当前客户端浮动
 */
void toggle_floating(const Arg *arg);

/**
 * 切换全部客户端浮动
 */
void toggle_all_floating(const Arg *arg);

/**
 * 切换全屏
 */
void toggle_full_screen(const Arg *arg);

/**
 * 切换伪全屏
 */
void toggle_fake_full_screen(const Arg *arg);

/**
 * 隐藏其他窗口仅保留该窗口
 */
void hide_other_wins(const Arg *arg);

/**
 * 切换 只显示一个窗口 / 全部显示
 */
void show_only_or_all(const Arg *arg);

/**
 * 移动窗口
 */
void move_window(const Arg *arg);

/**
 * 调整窗口
 */
void resize_window(const Arg *arg);

#endif  // CLIENT_H