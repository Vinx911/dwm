#ifndef LAYOUT_H
#define LAYOUT_H

/**
 * 排列窗口
 */
void arrange(Monitor *m);

/**
 * 排列尺寸计算
 */
void arrangemon(Monitor *m);


void magicgrid(Monitor *m);

void overview(Monitor *m);

void grid(Monitor *m, uint gappx);


/**
 * 单窗口布局
 */
void monocle(Monitor *m);
/**
 * 切换monocle显示窗口个数
 */
void togglemonocle(const Arg *arg);


/**
 * 平铺布局
 */
void tile(Monitor *m);




/**
 * 设置布局
 */
void setlayout(const Arg *arg);

/**
 * 重新堆叠窗口
 */
void restack(Monitor *m);

#endif  // LAYOUT_H