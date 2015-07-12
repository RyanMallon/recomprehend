/*
 * This file is public domain
 *
 * Ryan Mallon, 2015
 *
 */

#ifndef _RECOMPREHEND_DUMP_GAME_DATA_H
#define _RECOMPREHEND_DUMP_GAME_DATA_H

struct comprehend_game;
struct function_state;
struct instruction;

#define DUMP_STRINGS		(1 << 0)
#define DUMP_EXTRA_STRINGS	(1 << 1)
#define DUMP_ROOMS		(1 << 2)
#define DUMP_ITEMS		(1 << 3)
#define DUMP_DICTIONARY		(1 << 4)
#define DUMP_WORD_PAIRS		(1 << 5)
#define DUMP_ACTIONS		(1 << 6)
#define DUMP_FUNCTIONS		(1 << 7)
#define DUMP_REPLACE_WORDS	(1 << 8)
#define DUMP_HEADER		(1 << 9)
#define DUMP_ALL		(~0)

void dump_instruction(struct comprehend_game *game,
		      struct function_state *func_state,
		      struct instruction *instr);
void dump_game_data(struct comprehend_game *game, unsigned flags);

#endif /* _RECOMPREHEND_DUMP_GAME_DATA_H */
