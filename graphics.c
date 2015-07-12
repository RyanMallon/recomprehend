/*
 * This file is public domain
 *
 * Ryan Mallon, 2015
 *
 */

#include <stdbool.h>
#include <endian.h>
#include <stdint.h>
#include <stdio.h>

#include <SDL2/SDL.h>

#include "image_data.h"
#include "graphics.h"
#include "util.h"

#define RENDER_X_MAX		278
#define RENDER_Y_MAX		162

#define RENDERER_SCREEN		0
#define RENDERER_PIXEL_DATA	1

static bool graphics_enabled;

static unsigned pen_colors[] = {
	[0x00] = G_COLOR_BLACK,
	[0x01] = RGB(0x00, 0x66, 0x00),
	[0x02] = RGB(0x00, 0xff, 0x00),
	[0x03] = G_COLOR_WHITE,
	[0x04] = G_COLOR_BLACK,
	[0x05] = RGB(0x00, 0xff, 0xff),
	[0x06] = RGB(0xff, 0x00, 0xff),
	[0x07] = RGB(0xff, 0x00, 0x00),
};

struct graphics_context {
	SDL_Window	*screen;

	/*
	 * FIXME - Currently using two renderers. One for drawing the (possibly
	 *         scaled) image to the screen and the other for getting pixel
	 *         data for floodfill boundaries. This is almost certainly not
	 *         the best way to do this.
	 */
	SDL_Renderer	*renderer[2];

	/* Used for pixel access for flood fills */
	SDL_Surface	*surface;
};

static struct graphics_context ctx;

unsigned g_set_pen_color(uint8_t opcode)
{
	return pen_colors[opcode - IMAGE_OP_PEN_COLOR_A];
}

/* Used by Transylvania and Crimson Crown */
static unsigned default_color_table[] = {
	[0x00] = G_COLOR_WHITE,
	[0x01] = G_COLOR_DARK_BLUE,
	[0x02] = G_COLOR_GRAY1,
	[0x03] = G_COLOR_DARK_RED,
	[0x04] = G_COLOR_GRAY2,
	[0x06] = G_COLOR_GRAY3,
	[0x0d] = G_COLOR_BROWN1,
	[0x0e] = G_COLOR_DARK_PURPLE,
	[0x12] = G_COLOR_DARK_RED,
	[0x13] = G_COLOR_BROWN2,
	[0x17] = G_COLOR_DARK_BLUE,
	[0x18] = G_COLOR_BLACK,
	[0x1f] = G_COLOR_DARK_PURPLE,
	[0x20] = G_COLOR_DARK_PURPLE,
	[0x22] = G_COLOR_DARK_RED,
	[0x2b] = G_COLOR_DARK_PURPLE,
	[0x34] = G_COLOR_WHITE,
	[0x35] = G_COLOR_GRAY0,
	[0x36] = RGB(0xb5, 0x6c, 0x47),
	[0x3c] = G_COLOR_CYAN,
	[0x3d] = G_COLOR_DARK_RED,
	[0x3e] = G_COLOR_DARK_GREEN1,
	[0x3f] = G_COLOR_DARK_GREEN2,
	[0x40] = G_COLOR_DARK_PURPLE,
	[0x42] = G_COLOR_DITHERED_PINK,
	[0x45] = G_COLOR_BROWN2,
	[0x46] = G_COLOR_DARK_RED,
	[0x47] = G_COLOR_DARK_BLUE,
	[0x48] = G_COLOR_DARK_BLUE,
	[0x49] = G_COLOR_DARK_BLUE,
	[0x4d] = G_COLOR_WHITE,
	[0x4e] = G_COLOR_BROWN2,
	[0x4f] = G_COLOR_BROWN2,
	[0x50] = G_COLOR_BLACK,
	[0x51] = G_COLOR_DARK_PURPLE,
	[0x53] = G_COLOR_GRAY2,
	[0x54] = G_COLOR_BROWN2,
	[0x57] = G_COLOR_AQUA,
	[0x5a] = G_COLOR_GREEN,
	[0x5b] = G_COLOR_DARK_BLUE,
	[0x5c] = G_COLOR_DARK_PURPLE,
	[0x5d] = G_COLOR_BROWN1,
	[0x5e] = G_COLOR_BROWN2,
	[0x60] = G_COLOR_DARK_PURPLE,
	[0x61] = G_COLOR_LIGHT_ORANGE,
	[0x64] = G_COLOR_ORANGE,
	[0x65] = G_COLOR_RED,
	[0x66] = G_COLOR_DARK_RED,
	[0x6a] = G_COLOR_DARK_BLUE,
	[0x6b] = G_COLOR_DARK_PURPLE,
	[0x80] = G_COLOR_BLACK,

	[0xff] = 0,
};

/* Used by OO-topos */
/* FIXME - incomplete */
static unsigned color_table_1[] = {
	[0x35] = RGB(0x80, 0x00, 0x00),
	[0x37] = RGB(0xe6, 0xe6, 0x00),
	[0x3c] = RGB(0xc0, 0x00, 0x00),
	[0x3d] = RGB(0x80, 0x00, 0x00),
	[0x3e] = G_COLOR_ORANGE,
	[0x41] = G_COLOR_BROWN1,
	[0x42] = RGB(0x00, 0x00, 0x66),
	[0x43] = RGB(0x33, 0x99, 0xff),
	[0x45] = RGB(0xe8, 0xe8, 0xe8),
	[0x46] = RGB(0x99, 0xcc, 0xff),
	[0x48] = RGB(0x99, 0x33, 0x33),
	[0x49] = RGB(0xcc, 0x66, 0x00),
	[0x50] = G_COLOR_GRAY3,
	[0x57] = RGB(0x99, 0x33, 0x00),
	[0x58] = G_COLOR_CYAN,
	[0x5b] = RGB(0x66, 0x00, 0x33),
	[0x60] = G_COLOR_AQUA,
	[0x61] = G_COLOR_GRAY2,
	[0x65] = G_COLOR_DARK_BLUE,
	[0x6a] = G_COLOR_GRAY1,

	[0xff] = 0,
};

static unsigned *color_tables[] = {
	default_color_table,
	color_table_1,
};

static unsigned *color_table = default_color_table;

void g_set_color_table(unsigned index)
{
	if (index >= ARRAY_SIZE(color_tables)) {
		printf("Bad color table %d - using default\n", index);
		color_table = default_color_table;
	}

	color_table = color_tables[index];
}

unsigned g_set_fill_color(uint8_t index)
{
	unsigned color;

	color = color_table[index];
	if (!color) {
		/* Unknown color - use ugly purple */
		debug_printf(DEBUG_IMAGE_DRAW, "Unknown color %.2x\n", index);
		return RGB(0xff, 0x00, 0xff);
	}

	return color;
}

static void set_color(unsigned color)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(ctx.renderer); i++)
		SDL_SetRenderDrawColor(ctx.renderer[i],
				       (color >> 24) & 0xff,
				       (color >> 16) & 0xff,
				       (color >>  8) & 0xff,
				       (color >>  0) & 0xff);
}

void g_draw_box(unsigned x1, unsigned y1, unsigned x2, unsigned y2,
		unsigned color)
{
	SDL_Rect rect;
	int i;

	rect.x = x1;
	rect.y = y1;
	rect.w = x2 - x1;
	rect.h = y2 - y1;

	set_color(color);
	for (i = 0; i < ARRAY_SIZE(ctx.renderer); i++)
		SDL_RenderDrawRect(ctx.renderer[i], &rect);
}

static void g_draw_filled_box(unsigned x1, unsigned y1,
			      unsigned x2, unsigned y2, unsigned color)
{
	SDL_Rect rect;
	int i;

	rect.x = x1;
	rect.y = y1;
	rect.w = x2 - x1;
	rect.h = y2 - y1;

	set_color(color);
	for (i = 0; i < ARRAY_SIZE(ctx.renderer); i++)
		SDL_RenderFillRect(ctx.renderer[i], &rect);
}

unsigned g_get_pixel_color(int x, int y)
{
	uint32_t *pixels, val;

	pixels = ctx.surface->pixels;
	val = pixels[(y * G_RENDER_WIDTH) + x];

	/* FIXME - correct endianess on all platforms? */
	return be32toh(val);
}

void g_draw_pixel(unsigned x, unsigned y, unsigned color)
{
	int i;

	set_color(color);
	for (i = 0; i < ARRAY_SIZE(ctx.renderer); i++)
		SDL_RenderDrawPoint(ctx.renderer[i], x, y);
}

void g_draw_line(unsigned x1, unsigned y1, unsigned x2, unsigned y2,
		 unsigned color)
{
	int i;

	set_color(color);
	for (i = 0; i < ARRAY_SIZE(ctx.renderer); i++)
		SDL_RenderDrawLine(ctx.renderer[i], x1, y1, x2, y2);
}

void g_draw_shape(int x, int y, int shape_type, unsigned fill_color)
{
	int i, j;

	switch (shape_type) {
	case IMAGE_OP_SHAPE_PIXEL:
		x += 7; y += 7;
		g_draw_pixel(x, y, fill_color);
		break;

	case IMAGE_OP_SHAPE_BOX:
		x += 6; y += 7;
		g_draw_filled_box(x, y, x + 2, y + 2, fill_color);
		break;

	case IMAGE_OP_SHAPE_CIRCLE_TINY:
		x += 5;
		y += 5;
		g_draw_filled_box(x + 1, y, x + 3, y + 4, fill_color);
		g_draw_filled_box(x, y + 1, x + 4, y + 3, fill_color);
		break;

	case IMAGE_OP_SHAPE_CIRCLE_SMALL:
		x += 4; y += 4;
		g_draw_filled_box(x + 1, y, x + 5, y + 6, fill_color);
		g_draw_filled_box(x, y + 1, x + 6, y + 5, fill_color);
		break;

	case IMAGE_OP_SHAPE_CIRCLE_MED:
		x += 1; y += 1;
		g_draw_filled_box(x + 1,
				  y + 1,
				  x + 1 + (2 + 4 + 2),
				  y + 1 + (2 + 4 + 2),
				  fill_color);
		g_draw_filled_box(x + 3,
				  y,
				  x + 3 + 4,
				  y + (1 + 2 + 4 + 2 + 1),
				  fill_color);
		g_draw_filled_box(x,
				  y + 3,
				  x + (1 + 2 + 4 + 2 + 1),
				  y + 3 + 4,
				  fill_color);
		break;

	case IMAGE_OP_SHAPE_CIRCLE_LARGE:
		g_draw_filled_box(x + 2,
				  y + 1,
				  x + 2 + (3 + 4 + 3),
				  y + 1 + (1 + 3 + 4 + 3 + 1),
				  fill_color);
		g_draw_filled_box(x + 1,
				  y + 2,
				  x + 1 + (1 + 3 + 4 + 3 + 1),
				  y + 2 + (3 + 4 + 3),
				  fill_color);
		g_draw_filled_box(x + 5,
				  y,
				  x + 5 + 4,
				  y + 1 + 1 + 3 + 4 + 3 + 1 + 1,
				  fill_color);
		g_draw_filled_box(x,
				  y + 5,
				  x + 1 + 1 + 3 + 4 + 3 + 1 + 1,
				  y + 5 + 4,
				  fill_color);
		break;

	case IMAGE_OP_SHAPE_A:
		/* FIXME - very large circle? */
		break;

	case IMAGE_OP_SHAPE_SPRAY:
	{
		char spray[13][13] = {
			{0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0},
			{0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0},
			{0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1},
			{0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
			{1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0},
			{0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0},
			{1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0},
			{0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0},
			{1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0},
			{0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0},
			{0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0},
		};
		for (i = 0; i < 13; i++)
			for (j = 0; j < 13; j++)
				if (spray[i][j])
					g_draw_pixel(x + i, y + j, fill_color);
		break;
	}

	default:
		/* Unknown shape */
		break;
	}
}

void g_floodfill(int x, int y, unsigned fill_color,
			unsigned old_color)
{
	int x1, x2, i;

	if (g_get_pixel_color(x, y) != old_color || fill_color == old_color)
		return;

	/* Left end of scanline */
	for (x1 = x; x1 > 0; x1--)
		if (g_get_pixel_color(x1 - 1, y) != old_color)
			break;

	/* Right end of scanline */
	for (x2 = x; x2 < RENDER_X_MAX; x2++)
		if (g_get_pixel_color(x2 + 1, y) != old_color)
			break;

	g_draw_line(x1, y, x2, y, fill_color);
	SDL_RenderPresent(ctx.renderer[RENDERER_SCREEN]);

	/* Scanline above */
	for (i = x1; i < x2; i++)
		if (y > 0 && g_get_pixel_color(i, y - 1) == old_color)
			g_floodfill(i, y - 1, fill_color, old_color);

	/* Scanline below */
	for (i = x1; i < x2; i++)
		if (y < RENDER_Y_MAX && g_get_pixel_color(i, y + 1) == old_color)
			g_floodfill(i, y + 1, fill_color, old_color);

}

void g_flip_buffers(void)
{
	SDL_RenderPresent(ctx.renderer[RENDERER_SCREEN]);
}

void g_clear_screen(unsigned color)
{
	int i;

	set_color(color);
	for (i = 0; i < ARRAY_SIZE(ctx.renderer); i++)
		SDL_RenderClear(ctx.renderer[i]);

	SDL_RenderPresent(ctx.renderer[RENDERER_SCREEN]);
}

void g_init(unsigned width, unsigned height)
{
	int err;

	err = SDL_Init(SDL_INIT_VIDEO);
	if (err == -1)
		fatal_error("Failed to initialize graphics\n");

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

	ctx.screen = SDL_CreateWindow("Re-Comprehend",
				      SDL_WINDOWPOS_CENTERED,
				      SDL_WINDOWPOS_CENTERED,
				      width, height, 0);

	ctx.renderer[RENDERER_SCREEN] =
		SDL_CreateRenderer(ctx.screen, -1, SDL_RENDERER_ACCELERATED);
	SDL_RenderSetLogicalSize(ctx.renderer[RENDERER_SCREEN],
				 G_RENDER_WIDTH, G_RENDER_HEIGHT);

	ctx.surface = SDL_CreateRGBSurface(0, G_RENDER_WIDTH, G_RENDER_HEIGHT,
					   32, 0x000000ff, 0x0000ff00,
					   0x00ff0000, 0xff000000);
	ctx.renderer[RENDERER_PIXEL_DATA] =
		SDL_CreateSoftwareRenderer(ctx.surface);

	graphics_enabled = true;
}

bool g_enabled(void)
{
	return graphics_enabled;
}
