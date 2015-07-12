/*
 * This file is public domain
 *
 * Ryan Mallon, 2015
 *
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "recomprehend.h"
#include "game_data.h"
#include "game.h"
#include "util.h"

struct tr_monster {
	uint8_t		object;
	uint8_t		dead_flag;
	unsigned	min_turns_before;
	unsigned	room_allow_flag;
	unsigned	randomness;
};

static struct tr_monster tr_werewolf = {
	.object			= 0x21,
	.dead_flag		= 7,
	.room_allow_flag	= (1 << 6),
	.min_turns_before	= 5,
	.randomness		= 5,
};

static struct tr_monster tr_vampire = {
	.object			= 0x26,
	.dead_flag		= 5,
	.room_allow_flag	= (1 << 7),
	.min_turns_before	= 0,
	.randomness		= 5,
};

static void tr_update_monster(struct comprehend_game *game,
			      struct tr_monster *monster_info)
{
	struct item *monster;
	struct room *room;
	uint16_t turn_count;

	room = &game->info->rooms[game->info->current_room];
	turn_count = game->info->variable[VAR_TURN_COUNT];

	monster = get_item(game, monster_info->object);
	if (monster->room == game->info->current_room) {
		/* The monster is in the current room - leave it there */
		return;
	}

	if ((room->flags & monster_info->room_allow_flag) &&
	    !game->info->flags[monster_info->dead_flag] &&
	    turn_count > monster_info->min_turns_before) {
		/*
		 * The monster is alive and allowed to move to the current
		 * room. Randomly decide whether on not to. If not, move
		 * it back to limbo.
		 */
		if ((rand() % monster_info->randomness) == 0) {
			move_object(game, monster, game->info->current_room);
			game->info->variable[0xf] = turn_count + 1;
		} else {
			move_object(game, monster, ROOM_NOWHERE);
		}
	}
}

static int tr_room_is_special(struct comprehend_game *game, unsigned room_index,
			      unsigned *room_desc_string)
{
	struct room *room = &game->info->rooms[room_index];

	if (room_index == 0x28) {
		if (room_desc_string)
			*room_desc_string = room->string_desc;
		return ROOM_IS_DARK;
	}

	return ROOM_IS_NORMAL;
}

static bool tr_before_turn(struct comprehend_game *game)
{
	tr_update_monster(game, &tr_werewolf);
	tr_update_monster(game, &tr_vampire);
	return false;
}

static void tr_handle_special_opcode(struct comprehend_game *game,
				     uint8_t operand)
{
	switch (operand) {
	case 0x01:
		/*
		 * FIXME - Called when the mice are dropped and the cat chases
		 *         them.
		 */
		break;

	case 0x02:
		/* FIXME - Called when the gun is fired */
		break;

	case 0x06:
		game_save(game);
		break;

	case 0x07:
		game_restore(game);
		break;

	case 0x03:
		/* Game over - failure */
	case 0x05:
		/* Won the game */
	case 0x08:
		/* Restart game */
		game_restart(game);
		break;

	case 0x09:
		/*
		 * Show the Zin screen in reponse to doing 'sing some enchanted
		 * evening' in his cabin.
		 */
		draw_location_image(&game->info->room_images, 41);
		console_get_key();
		game->info->update_flags |= UPDATE_GRAPHICS;
		break;
	}
}

static void read_string(char *buffer, size_t size)
{
	char *p;

	printf("> ");
	fgets(buffer, size, stdin);

	/* Remove trailing newline */
	p = strchr(buffer, '\n');
	if (p)
		*p = '\0';

}

static void tr_before_game(struct comprehend_game *game)
{
	char buffer[128];

	/* Welcome to Transylvania - sign your name */
	console_println(game, game->info->strings.strings[0x20]);
	read_string(buffer, sizeof(buffer));

	/*
	 * Transylvania uses replace word 0 as the player's name, the game
	 * data file stores a bunch of dummy characters, so the length is
	 * limited (the original game will break if you put a name in that
	 * is too long).
	 */
	if (!game->info->replace_words[0])
		game->info->replace_words[0] = xstrndup(buffer, strlen(buffer));
	else
		snprintf(game->info->replace_words[0],
			 strlen(game->info->replace_words[0]),
			 "%s", buffer);

	/* And your next of kin - This isn't store by the game */
	console_println(game, game->info->strings.strings[0x21]);
	read_string(buffer, sizeof(buffer));
}

static struct game_strings tr_strings = {
	.game_restart		= EXTRA_STRING_TABLE(0x8a),
};

static struct game_ops tr_ops = {
	.before_game		= tr_before_game,
	.before_turn		= tr_before_turn,
	.room_is_special	= tr_room_is_special,
	.handle_special_opcode	= tr_handle_special_opcode,
};

struct comprehend_game game_transylvania = {
	.game_name		= "Transylvania",
	.short_name		= "tr",
	.game_data_file		= "TR.GDA",
	.string_files		= {
		{"MA.MS1", 0x88},
		{"MB.MS1", 0x88},
		{"MC.MS1", 0x88},
		{"MD.MS1", 0x88},
		{"ME.MS1", 0x88},
	},
	.location_graphic_files	= {"RA.MS1", "RB.MS1", "RC.MS1"},
	.item_graphic_files	= {"OA.MS1", "OB.MS1", "OC.MS1"},
	.save_game_file_fmt	= "G%d.MS0",

	.strings		= &tr_strings,
	.ops			= &tr_ops,
};
