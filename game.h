/*
 * This file is public domain
 *
 * Ryan Mallon, 2015
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
