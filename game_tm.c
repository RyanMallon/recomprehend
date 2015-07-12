/*
 * This file is public domain
 *
 * Ryan Mallon, 2015
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


