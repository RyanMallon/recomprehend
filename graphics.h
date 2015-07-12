/*
 * This file is public domain
 *
 * Ryan Mallon, 2015
 *
 */

#ifndef _RECOMPREHEND_GRAPHICS_H
#define _RECOMPREHEND_GRAPHICS_H

#include <stdbool.h>
#include <stdint.h>

#define G_RENDER_WIDTH	320
#define G_RENDER_HEIGHT	240

#define RGB(r, g, b)		(((r) << 24) | ((g) << 16) | ((b) << 8) | 0xff)

#define G_COLOR_BLACK		0x000000ff
#define G_COLOR_WHITE		0xffffffff
#define G_COLOR_CYAN		0x3366ffff
#define G_COLOR_YELLOW		0xffff00ff
#define G_COLOR_RED		0xff0000ff

#define G_COLOR_GRAY0		0x202020ff
#define G_COLOR_GRAY1		0x404040ff
#define G_COLOR_GRAY2		0x808080ff
#define G_COLOR_GRAY3		0xc0c0c0ff

#define G_COLOR_LIGHT_ORANGE	0xff9966ff
#define G_COLOR_ORANGE		0xff9900ff
#define G_COLOR_DARK_PURPLE	0x666699ff
#define G_COLOR_DARK_BLUE	0x000099ff

#define G_COLOR_DARK_RED	0xcc0033ff
#define G_COLOR_DITHERED_PINK	0xff6699ff

#define G_COLOR_DARK_GREEN1	0x009966ff
#define G_COLOR_DARK_GREEN2	0x003300ff

#define G_COLOR_AQUA		0x33ccccff

#define G_COLOR_GREEN		0x33cc00ff

#define G_COLOR_BROWN1		0x7a5200ff
#define G_COLOR_BROWN2		0x663300ff

void g_set_color_table(unsigned index);

unsigned g_set_fill_color(uint8_t index);
unsigned g_set_pen_color(uint8_t opcode);

unsigned g_get_pixel_color(int x, int y);

void g_draw_pixel(unsigned x, unsigned y, unsigned color);
void g_draw_line(unsigned x1, unsigned y1, unsigned x2, unsigned y2,
		 unsigned color);
void g_draw_box(unsigned x1, unsigned y1, unsigned x2, unsigned y2,
		unsigned color);
void g_draw_shape(int x, int y, int shape_type, unsigned fill_color);
void g_floodfill(int x, int y, unsigned fill_color, unsigned old_color);

void g_clear_screen(unsigned color);
void g_flip_buffers(void);
void g_init(unsigned width, unsigned height);
bool g_enabled(void);
       
#endif /* _RECOMPREHEND_GRAPHICS_H */
