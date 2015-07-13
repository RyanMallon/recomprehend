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

#ifndef _RECOMPREHEND_GAME_H
#define _RECOMPREHEND_GAME_H

#include <stdbool.h>
#include <stdint.h>

struct comprehend_game;
struct function;
struct item;
struct word;

void console_println(struct comprehend_game *game, const char *text);
int console_get_key(void);

struct item *get_item(struct comprehend_game *game, uint16_t index);
void move_object(struct comprehend_game *game, struct item *item, int new_room);
void eval_function(struct comprehend_game *game, struct function *func,
		   struct word *verb, struct word *noun);

void comprehend_play_game(struct comprehend_game *game);
void game_save(struct comprehend_game *game);
void game_restore(struct comprehend_game *game);
void game_restart(struct comprehend_game *game);

#endif /* _RECOMPREHEND_GAME_H */
