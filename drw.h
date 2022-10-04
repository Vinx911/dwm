/* See LICENSE file for copyright and license details. */
typedef struct {
    Cursor cursor;
} Cur;

typedef struct Fnt {
    Display* dpy; //所属的显示器
    unsigned int h; //字体高度（字号大小）
    XftFont* xfont; //指向X库中的xftFont结构体，是字体实现的核心
    FcPattern* pattern; //模式
    struct Fnt* next; //指向下一个字体
} Fnt;

enum {
    ColFg,
    ColBg,
    ColBorder,
}; /* Clr scheme index */
typedef XftColor Clr;

/**
 * 可绘制视窗的管理结构体
 */
typedef struct {
    unsigned int w, h; //宽，高
    Display* dpy; //所属显示器
    int screen; //屏幕号
    Window root; //根视窗
    Visual* visual; // visual
    unsigned int depth; // 颜色位深
    Colormap cmap; // 颜色映射
    Drawable drawable;
    Picture picture; // 窗口图标
    GC gc; // 图形上下文,存储前景色、背景色、线条样式等
    Clr* scheme; //配色
    Fnt* fonts; //字体
} Drw;

/* Drawable abstraction */
Drw* drw_create(Display* dpy, int screen, Window win, unsigned int w, unsigned int h, Visual* visual, unsigned int depth, Colormap cmap);
void drw_resize(Drw* drw, unsigned int w, unsigned int h);
void drw_free(Drw* drw);

/* Fnt abstraction */
Fnt* drw_fontset_create(Drw* drw, const char* fonts[], size_t fontcount);
void drw_fontset_free(Fnt* set);
unsigned int drw_fontset_getwidth(Drw* drw, const char* text);
unsigned int drw_fontset_getwidth_clamp(Drw* drw, const char* text, unsigned int n);
void drw_font_getexts(Fnt* font, const char* text, unsigned int len, unsigned int* w, unsigned int* h);

/* Colorscheme abstraction */
void drw_clr_create(Drw* drw, Clr* dest, const char* clrname, unsigned int alpha);
Clr* drw_scm_create(Drw* drw, const char* clrnames[], const unsigned int alphas[], size_t clrcount);

/* Cursor abstraction */
Cur* drw_cur_create(Drw* drw, int shape);
void drw_cur_free(Drw* drw, Cur* cursor);

/* Drawing context manipulation */
void drw_setfontset(Drw* drw, Fnt* set);
void drw_setscheme(Drw* drw, Clr* scm);

Picture drw_picture_create_resized(Drw* drw, char* src, unsigned int src_w, unsigned int src_h, unsigned int dst_w, unsigned int dst_h);

/* Drawing functions */
void drw_rect(Drw* drw, int x, int y, unsigned int w, unsigned int h, int filled, int invert);
int drw_text(Drw* drw, int x, int y, unsigned int w, unsigned int h, unsigned int lpad, const char* text, int invert);
void drw_pic(Drw* drw, int x, int y, unsigned int w, unsigned int h, Picture pic);

/* Map functions */
void drw_map(Drw* drw, Window win, int x, int y, unsigned int w, unsigned int h);
