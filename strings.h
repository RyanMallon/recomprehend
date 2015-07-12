/*
 * This file is public domain
 *
 * Ryan Mallon, 2015
 *
 */

#ifndef _RECOMPREHEND_STRINGS_H
#define _RECOMPREHEND_STRINGS_H

#include <stdint.h>

struct comprehend_game;

const char *string_lookup(struct comprehend_game *game, uint16_t index);
const char *instr_lookup_string(struct comprehend_game *game, uint8_t index,
				uint8_t table);

#endif /* _RECOMPREHEND_STRINGS_H */
