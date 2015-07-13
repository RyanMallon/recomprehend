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

#include <sys/ioctl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>
#include <ctype.h>

#include "recomprehend.h"
#include "dictionary.h"
#include "game_data.h"
#include "dump_game_data.h"
#include "graphics.h"
#include "strings.h"
#include "game.h"
#include "util.h"
#include "opcode_map.h"

struct sentence {
	struct word	words[4];
	size_t		nr_words;
};

static struct winsize console_winsize;

static void console_init(void)
{
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &console_winsize);
}

int console_get_key(void)
{
	int c, dummy;

	dummy = c = getchar();

	/* Clear input buffer */
	while (dummy != '\n' && dummy != EOF)
		dummy = getchar();

	return c;
}

void console_println(struct comprehend_game *game, const char *text)
{
	const char *replace, *word, *p = text;
	char bad_word[64];
	size_t line_length = 0;
	int word_len;

	if (!text) {
		printf("\n");
		return;
	}

	while (*p) {
		switch (*p) {
		case '\n':
			word = NULL;
			word_len = 0;
			line_length = 0;
			printf("\n");
			p++;
			break;

		case '@':
			/* Replace word */
			if (game->info->current_replace_word >= game->info->nr_replace_words) {
				snprintf(bad_word, sizeof(bad_word),
					 "[BAD_REPLACE_WORD(%.2x)]",
					 game->info->current_replace_word);
				word = bad_word;
			} else {
				word = game->info->replace_words[game->info->current_replace_word];
			}
			word_len = strlen(word);
			p++;
			break;

		default:
			/* Find next space */
			word_len = strcspn(p, " \n");
			if (word_len == 0)
				break;

			/*
			 * If this word contains a replacement symbol, then
			 * print everything before the symbol.
			 */
			replace = strchr(p, '@');
			if (replace)
				word_len = replace - p;

			word = p;
			p += word_len;
			break;
		}

		if (!word || !word_len)
			continue;

		/* Print this word */
		if (line_length + word_len > console_winsize.ws_col) {
			/* Too long - insert a line break */
			printf("\n");
			line_length = 0;
		}

		printf("%.*s", word_len, word);
		line_length += word_len;

		if (*p == ' ') {
			if (line_length >= console_winsize.ws_col) {
				/* Newline, don't print the space */
				printf("\n");
				line_length = 0;
			} else {
				printf(" ");
				line_length++;
			}
			p++;

			/* Skip any double spaces */
			while (*p == ' ')
				p++;
		}
	}

	printf("\n");
}

static struct room *get_room(struct comprehend_game *game, uint16_t index)
{
	/* Room zero is reserved for the players inventory */
	if (index == 0)
		fatal_error("Room index 0 (player inventory) is invalid");

	if (index - 1 >= game->info->nr_rooms)
		fatal_error("Room index %d is invalid", index);

	return &game->info->rooms[index];
}

struct item *get_item(struct comprehend_game *game, uint16_t index)
{
	if (index >= game->info->header.nr_items)
		fatal_error("Bad item %d\n", index);

	return &game->info->item[index];
}

void game_save(struct comprehend_game *game)
{
	char path[PATH_MAX], filename[32];
	int c;

	console_println(game, game->info->strings.strings[STRING_SAVE_GAME]);

	c = console_get_key();
	if (c < '1' || c > '3') {
		/*
		 * The original Comprehend games just silently ignore any
		 * invalid selection.
		 */
		console_println(game, "Invalid save game number");
		return;
	}

	snprintf(filename, sizeof(filename), game->save_game_file_fmt, c - '0');
	snprintf(path, sizeof(path), "%s%s", game->game_dir, filename);
	comprehend_save_game(game, path);
}

void game_restore(struct comprehend_game *game)
{
	char path[PATH_MAX], filename[32];
	int c;

	console_println(game, game->info->strings.strings[STRING_RESTORE_GAME]);

	c = console_get_key();
	if (c < '1' || c > '3') {
		/*
		 * The original Comprehend games just silently ignore any
		 * invalid selection.
		 */
		console_println(game, "Invalid save game number");
		return;
	}

	snprintf(filename, sizeof(filename), game->save_game_file_fmt, c - '0');
	snprintf(path, sizeof(path), "%s%s", game->game_dir, filename);
	comprehend_restore_game(game, path);

	game->info->update_flags = UPDATE_ALL;
}

void game_restart(struct comprehend_game *game)
{
	console_println(game, string_lookup(game, game->strings->game_restart));
	console_get_key();

	comprehend_load_game(game, game->game_dir);
	game->info->update_flags = UPDATE_ALL;
}

static struct word_index *is_word_pair(struct comprehend_game *game,
				       struct word *word1, struct word *word2)
{
	struct word_map *map;
	int i;

	/* Check if this is a word pair */
	for (i = 0; i < game->info->nr_word_maps; i++) {
		map = &game->info->word_map[i];

		if (map->word[0].index == word1->index &&
		    map->word[0].type == word1->type &&
		    map->word[1].index == word2->index &&
		    map->word[1].type == word2->type)
			return &map->word[2];
	}

	return NULL;
}

static struct item *get_item_by_noun(struct comprehend_game *game,
				     struct word *noun)
{
	int i;

	if (!noun || !(noun->type & WORD_TYPE_NOUN_MASK))
		return NULL;

	/*
	 * FIXME - in oo-topos the word 'box' matches more than one object
	 *         (the box and the snarl-in-a-box). The player is unable
	 *         to drop the latter because this will match the former.
	 */
	for (i = 0; i < game->info->header.nr_items; i++)
		if (game->info->item[i].word == noun->index)
			return &game->info->item[i];

	return NULL;
}

static void update_graphics(struct comprehend_game *game)
{
	struct item *item;
	struct room *room;
	int type, i;

	if (!g_enabled())
		return;

	type = ROOM_IS_NORMAL;
	if (game->ops->room_is_special)
		type = game->ops->room_is_special(game, game->info->current_room, NULL);

	switch (type) {
	case ROOM_IS_DARK:
		if (game->info->update_flags & UPDATE_GRAPHICS)
			draw_dark_room();
		break;

	case ROOM_IS_TOO_BRIGHT:
		if (game->info->update_flags & UPDATE_GRAPHICS)
			draw_bright_room();
		break;

	default:
		if (game->info->update_flags & UPDATE_GRAPHICS) {
			room = get_room(game, game->info->current_room);
			draw_location_image(&game->info->room_images,
					    room->graphic - 1);
		}

		if ((game->info->update_flags & UPDATE_GRAPHICS) ||
		    (game->info->update_flags & UPDATE_GRAPHICS_ITEMS)) {
			for (i = 0; i < game->info->header.nr_items; i++) {
				item = &game->info->item[i];

				if (item->room == game->info->current_room &&
				    item->graphic != 0)
					draw_image(&game->info->item_images,
						   item->graphic - 1);
			}
		}
		break;
	}
}

static void describe_objects_in_current_room(struct comprehend_game *game)
{
	struct item *item;
	size_t count = 0;
	int i;

	for (i = 0; i < game->info->header.nr_items; i++) {
		item = &game->info->item[i];

		if (item->room == game->info->current_room &&
		    item->string_desc != 0)
			count++;
	}

	if (count > 0) {
		console_println(game, string_lookup(game, STRING_YOU_SEE));

		for (i = 0; i < game->info->header.nr_items; i++) {
			item = &game->info->item[i];

			if (item->room == game->info->current_room &&
			    item->string_desc != 0)
				console_println(game, string_lookup(game, item->string_desc));
		}
	}
}

static void update(struct comprehend_game *game)
{
	struct room *room = get_room(game, game->info->current_room);
	unsigned room_type, room_desc_string;

	update_graphics(game);

	/* Check if the room is special (dark, too bright, etc) */
	room_type = ROOM_IS_NORMAL;
	room_desc_string = room->string_desc;
	if (game->ops->room_is_special)
		room_type = game->ops->room_is_special(game,
						       game->info->current_room,
						       &room_desc_string);

	if (game->info->update_flags & UPDATE_ROOM_DESC)
		console_println(game, string_lookup(game, room_desc_string));

	if ((game->info->update_flags & UPDATE_ITEM_LIST) &&
	    room_type == ROOM_IS_NORMAL)
		describe_objects_in_current_room(game);

	game->info->update_flags = 0;
}

static void move_to(struct comprehend_game *game, uint8_t room)
{
	if (room - 1 >= game->info->nr_rooms)
		fatal_error("Attempted to move to invalid room %.2x\n", room);

	game->info->current_room = room;
	game->info->update_flags = (UPDATE_GRAPHICS | UPDATE_ROOM_DESC |
				    UPDATE_ITEM_LIST);
}

static void func_set_test_result(struct function_state *func_state, bool value)
{
	if (func_state->or_count == 0) {
		/* And */
		if (func_state->and) {
			if (!value)
				func_state->test_result = false;
		} else {
			func_state->test_result = value;
			func_state->and = true;
		}

	} else {
		/* Or */
		if (value)
			func_state->test_result = value;
	}
}

static size_t num_objects_in_room(struct comprehend_game *game, int room)
{
	size_t count = 0, i;

	for (i = 0; i < game->info->header.nr_items; i++)
		if (game->info->item[i].room == room)
			count++;

	return count;
}

void move_object(struct comprehend_game *game, struct item *item, int new_room)
{
	unsigned obj_weight = item->flags & ITEMF_WEIGHT_MASK;

	if (item->room == new_room)
		return;

	if (item->room == ROOM_INVENTORY) {
		/* Removed from player's inventory */
		game->info->variable[VAR_INVENTORY_WEIGHT] -= obj_weight;
	}
	if (new_room == ROOM_INVENTORY) {
		/* Moving to the player's inventory */
		game->info->variable[VAR_INVENTORY_WEIGHT] += obj_weight;
	}

	if (item->room == game->info->current_room) {
		/* Item moved away from the current room */
		game->info->update_flags |= UPDATE_GRAPHICS;

	} else if (new_room == game->info->current_room) {
		/*
		 * Item moved into the current room. Only the item needs a
		 * redraw, not the whole room.
		 */
		game->info->update_flags |= (UPDATE_GRAPHICS_ITEMS |
					     UPDATE_ITEM_LIST);
	}

	item->room = new_room;

}

static void eval_instruction(struct comprehend_game *game,
			     struct function_state *func_state,
			     struct instruction *instr,
			     struct word *verb, struct word *noun)
{
	uint8_t *opcode_map;
	struct room *room;
	struct item *item;
	uint16_t index;
	bool test;
	int i, count;

	room = get_room(game, game->info->current_room);

	if (debugging_enabled()) {
		if (!instr->is_command) {
			printf("? ");
		} else {
			if (func_state->test_result)
				printf("+ ");
			else
				printf("- ");
		}

		dump_instruction(game, func_state, instr);
	}

	if (func_state->or_count)
		func_state->or_count--;

	if (instr->is_command) {
		bool do_command;

		func_state->in_command = true;
		do_command = func_state->test_result;

		if (func_state->or_count != 0)
			printf("Warning: or_count == %d\n",
			       func_state->or_count);
		func_state->or_count = 0;

		if (!do_command)
			return;

		func_state->else_result = false;
		func_state->executed = true;

	} else {
		if (func_state->in_command) {
			/* Finished command sequence - clear test result */
			func_state->in_command = false;
			func_state->test_result = false;
			func_state->and = false;
		}
	}

	opcode_map = get_opcode_map(game);
	switch (opcode_map[instr->opcode]) {
	case OPCODE_VAR_ADD:
		game->info->variable[instr->operand[0]] +=
			game->info->variable[instr->operand[1]];
		break;

	case OPCODE_VAR_SUB:
		game->info->variable[instr->operand[0]] -=
			game->info->variable[instr->operand[1]];
		break;

	case OPCODE_VAR_INC:
		game->info->variable[instr->operand[0]]++;
		break;

	case OPCODE_VAR_DEC:
		game->info->variable[instr->operand[0]]--;
		break;

	case OPCODE_VAR_EQ:
		func_set_test_result(func_state,
				     game->info->variable[instr->operand[0]] ==
				     game->info->variable[instr->operand[1]]);
		break;

	case OPCODE_TURN_TICK:
		game->info->variable[VAR_TURN_COUNT]++;
		break;

	case OPCODE_PRINT:
		console_println(game, instr_lookup_string(game,
							  instr->operand[0],
							  instr->operand[1]));
		break;

	case OPCODE_TEST_NOT_ROOM_FLAG:
		func_set_test_result(func_state,
				     !(room->flags & instr->operand[0]));
		break;

	case OPCODE_TEST_ROOM_FLAG:
		func_set_test_result(func_state,
				     room->flags & instr->operand[0]);
		break;

	case OPCODE_NOT_IN_ROOM:
		func_set_test_result(func_state,
				     game->info->current_room != instr->operand[0]);
		break;

	case OPCODE_IN_ROOM:
		func_set_test_result(func_state,
				     game->info->current_room == instr->operand[0]);
		break;

	case OPCODE_MOVE_TO_ROOM:
		if (instr->operand[0] == 0xff) {
			/*
			 * FIXME - Not sure what this is for. Transylvania
			 * uses it in the 'go north' case when in room
			 * 0x01 or 0x0c, and Oo-Topos uses it when you shoot
			 * the alien. Ignore it for now.
			 */
			break;
		}

		move_to(game, instr->operand[0]);
		break;

	case OPCODE_MOVE:
		/* Move in the direction dictated by the current verb */
		if (verb->index - 1 >= NR_DIRECTIONS)
			fatal_error("Bad verb %d:%d in move",
				    verb->index, verb->type);

		if (room->direction[verb->index - 1])
			move_to(game, room->direction[verb->index - 1]);
		else
			console_println(game, string_lookup(game, STRING_CANT_GO));
		break;

	case OPCODE_MOVE_DIRECTION:
		if (room->direction[instr->operand[0] - 1])
			move_to(game, room->direction[instr->operand[0] - 1]);
		else
			console_println(game, string_lookup(game, STRING_CANT_GO));
		break;

	case OPCODE_ELSE:
		func_state->test_result = func_state->else_result;
		break;

	case OPCODE_MOVE_OBJECT_TO_CURRENT_ROOM:
		item = get_item(game, instr->operand[0] - 1);
		move_object(game, item, game->info->current_room);
		break;

	case OPCODE_OBJECT_IN_ROOM:
		item = get_item(game, instr->operand[0] - 1);
		func_set_test_result(func_state,
				     item->room == instr->operand[1]);
		break;

	case OPCODE_OBJECT_NOT_IN_ROOM:
		item = get_item(game, instr->operand[0] - 1);
		func_set_test_result(func_state,
				     item->room != instr->operand[1]);
		break;

	case OPCODE_MOVE_OBJECT_TO_ROOM:
		item = get_item(game, instr->operand[0] - 1);
		move_object(game, item, instr->operand[1]);
		break;

	case OPCODE_INVENTORY_FULL:
		item = get_item_by_noun(game, noun);
		func_set_test_result(func_state,
				     game->info->variable[VAR_INVENTORY_WEIGHT] +
				     (item->flags & ITEMF_WEIGHT_MASK) >
				     game->info->variable[VAR_INVENTORY_LIMIT]);
		break;

	case OPCODE_DESCRIBE_CURRENT_OBJECT:
		/*
		 * This opcode is only used in version 2
		 * FIXME - unsure what the single operand is for.
		 */
		item = get_item_by_noun(game, noun);
		printf("%s\n", string_lookup(game, item->long_string));
		break;

	case OPCODE_CURRENT_OBJECT_IN_ROOM:
		/* FIXME - use common code for these two ops */
		test = false;

		if (noun) {
			for (i = 0; i < game->info->header.nr_items; i++) {
				struct item *item = &game->info->item[i];

				if (item->word == noun->index &&
				    item->room == instr->operand[0]) {
					test = true;
					break;
				}
			}
		}

		func_set_test_result(func_state, test);
		break;

	case OPCODE_CURRENT_OBJECT_NOT_PRESENT:
		/* FIXME - use common code for these two ops */
		item = get_item_by_noun(game, noun);
		if (item)
			func_set_test_result(func_state,
					     item->room != game->info->current_room);
		else
			func_set_test_result(func_state, true);
		break;

	case OPCODE_CURRENT_OBJECT_PRESENT:
		item = get_item_by_noun(game, noun);
		if (item)
			func_set_test_result(func_state,
					     item->room == game->info->current_room);
		else
			func_set_test_result(func_state, false);
		break;

	case OPCODE_HAVE_OBJECT:
		item = get_item(game, instr->operand[0] - 1);
		func_set_test_result(func_state,
				     item->room == ROOM_INVENTORY);
		break;

	case OPCODE_NOT_HAVE_CURRENT_OBJECT:
		item = get_item_by_noun(game, noun);
		func_set_test_result(func_state,
				     !item || item->room != ROOM_INVENTORY);
		break;

	case OPCODE_HAVE_CURRENT_OBJECT:
		item = get_item_by_noun(game, noun);
		func_set_test_result(func_state,
				     item->room == ROOM_INVENTORY);
		break;

	case OPCODE_NOT_HAVE_OBJECT:
		item = get_item(game, instr->operand[0] - 1);
		func_set_test_result(func_state,
				     item->room != ROOM_INVENTORY);
		break;

	case OPCODE_CURRENT_OBJECT_TAKEABLE:
		item = get_item_by_noun(game, noun);
		if (!item)
			func_set_test_result(func_state, false);
		else
			func_set_test_result(func_state,
					     (item->flags & ITEMF_CAN_TAKE));
		break;

	case OPCODE_CURRENT_OBJECT_NOT_TAKEABLE:
		item = get_item_by_noun(game, noun);
		if (!item)
			func_set_test_result(func_state, true);
		else
			func_set_test_result(func_state,
					     !(item->flags & ITEMF_CAN_TAKE));
		break;

	case OPCODE_CURRENT_OBJECT_IS_NOWHERE:
		item = get_item_by_noun(game, noun);
		if (!item)
			func_set_test_result(func_state, false);
		else
			func_set_test_result(func_state,
					     item->room == ROOM_NOWHERE);
		break;

	case OPCODE_OBJECT_IS_NOWHERE:
		item = get_item(game, instr->operand[0] - 1);
		func_set_test_result(func_state,
				     item->room == ROOM_NOWHERE);
		break;

	case OPCODE_OBJECT_IS_NOT_NOWHERE:
		item = get_item(game, instr->operand[0] - 1);
		func_set_test_result(func_state,
				     item->room != ROOM_NOWHERE);
		break;

	case OPCODE_OBJECT_NOT_PRESENT:
		item = get_item(game, instr->operand[0] - 1);
		func_set_test_result(func_state,
				     item->room != game->info->current_room);
		break;

	case OPCODE_OBJECT_PRESENT:
		item = get_item(game, instr->operand[0] - 1);
		func_set_test_result(func_state,
				     item->room == game->info->current_room);
		break;

	case OPCODE_OBJECT_NOT_VALID:
		/* FIXME - should be called OPCODE_CURRENT_OBJECT_NOT_VALID */
		func_set_test_result(func_state, !noun ||
				     (noun->type & WORD_TYPE_NOUN_MASK) == 0);
		break;

	case OPCODE_CURRENT_IS_OBJECT:
		func_set_test_result(func_state,
				     get_item_by_noun(game, noun) != NULL);
		break;

	case OPCODE_CURRENT_NOT_OBJECT:
		func_set_test_result(func_state,
				     get_item_by_noun(game, noun) == NULL);
		break;

	case OPCODE_REMOVE_OBJECT:
		item = get_item(game, instr->operand[0] - 1);
		move_object(game, item, ROOM_NOWHERE);
		break;

	case OPCODE_REMOVE_CURRENT_OBJECT:
		item = get_item_by_noun(game, noun);
		move_object(game, item, ROOM_NOWHERE);
		break;

	case OPCODE_INVENTORY:
		count = num_objects_in_room(game, ROOM_INVENTORY);
		if (count == 0) {
			console_println(game, string_lookup(game, STRING_INVENTORY_EMPTY));
			break;
		}

		console_println(game, string_lookup(game, STRING_INVENTORY));
		for (i = 0; i < game->info->header.nr_items; i++) {
			item = &game->info->item[i];
			if (item->room == ROOM_INVENTORY)
				printf("%s\n",
				       string_lookup(game, item->string_desc));
		}
		break;

	case OPCODE_INVENTORY_ROOM:
		count = num_objects_in_room(game, instr->operand[0]);
		if (count == 0) {
			console_println(game, string_lookup(game, instr->operand[1] + 1));
			break;
		}

		console_println(game, string_lookup(game, instr->operand[1]));
		for (i = 0; i < game->info->header.nr_items; i++) {
			item = &game->info->item[i];
			if (item->room == instr->operand[0])
				printf("%s\n",
				       string_lookup(game, item->string_desc));
		}
		break;

	case OPCODE_MOVE_CURRENT_OBJECT_TO_ROOM:
		item = get_item_by_noun(game, noun);
		if (!item)
			fatal_error("Bad current object\n");

		move_object(game, item, instr->operand[0]);
		break;

	case OPCODE_DROP_OBJECT:
		item = get_item(game, instr->operand[0] - 1);
		move_object(game, item, game->info->current_room);
		break;

	case OPCODE_DROP_CURRENT_OBJECT:
		item = get_item_by_noun(game, noun);
		if (!item)
			fatal_error("Attempt to take object failed\n");

		move_object(game, item, game->info->current_room);
		break;

	case OPCODE_TAKE_CURRENT_OBJECT:
		item = get_item_by_noun(game, noun);
		if (!item)
			fatal_error("Attempt to take object failed\n");

		move_object(game, item, ROOM_INVENTORY);
		break;

	case OPCODE_TAKE_OBJECT:
		item = get_item(game, instr->operand[0] - 1);
		move_object(game, item, ROOM_INVENTORY);
		break;

	case OPCODE_TEST_FLAG:
		func_set_test_result(func_state,
				     game->info->flags[instr->operand[0]]);
		break;

	case OPCODE_TEST_NOT_FLAG:
		func_set_test_result(func_state,
				     !game->info->flags[instr->operand[0]]);
		break;

	case OPCODE_CLEAR_FLAG:
		game->info->flags[instr->operand[0]] = false;
		break;

	case OPCODE_SET_FLAG:
		game->info->flags[instr->operand[0]] = true;
		break;

	case OPCODE_OR:
		if (func_state->or_count) {
			func_state->or_count += 2;
		} else {
			func_state->test_result = false;
			func_state->or_count += 3;
		}
		break;

	case OPCODE_SET_OBJECT_DESCRIPTION:
		item = get_item(game, instr->operand[0] - 1);
		item->string_desc = (instr->operand[2] << 8) | instr->operand[1];
		break;

	case OPCODE_SET_OBJECT_LONG_DESCRIPTION:
		item = get_item(game, instr->operand[0] - 1);
		item->long_string = (instr->operand[2] << 8) | instr->operand[1];
		break;

	case OPCODE_SET_ROOM_DESCRIPTION:
		room = get_room(game, instr->operand[0]);
		switch (instr->operand[2]) {
		case 0x80:
			room->string_desc = instr->operand[1];
			break;
		case 0x81:
			room->string_desc = instr->operand[1] + 0x100;
			break;
		case 0x82:
			room->string_desc = instr->operand[1] + 0x200;
			break;
		default:
			fatal_error("Bad string desc %.2x:%.2x\n",
				    instr->operand[1], instr->operand[2]);
			break;
		}
		break;

	case OPCODE_SET_OBJECT_GRAPHIC:
		item = get_item(game, instr->operand[0] - 1);
		item->graphic = instr->operand[1];
		if (item->room == game->info->current_room)
			game->info->update_flags |= UPDATE_GRAPHICS;
		break;

	case OPCODE_SET_ROOM_GRAPHIC:
		room = get_room(game, instr->operand[0]);
		room->graphic = instr->operand[1];
		if (instr->operand[0] == game->info->current_room)
			game->info->update_flags |= UPDATE_GRAPHICS;
		break;

	case OPCODE_CALL_FUNC:
		index = instr->operand[0];
		if (instr->operand[1] == 0x81)
			index += 256;
		if (index >= game->info->nr_functions)
			fatal_error("Bad function %.4x >= %.4x\n",
				    index, game->info->nr_functions);

		debug_printf(DEBUG_FUNCTIONS,
			     "Calling subfunction %.4x\n", index);
		eval_function(game, &game->info->functions[index], verb, noun);
		break;

	case OPCODE_TEST_FALSE:
		/*
		 * FIXME - not sure what this is for. In Transylvania
		 * it is opcode 0x50 and is used when attempting to
		 * take the bar in the cellar. If it returns true then
		 * the response is "there's none here".
		 */
		func_set_test_result(func_state, false);
		break;

	case OPCODE_SAVE_ACTION:
		/*
		 * FIXME - This saves the current verb and allows the next
		 * command to use just the noun. This is used to allow
		 * responses to ask the player what they meant, e.g:
		 *
		 *   > drop
		 *   I don't understand what you want to drop.
		 *   > gun
		 *   Okay.
		 */
		break;

	case OPCODE_SET_STRING_REPLACEMENT:
		game->info->current_replace_word = instr->operand[0] - 1;
		break;

	case OPCODE_SET_CURRENT_NOUN_STRING_REPLACEMENT:
		/*
		 * FIXME - Not sure what the operand is for,
		 * maybe capitalisation?
		 */
		if (noun && (noun->type & WORD_TYPE_NOUN_PLURAL))
			game->info->current_replace_word = 3;
		else if (noun && (noun->type & WORD_TYPE_FEMALE))
			game->info->current_replace_word = 0;
		else if (noun && (noun->type & WORD_TYPE_MALE))
			game->info->current_replace_word = 1;
		else
			game->info->current_replace_word = 2;
		break;

	case OPCODE_DRAW_ROOM:
		draw_location_image(&game->info->room_images,
				    instr->operand[0] - 1);
		break;

	case OPCODE_DRAW_OBJECT:
		draw_image(&game->info->item_images, instr->operand[0] - 1);
		break;

	case OPCODE_WAIT_KEY:
		console_get_key();
		break;

	case OPCODE_SPECIAL:
		/* Game specific opcode */
		if (game->ops->handle_special_opcode)
			game->ops->handle_special_opcode(game,
							 instr->operand[0]);
		break;

	default:
		if (instr->opcode & 0x80) {
			debug_printf(DEBUG_FUNCTIONS,
				     "Unhandled command opcode %.2x\n",
				     instr->opcode);
		} else {
			debug_printf(DEBUG_FUNCTIONS,
				     "Unhandled test opcode %.2x - returning false\n",
				     instr->opcode);
			func_set_test_result(func_state, false);
		}
		break;
	}
}

/*
 * Comprehend functions consist of test and command instructions (if the MSB
 * of the opcode is set then it is a command). Functions are parsed by
 * evaluating each test until a command instruction is encountered. If the
 * overall result of the tests was true then the command instructions are
 * executed until either a test instruction is found or the end of the function
 * is reached. Otherwise the commands instructions are skipped over and the
 * next test sequence (if there is one) is tried.
 */
void eval_function(struct comprehend_game *game, struct function *func,
		   struct word *verb, struct word *noun)
{
	struct function_state func_state = {
		.test_result = true
	};
	int i;

	func_state.else_result = true;
	func_state.executed = false;

	for (i = 0; i < func->nr_instructions; i++) {
		if (func_state.executed && !func->instructions[i].is_command) {
			/*
			 * At least one command has been executed and the
			 * current instruction is a test. Exit the function.
			 */
			break;
		}

		eval_instruction(game, &func_state, &func->instructions[i],
				 verb, noun);
	}
}

static void skip_whitespace(char **p)
{
	while (**p && isspace(**p))
		(*p)++;
}

static void skip_non_whitespace(char **p)
{
	while (**p && !isspace(**p) && **p != ',' && **p != '\n')
		(*p)++;
}

static void handle_debug_command(struct comprehend_game *game,
				 const char *line)
{
	int i;

	if (strncmp(line, "quit", 4) == 0) {
		exit(EXIT_SUCCESS);

	} else if (strncmp(line, "debug", 5) == 0) {
		if (debugging_enabled())
			debug_disable(DEBUG_ALL);
		else
			debug_enable(DEBUG_FUNCTIONS);
		printf("Debugging %s\n", debugging_enabled() ? "on" : "off");

	} else if (strncmp(line, "dump objects", 12) == 0) {
		dump_game_data(game, DUMP_ITEMS);

	} else if (strncmp(line, "dump rooms", 10) == 0) {
		dump_game_data(game, DUMP_ROOMS);

	} else if (strncmp(line, "dump state", 10) == 0) {
		printf("Current room: %.2x\n", game->info->current_room);
		printf("Carry weight %d/%d\n\n",
		       game->info->variable[VAR_INVENTORY_WEIGHT],
		       game->info->variable[VAR_INVENTORY_LIMIT]);

		printf("Flags:\n");
		for (i = 0; i < ARRAY_SIZE(game->info->flags); i++)
			printf("  [%.2x]: %d\n", i, game->info->flags[i]);
		printf("\n");

		printf("Variables:\n");
		for (i = 0; i < ARRAY_SIZE(game->info->variable); i++)
			printf("  [%.2x]: %5d (0x%.4x)\n",
			       i, game->info->variable[i],
			       game->info->variable[i]);
		printf("\n");
	}
}

static bool handle_sentence(struct comprehend_game *game,
			    struct sentence *sentence)
{
	struct function *func;
	struct action *action;
	int i, j;

	if (sentence->nr_words == 0)
		return false;

	/* Find a matching action */
	for (i = 0; i < game->info->nr_actions; i++) {
		action = &game->info->action[i];

		if (action->type == ACTION_VERB_OPT_NOUN &&
		    sentence->nr_words > action->nr_words + 1)
			continue;
		if (action->type != ACTION_VERB_OPT_NOUN &&
		    sentence->nr_words != action->nr_words)
			continue;

		/*
		 * If all words in a sentence match those for an action then
		 * run that action's function.
		 */
		for (j = 0; j < action->nr_words; j++) {
			if (sentence->words[j].index == action->word[j] &&
			    (sentence->words[j].type & action->word_type[j]))
				continue;

			/* Word didn't match */
			break;
		}
		if (j == action->nr_words)
		{
			/* Match */
			func = &game->info->functions[action->function];
			eval_function(game, func,
				      &sentence->words[0], &sentence->words[1]);
			return true;
		}
	}

	/* No matching action */
	console_println(game, string_lookup(game, STRING_DONT_UNDERSTAND));
	return false;
}

static void read_sentence(struct comprehend_game *game, char **line,
			  struct sentence *sentence)
{
	bool sentence_end = false;
	char *word_string, *p = *line;
	struct word_index *pair;
	struct word *word;
	int index;

	memset(sentence, 0, sizeof(*sentence));
	while (1) {
		skip_whitespace(&p);
		word_string = p;
		skip_non_whitespace(&p);

		if (*p == ',' || *p == '\n') {
			/* Sentence separator */
			*p++ = '\0';
			sentence_end = true;
		} else {
			if (*p == '\0')
				sentence_end = true;
			else
				*p++ = '\0';
		}

		/* Find the dictionary word for this */
		word = dict_find_word_by_string(game, word_string);
		if (!word)
			memset(&sentence->words[sentence->nr_words], 0,
			       sizeof(sentence->words[sentence->nr_words]));
		else
			memcpy(&sentence->words[sentence->nr_words],
			       word, sizeof(*word));

		sentence->nr_words++;

		if (sentence->nr_words > 1) {
			index = sentence->nr_words;

			/* See if this word and the previous are a word pair */
			pair = is_word_pair(game,
					    &sentence->words[index - 2],
					    &sentence->words[index - 1]);
			if (pair) {
				sentence->words[index - 2].index = pair->index;
				sentence->words[index - 2].type = pair->type;
				strcpy(sentence->words[index - 2].word,
				       "[PAIR]" );
				sentence->nr_words--;
			}
		}

		if (sentence->nr_words >= ARRAY_SIZE(sentence->words) ||
		    sentence_end)
			break;
	}

	*line = p;
}

static void before_turn(struct comprehend_game *game)
{
	/* Run the game specific before turn bits */
	if (game->ops->before_turn)
		game->ops->before_turn(game);

	/* Run the each turn functions */
	eval_function(game, &game->info->functions[0], NULL, NULL);

	update(game);
}

static void after_turn(struct comprehend_game *game)
{
	/* Do post turn game specific bits */
	if (game->ops->after_turn)
		game->ops->after_turn(game);
}

static void read_input(struct comprehend_game *game)
{
	struct sentence sentence;
	char *line = NULL, buffer[1024];
	bool handled;

	if (game->ops->before_prompt)
		game->ops->before_prompt(game);
	before_turn(game);

	while (!line) {
		printf("> ");
		line = fgets(buffer, sizeof(buffer), stdin);
	}

	/* Re-comprehend special commands start with '!' */
	if (*line == '!') {
		handle_debug_command(game, &line[1]);
		return;
	}

	while (1) {
		read_sentence(game, &line, &sentence);
		handled = handle_sentence(game, &sentence);
		if (handled)
			after_turn(game);

		/* FIXME - handle the 'before you can continue' case */
		if (*line == '\0')
			break;
		line++;

		if (handled)
			before_turn(game);
	}
}

void comprehend_play_game(struct comprehend_game *game)
{
	console_init();

	if (game->ops->before_game)
		game->ops->before_game(game);

	game->info->update_flags = UPDATE_ALL;
	while (1)
		read_input(game);
}
