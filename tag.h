#ifndef TAG_H
#define TAG_H

/**
 * 显示tag
 */
void view_tag(const Arg *arg);

/**
 * 切换当前客户端tag
 */
void move_to_tag(const Arg *arg);

/**
 * 切换当前客户端所属tag, 可以添加一个新的tag
 */
void append_to_tag(const Arg *arg);

/**
 * 切换tag显示状态
 */
void toggle_tag_view(const Arg *arg);

/**
 * 显示所有tag 或 跳转到聚焦窗口的tag
 */
void toggle_overview(const Arg *arg);

#endif  // TAG_H