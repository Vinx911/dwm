#ifndef BAR_H
#define BAR_H

/**
 * 更新Bar
 */
void bar_update_bars(void);

/**
 * 更新Bar位置
 */
void bar_update_pos(Monitor *m);

/**
 * 绘制Bar
 */
void bar_draw_bar(Monitor *m);

/**
 * 绘制Bar
 */
void bar_draw_bars(void);

/**
 * 切换bar显示状态
 */
void toggle_bar(const Arg *arg);

#endif  // BAR_H