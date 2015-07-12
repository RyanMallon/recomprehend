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

#include "image_data.h"
#include "graphics.h"
#include "util.h"

static void usage(const char *progname)
{
	printf("%s: [OPTION]... FILENAME INDEX\n", progname);
	printf("\nOptions:\n");
	printf("  -w, --width=WIDTH       Graphics width\n");
	printf("  -h, --height=HEIGHT     Graphics height\n");
	printf("  -c, --clear=COLOR       Graphics clear color\n");
	printf("  -t, --color-table=INDEX Color table\n");
	printf("  -s, --sequence          Disable sequence of images\n");
	printf("  -p, --pause             Wait for keypress after each draw operation\n");
	printf("  -f, --floodfill-disable Disable floodfill operation\n");
	printf("  -d, --debug             Enable debugging\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	struct option long_opts[] = {
		{"width",		required_argument,	0, 'w'},
		{"height",		required_argument,	0, 'h'},
		{"clear",		required_argument,	0, 'c'},
		{"color-table",		required_argument,	0, 't'},
		{"sequence",		no_argument,		0, 's'},
		{"pause",		no_argument,		0, 'p'},
		{"floodfill-disable",	no_argument,		0, 'f'},
		{"debug",		no_argument,		0, 'd'},
		{"help",		no_argument,		0, '?'},
		{NULL,			0,			0, 0},
	};
	const char *short_opts = "w:h:c:t:spfd?";
	struct image_data info;
	const char *filename;
	unsigned index, clear_color = G_COLOR_WHITE,
		graphics_width = G_RENDER_WIDTH,
		graphics_height = G_RENDER_HEIGHT,
		color_table = 0;
	bool sequence = false;
	int c, opt_index;

	while (1) {
		c = getopt_long(argc, argv, short_opts, long_opts, &opt_index);
		if (c == -1)
			break;

		switch (c) {
		case 'w':
			graphics_width = strtoul(optarg, NULL, 0);
			break;

		case 'h':
			graphics_height = strtoul(optarg, NULL, 0);
			break;

		case 'c':
			clear_color = strtoul(optarg, NULL, 0);
			break;

		case 't':
			color_table = strtoul(optarg, NULL, 0);
			break;

		case 's':
			sequence = true;
			break;

		case 'p':
			image_set_draw_flags(IMAGEF_OP_WAIT_KEYPRESS);
			break;

		case 'f':
			image_set_draw_flags(IMAGEF_NO_FLOODFILL);
			break;

		case 'd':
			debug_enable(DEBUG_IMAGE_DRAW);
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

	filename = argv[optind++];
	index = strtoul(argv[optind++], NULL, 0);

	g_init(graphics_width, graphics_height);
	g_set_color_table(color_table);
	comprehend_load_image_file(filename, &info);

	while (index < 16) {
		g_clear_screen(clear_color);
		draw_image(&info, index);

		c = getchar();
		if (!sequence || c == 'q' || c == 'Q')
			break;

		index++;
		printf("Image %d\n", index);
	}

	exit(EXIT_SUCCESS);
}
