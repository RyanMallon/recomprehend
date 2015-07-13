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

#ifndef _RECOMPREHEND_OPCODE_MAP_H
#define _RECOMPREHEND_OPCODE_MAP_H

#include <stdint.h>

struct comprehend_game;

uint8_t *get_opcode_map(struct comprehend_game *game);

#endif /* _RECOMPREHEND_OPCODE_MAP_H */
