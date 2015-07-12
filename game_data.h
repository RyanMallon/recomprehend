/*
 * This file is public domain
 *
 * Ryan Mallon, 2015
 *
 */

#ifndef _RECOMPREHEND_GAME_DATA_H
#define _RECOMPREHEND_GAME_DATA_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "image_data.h"

struct comprehend_game;

#define MAX_FLAGS	64
#define MAX_VARIABLES	128

enum {
	DIRECTION_NORTH,
	DIRECTION_SOUTH,
	DIRECTION_EAST,
	DIRECTION_WEST,
	DIRECTION_UP,
	DIRECTION_DOWN,
	DIRECTION_IN,
	DIRECTION_OUT,
	NR_DIRECTIONS,
};

struct function_state {
	bool		test_result;
	bool		else_result;
	unsigned	or_count;
	bool		and;
	bool		in_command;
	bool		executed;
};

struct room {
	uint8_t			direction[NR_DIRECTIONS];
	uint8_t			flags;
	uint8_t			graphic;
	uint16_t		string_desc;
};

struct item {
	uint16_t		string_desc;
	uint16_t		long_string;	/* Only used by version 2 */
	uint8_t			room;
	uint8_t			flags;
	uint8_t			word;
	uint8_t			graphic;
};

struct word {
	char			word[7];
	uint8_t			index;
	uint8_t			type;
};

struct word_index {
	uint8_t			index;
	uint8_t			type;
};

struct word_map {
	/* <word[0]>, <word[1]> == <word[2]> */
	struct word_index	word[3];
	uint8_t			flags;
};

struct action {
	int			type;
	size_t			nr_words;
	// FIXME - use struct word_index here.
	uint8_t			word[4];
	uint8_t			word_type[4];
	uint16_t		function;
};

struct instruction {
	uint8_t			opcode;
	size_t			nr_operands;
	uint8_t			operand[3];
	bool			is_command;
};

struct function {
	struct instruction	instructions[0x100];
	size_t			nr_instructions;
};

struct string_table {
	char			*strings[0xffff];
	size_t			nr_strings;
};

struct game_header {
	uint16_t		magic;

	uint16_t		room_desc_table;
	uint16_t		room_direction_table[NR_DIRECTIONS];
	uint16_t		room_flags_table;
	uint16_t		room_graphics_table;

	size_t			nr_items;
	uint16_t		addr_item_locations;
	uint16_t		addr_item_flags;
	uint16_t		addr_item_word;
	uint16_t		addr_item_strings;
	uint16_t		addr_item_graphics;

	uint16_t		addr_dictionary;
	uint16_t		addr_word_map;

	uint16_t		addr_strings;
	uint16_t		addr_strings_end;

	uint16_t		addr_actions_vvnn;
	uint16_t		addr_actions_unknown;
	uint16_t		addr_actions_vnjn;
	uint16_t		addr_actions_vjn;
	uint16_t		addr_actions_vdn;
	uint16_t		addr_actions_vnn;
	uint16_t		addr_actions_vn;
	uint16_t		addr_actions_v;

	uint16_t		addr_vm; // FIXME - functions
};

struct game_info {
	struct game_header	header;

	unsigned		comprehend_version;

	uint8_t			start_room;

	struct room		rooms[0x100];
	size_t			nr_rooms;
	uint8_t			current_room;

	struct item		item[0xff];

	struct word		*words;
	size_t			nr_words;

	struct word_map		word_map[0xff];
	size_t			nr_word_maps;

	struct string_table	strings;
	struct string_table	strings2;

	struct action		action[0xffff];
	size_t			nr_actions;

	struct function		functions[0xffff];
	size_t			nr_functions;

	struct image_data	room_images;
	struct image_data	item_images;

	bool			flags[MAX_FLAGS];
	uint16_t		variable[MAX_VARIABLES];

	char			*replace_words[256];
	size_t			nr_replace_words;

	uint8_t			current_replace_word;
	unsigned		update_flags;
};

enum {
	OPCODE_UNKNOWN,
	OPCODE_TEST_FALSE,
	OPCODE_HAVE_OBJECT,
	OPCODE_OR,
	OPCODE_IN_ROOM,
	OPCODE_VAR_EQ,
	OPCODE_CURRENT_OBJECT_TAKEABLE,
	OPCODE_OBJECT_PRESENT,
	OPCODE_ELSE,
	OPCODE_OBJECT_IN_ROOM,
	OPCODE_OBJECT_NOT_VALID,
	OPCODE_INVENTORY_FULL,
	OPCODE_TEST_FLAG,
	OPCODE_CURRENT_OBJECT_IN_ROOM,
	OPCODE_HAVE_CURRENT_OBJECT,
	OPCODE_OBJECT_IS_NOT_NOWHERE,
	OPCODE_CURRENT_OBJECT_PRESENT,
	OPCODE_TEST_ROOM_FLAG,
	OPCODE_NOT_HAVE_OBJECT,
	OPCODE_NOT_IN_ROOM,
	OPCODE_CURRENT_OBJECT_IS_NOWHERE,
	OPCODE_OBJECT_NOT_PRESENT,
	OPCODE_OBJECT_NOT_IN_ROOM,
	OPCODE_TEST_NOT_FLAG,
	OPCODE_NOT_HAVE_CURRENT_OBJECT,
	OPCODE_OBJECT_IS_NOWHERE,
	OPCODE_CURRENT_OBJECT_NOT_PRESENT,
	OPCODE_CURRENT_OBJECT_NOT_TAKEABLE,
	OPCODE_TEST_NOT_ROOM_FLAG,
	OPCODE_INVENTORY,
	OPCODE_TAKE_OBJECT,
	OPCODE_MOVE_OBJECT_TO_ROOM,
	OPCODE_SAVE_ACTION,
	OPCODE_MOVE_TO_ROOM,
	OPCODE_VAR_ADD,
	OPCODE_SET_ROOM_DESCRIPTION,
	OPCODE_MOVE_OBJECT_TO_CURRENT_ROOM,
	OPCODE_VAR_SUB,
	OPCODE_SET_OBJECT_DESCRIPTION,
	OPCODE_SET_OBJECT_LONG_DESCRIPTION,
	OPCODE_MOVE,
	OPCODE_MOVE_DIRECTION,
	OPCODE_PRINT,
	OPCODE_REMOVE_OBJECT,
	OPCODE_SET_FLAG,
	OPCODE_CALL_FUNC,
	OPCODE_TURN_TICK,
	OPCODE_CLEAR_FLAG,
	OPCODE_INVENTORY_ROOM,
	OPCODE_TAKE_CURRENT_OBJECT,
	OPCODE_SPECIAL,
	OPCODE_DROP_OBJECT,
	OPCODE_DROP_CURRENT_OBJECT,
	OPCODE_SET_ROOM_GRAPHIC,
	OPCODE_SET_OBJECT_GRAPHIC,
	OPCODE_REMOVE_CURRENT_OBJECT,
	OPCODE_DO_VERB,
	OPCODE_VAR_INC,
	OPCODE_VAR_DEC,
	OPCODE_MOVE_CURRENT_OBJECT_TO_ROOM,
	OPCODE_DESCRIBE_CURRENT_OBJECT,
	OPCODE_SET_STRING_REPLACEMENT,
	OPCODE_SET_CURRENT_NOUN_STRING_REPLACEMENT,
	OPCODE_CURRENT_NOT_OBJECT,
	OPCODE_CURRENT_IS_OBJECT,
	OPCODE_DRAW_ROOM,
	OPCODE_DRAW_OBJECT,
	OPCODE_WAIT_KEY,
};

/* Game state update flags */
#define UPDATE_GRAPHICS		(1 << 0) /* Implies UPDATE_GRAPHICS_ITEMS */
#define UPDATE_GRAPHICS_ITEMS	(1 << 1)
#define UPDATE_ROOM_DESC	(1 << 2)
#define UPDATE_ITEM_LIST	(1 << 3)
#define UPDATE_ALL		(~0)

/* Action types */
enum {
	ACTION_VERB_VERB_NOUN_NOUN,
	ACTION_VERB_NOUN_JOIN_NOUN,
	ACTION_VERB_JOIN_NOUN,
	ACTION_VERB_DIR_NOUN,
	ACTION_VERB_NOUN_NOUN,
	ACTION_VERB_NOUN,
	ACTION_VERB_OPT_NOUN,
};

/* Standard strings (main string table) */
#define STRING_CANT_GO		0
#define STRING_DONT_UNDERSTAND	1
#define STRING_YOU_SEE		2
#define STRING_INVENTORY	3
#define STRING_INVENTORY_EMPTY	4
#define STRING_BEFORE_CONTINUE	5
#define STRING_SAVE_GAME	6
#define STRING_RESTORE_GAME	7

/* Special variables */
#define VAR_INVENTORY_WEIGHT	0
#define VAR_INVENTORY_LIMIT	1
#define VAR_TURN_COUNT		2

/* Special rooms */
#define ROOM_INVENTORY		0x00
#define ROOM_NOWHERE		0xff

/* Item flags */
#define ITEMF_WEIGHT_MASK	(0x3)
#define ITEMF_CAN_TAKE		(1 << 3)

/* Word types */
#define WORD_TYPE_VERB		0x01
#define WORD_TYPE_JOIN		0x02
#define WORD_TYPE_FEMALE	0x10
#define WORD_TYPE_MALE		0x20
#define WORD_TYPE_NOUN		0x40
#define WORD_TYPE_NOUN_PLURAL	0x80
#define WORD_TYPE_NOUN_MASK	(WORD_TYPE_FEMALE | WORD_TYPE_MALE |	\
				 WORD_TYPE_NOUN | WORD_TYPE_NOUN_PLURAL)

void comprehend_load_game(struct comprehend_game *game, const char *dirname);
void comprehend_restore_game(struct comprehend_game *game,
			     const char *filename);
void comprehend_save_game(struct comprehend_game *game, const char *filename);

#endif /* _RECOMPREHEND_GAME_DATA_H */
