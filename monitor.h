#ifndef MONITOR_H
#define MONITOR_H


/**
 * 创建一个监视器
 */
Monitor *monitor_create(void);

/**
 * 更新Bar尺寸
 *
 * @return 是否需要重新绘制
 */
int monitor_update_geometries(void);

/**
 * 矩形所在的监视器
 */
Monitor *monitor_rect_to_monitor(int x, int y, int w, int h);

/**
 * 指定方向的监视器
 */
Monitor *monitor_dir_to_monitor(int dir);

/**
 * 清除释放监视器
 */
void monitor_cleanup(Monitor *mon);

/**
 * 当前客户端切换监视器
 */
void move_to_monitor(const Arg *arg);

/**
 * 光标移动到上一个显示器
 */
void focus_monitor(const Arg *arg);

#endif  // MONITOR_H