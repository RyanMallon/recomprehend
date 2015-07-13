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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "recomprehend.h"
#include "game_data.h"
#include "dictionary.h"

static bool word_match(struct word *word, const char *string)
{
	/* Words less than 6 characters must match exactly */
	if (strlen(word->word) < 6 && strlen(string) != strlen(word->word))
		return false;

	return strncmp(word->word, string, strlen(word->word)) == 0;
}

struct word *dict_find_word_by_string(struct comprehend_game *game,
				      const char *string)
{
	int i;

	if (!string)
		return NULL;

	for (i = 0; i < game->info->nr_words; i++)
		if (word_match(&game->info->words[i], string))
			return &game->info->words[i];

	return NULL;
}

struct word *dict_find_word_by_index_type(struct comprehend_game *game,
					  uint8_t index, uint8_t type)
{
	int i;

	for (i = 0; i < game->info->nr_words; i++) {
		if (game->info->words[i].index == index &&
		    game->info->words[i].type == type)
			return &game->info->words[i];
	}

	return NULL;
}

struct word *find_dict_word_by_index(struct comprehend_game *game,
				     uint8_t index, uint8_t type_mask)
{
	int i;

	for (i = 0; i < game->info->nr_words; i++) {
		if (game->info->words[i].index == index &&
		    (game->info->words[i].type & type_mask) != 0)
			return &game->info->words[i];
	}

	return NULL;
}

bool dict_match_index_type(struct comprehend_game *game, const char *word,
			   uint8_t index, uint8_t type_mask)
{
	int i;

	for (i = 0; i < game->info->nr_words; i++)
		if (game->info->words[i].index == index &&
		    ((game->info->words[i].type & type_mask) != 0) &&
		    word_match(&game->info->words[i], word))
			return true;

	return false;
}
