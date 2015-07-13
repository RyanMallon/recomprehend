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

#ifndef _RECOMPREHEND_DICTIONARY_H
#define _RECOMPREHEND_DICTIONARY_H

#include <stdbool.h>
#include <stdint.h>

struct comprehend_game;
struct word;

struct word *find_dict_word_by_index(struct comprehend_game *game,
				     uint8_t index, uint8_t type_mask);
struct word *dict_find_word_by_index_type(struct comprehend_game *game,
					  uint8_t index, uint8_t type);
struct word *dict_find_word_by_string(struct comprehend_game *game,
				      const char *string);
bool dict_match_index_type(struct comprehend_game *game, const char *word,
			   uint8_t index, uint8_t type_mask);

#endif /* _RECOMPREHEND_DICTIONARY_H */
