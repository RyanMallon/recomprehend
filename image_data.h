/*
 * This file is public domain
 *
 * Ryan Mallon, 2015
 *
 */

#ifndef _RECOMPREHEND_IMAGE_DATA_H
#define _RECOMPREHEND_IMAGE_DATA_H

#include <stdint.h>
#include <stdio.h>

struct file_buf;
struct comprehend_game;

struct image_data {
	struct file_buf	*fb;
	uint16_t	*image_offsets;
	size_t		nr_images;
};

#define IMAGEF_OP_WAIT_KEYPRESS		(1 << 0)
#define IMAGEF_NO_FLOODFILL		(1 << 1)

#define IMAGE_OP_SCENE_END		0x00

#define IMAGE_OP_SET_TEXT_POS		0x10

#define IMAGE_OP_PEN_COLOR_A		0x20
#define IMAGE_OP_PEN_COLOR_B		0x21
#define IMAGE_OP_PEN_COLOR_C		0x22
#define IMAGE_OP_PEN_COLOR_D		0x23
#define IMAGE_OP_PEN_COLOR_E		0x24
#define IMAGE_OP_PEN_COLOR_F		0x25
#define IMAGE_OP_PEN_COLOR_G		0x26
#define IMAGE_OP_PEN_COLOR_H		0x27

#define IMAGE_OP_DRAW_CHAR		0x30

#define IMAGE_OP_SHAPE_PIXEL		0x40
#define IMAGE_OP_SHAPE_BOX		0x41
#define IMAGE_OP_SHAPE_CIRCLE_TINY	0x42
#define IMAGE_OP_SHAPE_CIRCLE_SMALL	0x43
#define IMAGE_OP_SHAPE_CIRCLE_MED	0x44
#define IMAGE_OP_SHAPE_CIRCLE_LARGE	0x45
#define IMAGE_OP_SHAPE_A		0x46
#define IMAGE_OP_SHAPE_SPRAY		0x47

#define IMAGE_OP_EOF			0x55

#define IMAGE_OP_FILL_COLOR		0x60

#define IMAGE_OP_MOVE_TO		0x80
#define IMAGE_OP_MOVE_TO_FAR		0x81

#define IMAGE_OP_DRAW_BOX		0x90
#define IMAGE_OP_DRAW_BOX_FAR		0x91

#define IMAGE_OP_DRAW_LINE		0xa0
#define IMAGE_OP_DRAW_LINE_FAR		0xa1

#define IMAGE_OP_DRAW_SHAPE		0xc0
#define IMAGE_OP_DRAW_SHAPE_FAR		0xc1

#define IMAGE_OP_PAINT			0xe0
#define IMAGE_OP_PAINT_FAR		0xe1

void image_set_draw_flags(unsigned flags);

void draw_dark_room(void);
void draw_bright_room(void);
void draw_image(struct image_data *info, unsigned index);
void draw_location_image(struct image_data *info, unsigned index);

void comprehend_load_image_file(const char *filename, struct image_data *info);
void comprehend_load_images(struct comprehend_game *game, const char *game_dir);

#endif /* _RECOMPREHEND_IMAGE_DATA_H */
