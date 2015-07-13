/*
 * This file is part of Re-Comprehend
 *
 * Ryan Mallon, 2015, <rmallon@gmail.com>
 *
 * To the extent possible under law, the author(s) have dedicated all
 * copyright and related and neighboring rights to this software to
 * the public domain worldwide. This software is distributed without
 * any warranty.  You should have received a copy of the CC0 Public
 * Domain Dedication along with this software. If not, see
 *
 * <http://creativecommons.org/publicdomain/zero/1.0/>.
 *
 */

#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "recomprehend.h"
#include "game_data.h"
#include "dump_game_data.h"
#include "dictionary.h"
#include "file_buf.h"
#include "strings.h"
#include "game.h"
#include "util.h"
#include "opcode_map.h"

static const char *opcode_names[] = {
	[OPCODE_UNKNOWN]			= "unknown",

	[OPCODE_HAVE_OBJECT]			= "have_object",
	[OPCODE_NOT_HAVE_OBJECT]		= "not_have_object",
	[OPCODE_HAVE_CURRENT_OBJECT]		= "have_current_object",
	[OPCODE_NOT_HAVE_CURRENT_OBJECT]	= "not_have_current_object",

	[OPCODE_OBJECT_IS_NOT_NOWHERE]		= "object_is_not_nowhere",

	[OPCODE_CURRENT_OBJECT_TAKEABLE]	= "current_object_takeable",
	[OPCODE_CURRENT_OBJECT_NOT_TAKEABLE]	= "current_object_not_takeable",

	[OPCODE_CURRENT_OBJECT_IS_NOWHERE]	= "current_object_is_nowhere",

	[OPCODE_CURRENT_OBJECT_NOT_PRESENT]	= "current_object_not_present",

	[OPCODE_TAKE_OBJECT]			= "take_object",
	[OPCODE_TAKE_CURRENT_OBJECT]		= "take_current_object",
	[OPCODE_DROP_OBJECT]			= "drop_object",
	[OPCODE_DROP_CURRENT_OBJECT]		= "drop_current_object",

	[OPCODE_OR]				= "or",
	[OPCODE_IN_ROOM]			= "in_room",
	[OPCODE_VAR_EQ]				= "var_eq",
	[OPCODE_OBJECT_NOT_VALID]	        = "object_not_valid",
	[OPCODE_INVENTORY_FULL]			= "inventory_full",
	[OPCODE_OBJECT_PRESENT]			= "object_present",
	[OPCODE_ELSE]				= "else",
	[OPCODE_OBJECT_IN_ROOM]			= "object_in_room",
	[OPCODE_TEST_FLAG]			= "test_flag",
	[OPCODE_CURRENT_OBJECT_IN_ROOM]		= "current_object_in_room",
	[OPCODE_CURRENT_OBJECT_PRESENT]		= "current_object_present",
	[OPCODE_TEST_ROOM_FLAG]			= "test_room_flag",
	[OPCODE_NOT_IN_ROOM]			= "not_in_room",
	[OPCODE_OBJECT_NOT_PRESENT]		= "object_not_present",
	[OPCODE_OBJECT_NOT_IN_ROOM]		= "object_not_in_room",
	[OPCODE_TEST_NOT_FLAG]			= "test_not_flag",
	[OPCODE_OBJECT_IS_NOWHERE]		= "object_is_nowhere",
	[OPCODE_TEST_NOT_ROOM_FLAG]		= "test_not_room_flag",
	[OPCODE_INVENTORY]			= "inventory",
	[OPCODE_MOVE_OBJECT_TO_ROOM]		= "move_object_to_room",
	[OPCODE_SAVE_ACTION]			= "save_action",
	[OPCODE_MOVE_TO_ROOM]			= "move_to_room",
	[OPCODE_VAR_ADD]			= "var_add",
	[OPCODE_SET_ROOM_DESCRIPTION]		= "set_room_description",
	[OPCODE_MOVE_OBJECT_TO_CURRENT_ROOM]	= "move_object_to_current_room",
	[OPCODE_VAR_SUB]			= "var_sub",
	[OPCODE_SET_OBJECT_DESCRIPTION]		= "set_object_description",
	[OPCODE_SET_OBJECT_LONG_DESCRIPTION]	= "set_object_long_description",
	[OPCODE_MOVE]				= "move",
	[OPCODE_PRINT]				= "print",
	[OPCODE_REMOVE_OBJECT]			= "remove_object",
	[OPCODE_SET_FLAG]			= "set_flag",
	[OPCODE_CALL_FUNC]			= "call_func",
	[OPCODE_TURN_TICK]			= "turn_tick",
	[OPCODE_CLEAR_FLAG]			= "clear_flag",
	[OPCODE_INVENTORY_ROOM]			= "inventory_room",
	[OPCODE_SPECIAL]			= "special",
	[OPCODE_SET_ROOM_GRAPHIC]		= "set_room_graphic",
	[OPCODE_SET_OBJECT_GRAPHIC]		= "set_object_graphic",
	[OPCODE_REMOVE_CURRENT_OBJECT]		= "remove_current_object",
	[OPCODE_DO_VERB]			= "do_verb",
	[OPCODE_VAR_INC]			= "var_inc",
	[OPCODE_VAR_DEC]			= "var_dec",
	[OPCODE_MOVE_CURRENT_OBJECT_TO_ROOM]	= "move_current_object_to_room",
	[OPCODE_DESCRIBE_CURRENT_OBJECT]	= "describe_current_object",
	[OPCODE_SET_STRING_REPLACEMENT]		= "set_string_replacement",
	[OPCODE_SET_CURRENT_NOUN_STRING_REPLACEMENT] = "set_current_noun_string_replacement",
	[OPCODE_CURRENT_NOT_OBJECT]		= "current_not_object",
	[OPCODE_CURRENT_IS_OBJECT]		= "current_is_object",
	[OPCODE_DRAW_ROOM]			= "draw_room",
	[OPCODE_DRAW_OBJECT]			= "draw_object",
	[OPCODE_WAIT_KEY]			= "wait_key",
};

void dump_instruction(struct comprehend_game *game,
		      struct function_state *func_state,
		      struct instruction *instr)
{
	int i, str_index, str_table;
	uint8_t *opcode_map, opcode;

	if (func_state)
		printf("[or=%d,and=%d,test=%d,else=%d]",
		       func_state->or_count, func_state->and,
		       func_state->test_result, func_state->else_result);

	opcode_map = get_opcode_map(game);
	opcode = opcode_map[instr->opcode];

	printf("  [%.2x] ", instr->opcode);
	if (opcode < ARRAY_SIZE(opcode_names) && opcode_names[opcode])
		printf("%s", opcode_names[opcode]);
	else
		printf("unknown");

	if (instr->nr_operands) {
		printf("(");
		for (i = 0; i < instr->nr_operands; i++)
			printf("%.2x%s", instr->operand[i],
			       i == instr->nr_operands - 1 ? ")" : ", ");
	}

	switch (opcode) {
	case OPCODE_PRINT:
	case OPCODE_SET_ROOM_DESCRIPTION:
	case OPCODE_SET_OBJECT_DESCRIPTION:
	case OPCODE_SET_OBJECT_LONG_DESCRIPTION:

		if (opcode == OPCODE_PRINT) {
			str_index = instr->operand[0];
			str_table = instr->operand[1];
		} else {
			str_index = instr->operand[1];
			str_table = instr->operand[2];
		}

		printf(" %s", instr_lookup_string(game, str_index, str_table));
		break;

	case OPCODE_SET_STRING_REPLACEMENT:
		printf(" %s", game->info->replace_words[instr->operand[0] - 1]);
		break;
	}

	printf("\n");
}

static void dump_functions(struct comprehend_game *game)
{
	struct function *func;
	int i, j;

	printf("Functions (%zd entries)\n", game->info->nr_functions);
	for (i = 0; i < game->info->nr_functions; i++) {
		func = &game->info->functions[i];

		printf("[%.4x] (%zd instructions)\n", i, func->nr_instructions);
		for (j = 0; j < func->nr_instructions; j++)
			dump_instruction(game, NULL, &func->instructions[j]);
		printf("\n");
	}
}

static void dump_action_table(struct comprehend_game *game)
{
	struct action *action;
	struct word *word;
	int i, j;

	printf("Action table (%zd entries)\n", game->info->nr_actions);
	for (i = 0; i < game->info->nr_actions; i++) {
		action = &game->info->action[i];

		printf("(");
		for (j = 0; j < 4; j++) {
			if (j < action->nr_words) {
				switch (action->word_type[j]) {
				case WORD_TYPE_VERB: printf("v"); break;
				case WORD_TYPE_JOIN: printf("j"); break;
				case WORD_TYPE_NOUN_MASK: printf("n"); break;
				default: printf("?"); break;
				}
			} else {
				printf(" ");
			}
		}

		printf(") [%.4x] ", i );

		for (j = 0; j < action->nr_words; j++)
			printf("%.2x:%.2x ",
			       action->word[j], action->word_type[j]);

		printf("| ");

		for (j = 0; j < action->nr_words; j++) {
			word = find_dict_word_by_index(game, action->word[j],
						       action->word_type[j]);
			if (word)
				printf("%-6s ", word->word);
			else
				printf("%.2x:%.2x  ", action->word[j],
				       action->word_type[j]);
		}

		printf("-> %.4x\n", action->function);
	}
}

static int word_index_compare(const void *a, const void *b)
{
	const struct word *word_a = a, *word_b = b;

	if (word_a->index > word_b->index)
		return 1;
	if (word_a->index < word_b->index)
		return -1;
	return 0;
}

static void dump_dictionary(struct comprehend_game *game)
{
	struct word *dictionary;
	struct word *word;
	int i;

	/* Sort the dictionary by index */
	dictionary = xmalloc(sizeof(*word) * game->info->nr_words);
	memcpy(dictionary, game->info->words,
	       sizeof(*word) * game->info->nr_words);
	qsort(dictionary, game->info->nr_words, sizeof(*word),
	      word_index_compare);

	printf("Dictionary (%zd words)\n", game->info->nr_words);
	for (i = 0; i < game->info->nr_words; i++) {
		word = &dictionary[i];
		printf("  [%.2x] %.2x %s\n", word->index, word->type,
		       word->word);
	}

	free(dictionary);
}

static void dump_word_map(struct comprehend_game *game)
{
	struct word *word[3];
	char str[3][6];
	struct word_map *map;
	int i, j;

	printf("Word pairs (%zd entries)\n", game->info->nr_word_maps);
	for (i = 0; i < game->info->nr_word_maps; i++) {
		map = &game->info->word_map[i];

		for (j = 0; j < 3; j++) {
			word[j] = dict_find_word_by_index_type(
				game, map->word[j].index, map->word[j].type);
			if (word[j])
				snprintf(str[j], sizeof(str[j]),
					 "%s", word[j]->word);
			else
				snprintf(str[j], sizeof(str[j]), "%.2x:%.2x ",
					 map->word[j].index, map->word[j].type);
		}

		printf("  [%.2x] %-6s %-6s -> %-6s\n",
		       i, str[0], str[1], str[2]);
	}
}

static void dump_rooms(struct comprehend_game *game)
{
	struct room *room;
	int i;

	/* Room zero acts as the players inventory */
	printf("Rooms (%zd entries)\n", game->info->nr_rooms);
	for (i = 1; i <= game->info->nr_rooms; i++) {
		room = &game->info->rooms[i];

		printf("  [%.2x] flags=%.2x, graphic=%.2x\n",
		       i, room->flags, room->graphic);
		printf("    %s\n", string_lookup(game, room->string_desc));
		printf("    n: %.2x  s: %.2x  e: %.2x  w: %.2x\n",
		       room->direction[DIRECTION_NORTH],
		       room->direction[DIRECTION_SOUTH],
		       room->direction[DIRECTION_EAST],
		       room->direction[DIRECTION_WEST]);
		printf("    u: %.2x  d: %.2x  i: %.2x  o: %.2x\n",
		       room->direction[DIRECTION_UP],
		       room->direction[DIRECTION_DOWN],
		       room->direction[DIRECTION_IN],
		       room->direction[DIRECTION_OUT]);
		printf("\n");
	}
}

static void dump_items(struct comprehend_game *game)
{
	struct item *item;
	int i, j;

	printf("Items (%zd entries)\n", game->info->header.nr_items);
	for (i = 0; i < game->info->header.nr_items; i++) {
		item = &game->info->item[i];

		printf("  [%.2x] %s\n", i + 1,
		       item->string_desc ?
		       string_lookup(game, item->string_desc) : "");
		if (game->info->comprehend_version == 2)
			printf("    long desc: %s\n",
			       string_lookup(game, item->long_string));

		printf("    words: ");
		for (j = 0; j < game->info->nr_words; j++)
			if (game->info->words[j].index == item->word &&
			    (game->info->words[j].type & WORD_TYPE_NOUN_MASK))
				printf("%s ", game->info->words[j].word);
		printf("\n");
		printf("    flags=%.2x (takeable=%d, weight=%d)\n",
		       item->flags, !!(item->flags & ITEMF_CAN_TAKE),
		       (item->flags & ITEMF_WEIGHT_MASK));
		printf("    room=%.2x, graphic=%.2x\n",
		       item->room, item->graphic);
		printf("\n");
	}
}

static void dump_string_table(struct string_table *table)
{
	int i;

	for (i = 0; i < table->nr_strings; i++)
		printf("[%.4x] %s\n", i, table->strings[i]);
}

static void dump_game_data_strings(struct comprehend_game *game)
{
	printf("Main string table (%zd entries)\n",
	       game->info->strings.nr_strings);
	dump_string_table(&game->info->strings);
}

static void dump_extra_strings(struct comprehend_game *game)
{
	printf("Extra strings (%zd entries)\n",
	       game->info->strings2.nr_strings);
	dump_string_table(&game->info->strings2);
}

static void dump_replace_words(struct comprehend_game *game)
{
	int i;

	printf("Replacement words (%zd entries)\n",
	       game->info->nr_replace_words);
	for (i = 0; i < game->info->nr_replace_words; i++)
		printf("  [%.2x] %s\n", i + 1, game->info->replace_words[i]);
}

static void dump_header(struct comprehend_game *game)
{
	struct game_header *header = &game->info->header;
	uint16_t *dir_table = header->room_direction_table;

	printf("Game header:\n");
	printf("  magic:                %.4x\n", header->magic);
	printf("  action(vvnn):         %.4x\n", header->addr_actions_vvnn);
	printf("  actions(?):\n");
	printf("  actions(vnjn):        %.4x\n", header->addr_actions_vnjn);
	printf("  actions(vjn):         %.4x\n", header->addr_actions_vjn);
	printf("  actions(vdn):         %.4x\n", header->addr_actions_vdn);
	printf("  actions(vnn):         %.4x\n", header->addr_actions_vnn);
	printf("  actions(vn):          %.4x\n", header->addr_actions_vn);
	printf("  actions(v):           %.4x\n", header->addr_actions_v);
	printf("  functions:            %.4x\n", header->addr_vm);
	printf("  dictionary:           %.4x\n", header->addr_dictionary);
	printf("  word map pairs:       %.4x\n", header->addr_word_map);
	printf("  room desc strings:    %.4x\n", header->room_desc_table);
	printf("  room north:           %.4x\n", dir_table[DIRECTION_NORTH]);
	printf("  room south:           %.4x\n", dir_table[DIRECTION_SOUTH]);
	printf("  room east:            %.4x\n", dir_table[DIRECTION_EAST]);
	printf("  room west:            %.4x\n", dir_table[DIRECTION_WEST]);
	printf("  room up:              %.4x\n", dir_table[DIRECTION_UP]);
	printf("  room down:            %.4x\n", dir_table[DIRECTION_DOWN]);
	printf("  room in:              %.4x\n", dir_table[DIRECTION_IN]);
	printf("  room out:             %.4x\n", dir_table[DIRECTION_OUT]);
	printf("  room flags:           %.4x\n", header->room_flags_table);
	printf("  room images:          %.4x\n", header->room_graphics_table);
	printf("  item locations:       %.4x\n", header->addr_item_locations);
	printf("  item flags:           %.4x\n", header->addr_item_flags);
	printf("  item words:           %.4x\n", header->addr_item_word);
	printf("  item desc strings:    %.4x\n", header->addr_item_strings);
	printf("  item images:          %.4x\n", header->addr_item_graphics);
	printf("  string table:         %.4x\n", header->addr_strings);
	printf("  string table end:     %.4x\n", header->addr_strings_end);
}

typedef void (*dump_func_t)(struct comprehend_game *game);

struct dumper {
	dump_func_t	dump_func;
	unsigned	flag;
};

static struct dumper dumpers[] = {
	{dump_header,			DUMP_HEADER},
	{dump_game_data_strings,	DUMP_STRINGS},
	{dump_extra_strings,		DUMP_EXTRA_STRINGS},
	{dump_rooms,			DUMP_ROOMS},
	{dump_items,			DUMP_ITEMS},
	{dump_dictionary,		DUMP_DICTIONARY},
	{dump_word_map,			DUMP_WORD_PAIRS},
	{dump_action_table,		DUMP_ACTIONS},
	{dump_functions,		DUMP_FUNCTIONS},
	{dump_replace_words,		DUMP_REPLACE_WORDS},
};

void dump_game_data(struct comprehend_game *game, unsigned flags)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(dumpers); i++)
		if (flags & dumpers[i].flag) {
			dumpers[i].dump_func(game);
			printf("\n\n");
		}
}
