#ifndef STATUS_BAR_H
#define STATUS_BAR_H

/**
 * 绘制状态栏
 */
int  status_bar_draw(Monitor *m, char *text);

/**
 * 更新状态栏
 */
void status_bar_update_status(void);

/**
 * 点击状态栏时执行的func
 * 传入参数为 i  => 鼠标点击的位置相对于左边界的距离
 * 传入参数为 ui => 鼠标按键, 1 => 左键, 2 => 中键, 3 => 右键, 4 => 滚轮向上, 5=> 滚轮向下
 */
void click_status_bar(const Arg *arg);

#endif  // STATUS_BAR_H