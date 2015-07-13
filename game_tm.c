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

#include "recomprehend.h"
#include "game_data.h"
#include "game.h"

static struct game_ops tm_ops = {
};

/* FIXME - This is broken */
struct comprehend_game game_talisman = {
	.game_name		= "Talisman, Challenging the Sands of Time (broken)",
	.short_name		= "tm",
	.game_data_file		= "G0",
	.location_graphic_files	= {"RA", "RB", "RC", "RD", "RE", "RF", "RG"},
	.item_graphic_files	= {"OA", "OB", "OE", "OF"},

	.ops			= &tm_ops,
};


