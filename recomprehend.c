/*
 * This file is public domain
 *
 * Ryan Mallon, 2015
 *
 */

#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "recomprehend.h"
#include "dump_game_data.h"
#include "game_data.h"
#include "graphics.h"
#include "game.h"
#include "util.h"

extern struct comprehend_game game_transylvania;
extern struct comprehend_game game_crimson_crown_1;
extern struct comprehend_game game_crimson_crown_2;
extern struct comprehend_game game_oo_topos;
extern struct comprehend_game game_talisman;

static struct comprehend_game *comprehend_games[] = {
	&game_transylvania,
	&game_crimson_crown_1,
	&game_crimson_crown_2,
	&game_oo_topos,
	&game_talisman,
};

struct dump_option {
	const char	*option;
	unsigned	flag;
};

static struct dump_option dump_options[] = {
	{"strings",		DUMP_STRINGS},
	{"extra-strings",	DUMP_EXTRA_STRINGS},
	{"rooms",		DUMP_ROOMS},
	{"items",		DUMP_ITEMS},
	{"dictionary",		DUMP_DICTIONARY},
	{"word-pairs",		DUMP_WORD_PAIRS},
	{"actions",		DUMP_ACTIONS},
	{"functions",		DUMP_FUNCTIONS},
	{"replace-words",	DUMP_REPLACE_WORDS},
	{"header",		DUMP_HEADER},
	{"all",			DUMP_ALL},
};

static void usage(const char *progname)
{
	int i;

	printf("Usage %s [OPTION]... GAME_NAME GAME_DIR\n", progname);
	printf("\nOptions:\n");
	printf("  -d, --debug                   Enable debugging\n");
	printf("  -D, --dump=OPTION             Dump game data\n");
	for (i = 0; i < ARRAY_SIZE(dump_options); i++)
		printf("        %s\n", dump_options[i].option);
	printf("  -p, --no-play                 Don't run the interpreter\n");
	printf("  -g, --no-graphics             Disable graphics\n");
	printf("  -f, --no-floodfill            Disable floodfill\n");
	printf("  -w, --graphics-width=WIDTH    Graphics width\n");
	printf("  -h, --graphics-height=HEIGHT  Graphics height\n");

	printf("\nSupported games:\n");
	for (i = 0; i < ARRAY_SIZE(comprehend_games); i++)
		printf("    %-10s %s\n", comprehend_games[i]->short_name,
		       comprehend_games[i]->game_name);

	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	struct option long_opts[] = {
		{"debug",		no_argument,		0, 'd'},
		{"dump",		required_argument,	0, 'D'},
		{"no-play",		no_argument,		0, 'p'},
		{"no-graphics",		no_argument,		0, 'g'},
		{"no-floodfill",	no_argument,		0, 'f'},
		{"graphics-width",	required_argument,	0, 'w'},
		{"graphics-height",	required_argument,	0, 'h'},
		{"help",		no_argument,		0, '?'},
		{NULL,			0,			0, 0},
	};
	const char *short_opts = "dD:pgfw:h:?";
	struct comprehend_game *game;
	const char *game_name, *game_dir;
	unsigned dump_flags = 0;
	int i, c, opt_index;
	unsigned graphics_width = G_RENDER_WIDTH,
		graphics_height = G_RENDER_HEIGHT;
	bool play_game = true, graphics_enabled = true;

	while (1) {
		c = getopt_long(argc, argv, short_opts, long_opts, &opt_index);
		if (c == -1)
			break;

		switch (c) {
		case 'd':
			// FIXME
			debug_enable(DEBUG_FUNCTIONS);
			break;

		case 'D':
			for (i = 0; i < ARRAY_SIZE(dump_options); i++)
				if (strcmp(optarg, dump_options[i].option) == 0)
					break;
			if (i == ARRAY_SIZE(dump_options)) {
				printf("Invalid dump option '%s'\n", optarg);
				usage(argv[0]);
			}

			dump_flags |= dump_options[i].flag;
			break;

		case 'p':
			play_game = false;
			break;

		case 'g':
			graphics_enabled = false;
			break;

		case 'f':
			image_set_draw_flags(IMAGEF_NO_FLOODFILL);
			break;

		case 'w':
			graphics_width = strtoul(optarg, NULL, 0);
			break;

		case 'h':
			graphics_height = strtoul(optarg, NULL, 0);
			break;

		case '?':
			usage(argv[0]);
			break;

		default:
			printf("Invalid option\n");
			usage(argv[0]);
			break;
		}
	}

	if (optind >= argc || argc - optind != 2)
		usage(argv[0]);

	game_name = argv[optind++];
	game_dir = argv[optind++];

	/* Lookup game */
	game = NULL;
	for (i = 0; i < ARRAY_SIZE(comprehend_games); i++) {
		if (strcmp(game_name, comprehend_games[i]->short_name) == 0) {
			game = comprehend_games[i];
			break;
		}
	}
	if (!game) {
		printf("Unknown game '%s'\n", game_name);
		usage(argv[0]);
	}

	if (graphics_enabled)
		g_init(graphics_width, graphics_height);

	game->info = xmalloc(sizeof(*game->info));
	comprehend_load_game(game, game_dir);

	if (dump_flags)
		dump_game_data(game, dump_flags);

	if (play_game)
		comprehend_play_game(game);

	exit(EXIT_SUCCESS);
}
