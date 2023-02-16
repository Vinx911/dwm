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
 * 获取窗口状态
 */
long getstate(Window w);

/**
 * 获取文本属性
 */
int gettextprop(Window w, Atom atom, char *text, unsigned int size);



/**
 * 获取图标
 */
Picture geticonprop(Window win, unsigned int *picw, unsigned int *pich);


/**
 * 隐藏窗口
 */
void hide(const Arg *arg);

/**
 * 隐藏客户端
 */
void hidewin(Client *c);


#endif  // WINDOW_H