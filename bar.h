#ifndef BAR_H
#define BAR_H

/**
 * 更新Bar
 */
void updatebars(void);

/**
 * 更新Bar位置
 */
void updatebarpos(Monitor *m);

/**
 * 更新Bar尺寸
 *
 * @return 是否需要重新绘制
 */
int updategeom(void);

/**
 * 切换bar显示状态
 */
void togglebar(const Arg *arg);

/**
 * 绘制Bar
 */
void drawbar(Monitor *m);

/**
 * 绘制Bar
 */
void drawbars(void);

#endif  // BAR_H