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
#include <stdlib.h>

#include "recomprehend.h"
#include "game_data.h"
#include "game.h"

static void cc_clear_companion_flags(struct comprehend_game *game)
{
	/* Clear the Sabrina/Erik action flags */
	game->info->flags[0xa] = 0;
	game->info->flags[0xb] = 0;
}

static bool cc_common_handle_special_opcode(struct comprehend_game *game,
					    uint8_t operand)
{
	switch (operand) {
	case 0x03:
		/*
		 * Game over - failure.
		 *
		 * FIXME - If playing the second disk this should restart
		 *         from the beginning of the first disk.
		 */
		game_restart(game);
		break;

	case 0x06:
		game_save(game);
		break;

	case 0x07:
		/*
		 * FIXME - This will only correctly restore games that were
		 *         saved for the disk currently being played.
		 */
		game_restore(game);
		return true;
	}

	return false;
}

static void cc1_handle_special_opcode(struct comprehend_game *game,
				      uint8_t operand)
{
	if (cc_common_handle_special_opcode(game, operand))
		return;

	switch (operand) {
	case 0x05:
		/*
		 * Completed first part (disk 1) of the game.
		 *
		 * FIXME - This should automatically load disk 2.
		 */
		console_println(game, "[Completed disk 1 - to continue run Re-Comprehend with the 'cc2' game]");
		exit(EXIT_SUCCESS);
		break;
	}
}

static void cc2_handle_special_opcode(struct comprehend_game *game,
				      uint8_t operand)
{
	if (cc_common_handle_special_opcode(game, operand))
		return;

	switch (operand) {
	case 0x01:
		/* Enter the Vampire's throne room */
		eval_function(game, &game->info->functions[0xe], NULL, NULL);
		break;

	case 0x05:
		/*
		 * Won the game.
		 *
		 * FIXME - The merchant ship should arrives, etc.
		 */
		game_restart(game);
		break;
	}
}

static void cc2_before_prompt(struct comprehend_game *game)
{
	cc_clear_companion_flags(game);
}

static void cc1_before_prompt(struct comprehend_game *game)
{
	cc_clear_companion_flags(game);
}

static struct game_strings cc1_strings = {
	.game_restart		= 0x9,
};

static struct game_ops cc1_ops = {
	.before_prompt		= cc1_before_prompt,
	.handle_special_opcode	= cc1_handle_special_opcode,
};

static struct game_ops cc2_ops = {
	.before_prompt		= cc2_before_prompt,
	.handle_special_opcode	= cc2_handle_special_opcode,
};

struct comprehend_game game_crimson_crown_1 = {
	.game_name		= "Crimson Crown (Part 1/2)",
	.short_name		= "cc1",
	.game_data_file		= "CC1.GDA",
	.string_files		= {
		{"MA.MS1", 0x89},
	},
	.location_graphic_files	= {"RA.MS1", "RB.MS1", "RC.MS1"},
	.item_graphic_files	= {"OA.MS1", "OB.MS1"},
	.save_game_file_fmt	= "G%d.MS0",

	.strings		= &cc1_strings,
	.ops			= &cc1_ops,
};

struct comprehend_game game_crimson_crown_2 = {
	.game_name		= "Crimson Crown (Part 2/2)",
	.short_name		= "cc2",
	.game_data_file		= "CC2.GDA",
	.string_files		= {
		{"MA.MS2", 0x89},
	},
	.location_graphic_files	= {"RA.MS2", "RB.MS2"},
	.item_graphic_files	= {"OA.MS2", "OB.MS2"},
	.save_game_file_fmt	= "G%d.MS0",

	.ops			= &cc2_ops,
};
