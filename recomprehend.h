/*
 * This file is public domain
 *
 * Ryan Mallon, 2015
 *
 */

#ifndef _RECOMPREHEND_RECOMPREHEND_H
#define _RECOMPREHEND_RECOMPREHEND_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_FILES	10

struct comprehend_game;
struct game_info;
struct game_state;

struct string_file {
	const char		*filename;
	uint32_t		base_offset;
	uint32_t		end_offset;
};

#define EXTRA_STRING_TABLE(x)	(0x8200 | (x))

struct game_strings {
	uint16_t	game_restart;
};

#define ROOM_IS_NORMAL		0
#define ROOM_IS_DARK		1
#define ROOM_IS_TOO_BRIGHT	2

struct game_ops {
	void (*before_game)(struct comprehend_game *game);
	void (*before_prompt)(struct comprehend_game *game);
	bool (*before_turn)(struct comprehend_game *game);
	bool (*after_turn)(struct comprehend_game *game);
	int (*room_is_special)(struct comprehend_game *game,
			       unsigned room_index,
			       unsigned *room_desc_string);
	void (*handle_special_opcode)(struct comprehend_game *game,
				      uint8_t operand);
};

struct comprehend_game {
	const char		*game_name;
	const char		*short_name;

	const char		*game_dir;

	const char		*game_data_file;
	struct string_file	string_files[MAX_FILES];
	const char		*location_graphic_files[MAX_FILES];
	const char		*item_graphic_files[MAX_FILES];
	const char		*save_game_file_fmt;
	unsigned		color_table;

	struct game_strings	*strings;
	struct game_ops		*ops;

	struct game_info	*info;
};

#endif /* _RECOMPREHEND_RECOMPREHEND_H */
