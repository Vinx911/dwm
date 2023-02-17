#ifndef LAYOUT_H
#define LAYOUT_H

/**
 * 排列窗口
 */
void layout_arrange(Monitor *m);

/**
 * 排列监视器
 */
void layout_arrange_monitor(Monitor *m);

/**
 * 网格布局
 */
void grid(Monitor *m, uint gappx);

/**
 * 网格布局
 */
void magic_grid(Monitor *m);

/**
 * 任务概览
 */
void overview(Monitor *m);

/**
 * 单窗口布局
 */
void monocle(Monitor *m);

/**
 * 平铺布局
 */
void tile(Monitor *m);

/**
 * 设置布局
 */
void set_layout(const Arg *arg);

/**
 * 切换monocle显示窗口个数
 */
void toggle_monocle(const Arg *arg);

/**
 * 增加/减少主窗口个数
 */
void inc_nmaster(const Arg *arg);

/**
 * 设置窗口间距
 */
void set_gaps(const Arg *arg);

/**
 * 设置主窗口尺寸因子
 */
void set_mfact(const Arg *arg);

#endif  // LAYOUT_H