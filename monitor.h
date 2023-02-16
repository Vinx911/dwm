#ifndef MONITOR_H
#define MONITOR_H


/**
 * 当前客户端切换监视器
 */
void tagmon(const Arg *arg);



/**
 * 矩形所在的监视器
 */
Monitor *recttomon(int x, int y, int w, int h);



/**
 * 创建一个监视器
 */
Monitor *createmon(void);

/**
 * 指定方向的监视器
 */
Monitor *dirtomon(int dir);

/**
 * 清除释放监视器
 */
void cleanup_monitor(Monitor *mon);


#endif  // MONITOR_H