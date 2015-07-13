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

#include <stdint.h>

#include "recomprehend.h"
#include "game_data.h"
#include "strings.h"

static char bad_string[128];

const char *string_lookup(struct comprehend_game *game, uint16_t index)
{
	uint16_t string;
	uint8_t table;

	/*
	 * There are two tables of strings. The first is stored in the main
	 * game data file, and the second is stored in multiple string files.
	 *
	 * In instructions string indexes are split into a table and index
	 * value. In other places such as the save files strings from the
	 * main table are occasionally just a straight 16-bit index. We
	 * convert all string indexes to the former case so that we can handle
	 * them the same everywhere.
	 */
	table = (index >> 8) & 0xff;
	string = index & 0xff;

	switch (table) {
	case 0x81:
	case 0x01:
		string += 0x100;
		/* Fall-through */
	case 0x00:
	case 0x80:
		if (string < game->info->strings.nr_strings)
			return game->info->strings.strings[string];
		break;

	case 0x83:
		string += 0x100;
		/* Fall-through */
	case 0x02:
	case 0x82:
		if (string < game->info->strings2.nr_strings)
			return game->info->strings2.strings[string];
		break;
	}

	snprintf(bad_string, sizeof(bad_string), "BAD_STRING(%.4x)", index);
	return bad_string;
}

const char *instr_lookup_string(struct comprehend_game *game, uint8_t index,
				uint8_t table)
{
	return string_lookup(game, table << 8 | index);
}
