#ifndef CLIENT_H
#define CLIENT_H


/**
 * 客户端附加到列表中
 */
void attach(Client *c);

/**
 * 客户端附加到栈中
 */
void attachstack(Client *c);

/**
 * 客户端从列表中分离
 */
void detach(Client *c);

/**
 * 客户端从栈中分离
 */
void detachstack(Client *c);

/**
 * 客户端入队
 */
void enqueue(Client *c);

/**
 * 客户端栈中入队
 */
void enqueuestack(Client *c);


/**
 * 客户端提升到顶部
 */
void pop(Client *c);

/**
 * 杀死客户端
 */
void killclient(const Arg *arg);

/**
 * 重设客户端尺寸
 */
void resize(Client *c, int x, int y, int w, int h, int interact);

/**
 * 重设客户端尺寸
 */
void resizeclient(Client *c, int x, int y, int w, int h);

/**
 * 设置客户端全屏
 */
void setfullscreen(Client *c, int fullscreen);

/**
 * 显示客户端
 */
void show(const Arg *arg);

/**
 * 显示全部客户端
 */
void showall(const Arg *arg);

/**
 * 显示窗口
 */
void showwin(Client *c);

/**
 * 显示和隐藏窗口列表
 */
void showhide(Client *c);



/**
 * 更新客户端列表
 */
void updateclientlist();


/**
 * 更新客户端尺寸提示
 */
void updatesizehints(Client *c);

/**
 * 更新窗口标题
 */
void updatetitle(Client *c);

/**
 * 更新窗口图标
 */
void updateicon(Client *c);



/**
 * 释放窗口图标
 */
void freeicon(Client *c);

/**
 * 更新窗口类型
 */
void updatewindowtype(Client *c);

/**
 * 更新wm hint
 */
void updatewmhints(Client *c);


/**
 * 切换窗口显示状态
 */
void togglewin(const Arg *arg);


/**
 * 切换当前客户端浮动
 */
void togglefloating(const Arg *arg);


/**
 * 全屏
 */
void    fullscreen(const Arg *arg);

void togglefakefullscreen(const Arg *arg);



/**
 * 设置客户端焦点
 */
void setfocus(Client *c);


/**
 * 将客户端发送到监视器
 */
void sendmon(Client *c, Monitor *m);

/**
 * 设置客户端状态
 */
void setclientstate(Client *c, long state);


/**
 * 旋转窗口栈
 */
void rotatestack(const Arg *arg);


/**
 * 鼠标调整窗口大小
 */
void resizemouse(const Arg *arg);


/**
 * 下一个平铺的客户端
 */
Client *nexttiled(Client *c);


/**
 * 鼠标移动窗口
 */
void movemouse(const Arg *arg);




Atom getatomprop(Client *c, Atom prop);


/**
 * 配置客户端
 */
void configure(Client *c);


/**
 * 应用客户端尺寸hints
 */
int applysizehints(Client *c, int *x, int *y, int *w, int *h, int interact);

#endif  // CLIENT_H