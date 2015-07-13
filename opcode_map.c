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

#include <stdint.h>

#include "recomprehend.h"
#include "game_data.h"
#include "util.h"

/*
 * Version 2 of the Comprehend engine (OO-Topos) changes some of the opcode
 * numbers and adds new opcodes. Use tables to translate the opcodes used
 * in the original games into a generic version used by Re-Comprehend.
 *
 * FIXME - unimplemented/unknown ocpodes:
 *
 * d5(obj): Make object visible. This will print a "you see: object" when the
 *          object is in the room.
 */
static uint8_t opcode_map_v1[0x100] = {
	[0x01] = OPCODE_HAVE_OBJECT,
	[0x04] = OPCODE_OR,
	[0x05] = OPCODE_IN_ROOM,
	[0x06] = OPCODE_VAR_EQ,
	[0x08] = OPCODE_CURRENT_OBJECT_TAKEABLE,
	[0x09] = OPCODE_OBJECT_PRESENT,
	[0x0c] = OPCODE_ELSE,
	[0x0e] = OPCODE_OBJECT_IN_ROOM,
	[0x14] = OPCODE_OBJECT_NOT_VALID,
	[0x18] = OPCODE_INVENTORY_FULL,
	[0x19] = OPCODE_TEST_FLAG,
	[0x1d] = OPCODE_CURRENT_OBJECT_IN_ROOM,
	[0x20] = OPCODE_HAVE_CURRENT_OBJECT,
	[0x21] = OPCODE_OBJECT_IS_NOT_NOWHERE,
	[0x24] = OPCODE_CURRENT_OBJECT_PRESENT,
	[0x31] = OPCODE_TEST_ROOM_FLAG,
	[0x41] = OPCODE_NOT_HAVE_OBJECT,
	[0x45] = OPCODE_NOT_IN_ROOM,
	[0x48] = OPCODE_CURRENT_OBJECT_IS_NOWHERE,
	[0x49] = OPCODE_OBJECT_NOT_PRESENT,
	[0x43] = OPCODE_OBJECT_NOT_IN_ROOM,
	[0x50] = OPCODE_TEST_FALSE,
	[0x59] = OPCODE_TEST_NOT_FLAG,
	[0x60] = OPCODE_NOT_HAVE_CURRENT_OBJECT,
	[0x61] = OPCODE_OBJECT_IS_NOWHERE,
	[0x64] = OPCODE_CURRENT_OBJECT_NOT_PRESENT,
	[0x68] = OPCODE_CURRENT_OBJECT_NOT_TAKEABLE,
	[0x71] = OPCODE_TEST_NOT_ROOM_FLAG,
	[0x80] = OPCODE_INVENTORY,
	[0x81] = OPCODE_TAKE_OBJECT,
	[0x82] = OPCODE_MOVE_OBJECT_TO_ROOM,
	[0x84] = OPCODE_SAVE_ACTION,
	[0x85] = OPCODE_MOVE_TO_ROOM,
	[0x86] = OPCODE_VAR_ADD,
	[0x87] = OPCODE_SET_ROOM_DESCRIPTION,
	[0x89] = OPCODE_MOVE_OBJECT_TO_CURRENT_ROOM,
	[0x8a] = OPCODE_VAR_SUB,
	[0x8b] = OPCODE_SET_OBJECT_DESCRIPTION,
	[0x8c] = OPCODE_MOVE,
	[0x8e] = OPCODE_PRINT,
	[0x95] = OPCODE_REMOVE_OBJECT,
	[0x99] = OPCODE_SET_FLAG,
	[0x92] = OPCODE_CALL_FUNC,
	[0x98] = OPCODE_TURN_TICK,
	[0x9d] = OPCODE_CLEAR_FLAG,
	[0x9e] = OPCODE_INVENTORY_ROOM,
	[0xa0] = OPCODE_TAKE_CURRENT_OBJECT,
	[0xa1] = OPCODE_SPECIAL,
	[0xa4] = OPCODE_DROP_CURRENT_OBJECT,
	[0xa2] = OPCODE_SET_ROOM_GRAPHIC,
	[0xb0] = OPCODE_REMOVE_CURRENT_OBJECT,
	[0xb1] = OPCODE_DO_VERB,
	[0xb9] = OPCODE_SET_STRING_REPLACEMENT,
	[0xbd] = OPCODE_VAR_INC,
	[0xc1] = OPCODE_VAR_DEC,
	[0xc9] = OPCODE_MOVE_CURRENT_OBJECT_TO_ROOM,
};

static uint8_t opcode_map_v2[0x100] = {
	[0x01] = OPCODE_HAVE_OBJECT,
	[0x04] = OPCODE_OR,
	[0x05] = OPCODE_IN_ROOM,
	[0x06] = OPCODE_VAR_EQ,
	[0x08] = OPCODE_CURRENT_IS_OBJECT,
	[0x09] = OPCODE_OBJECT_PRESENT,
	[0x0c] = OPCODE_ELSE,
	[0x11] = OPCODE_OBJECT_IS_NOWHERE,
	[0x14] = OPCODE_OBJECT_NOT_VALID,
	[0x19] = OPCODE_TEST_FLAG,
	[0x1d] = OPCODE_TEST_ROOM_FLAG,
	[0x20] = OPCODE_HAVE_CURRENT_OBJECT,
	[0x21] = OPCODE_OBJECT_PRESENT,
	[0x22] = OPCODE_OBJECT_IN_ROOM,
	[0x30] = OPCODE_CURRENT_OBJECT_PRESENT,
	[0x31] = OPCODE_TEST_ROOM_FLAG,
	[0x38] = OPCODE_INVENTORY_FULL,
	[0x41] = OPCODE_NOT_HAVE_OBJECT,
	[0x45] = OPCODE_NOT_IN_ROOM,
	[0x48] = OPCODE_CURRENT_OBJECT_IS_NOWHERE,
	[0x43] = OPCODE_OBJECT_NOT_IN_ROOM,
	[0x59] = OPCODE_TEST_NOT_FLAG,
	[0x5d] = OPCODE_TEST_NOT_ROOM_FLAG,
	[0x60] = OPCODE_NOT_HAVE_CURRENT_OBJECT,
	[0x61] = OPCODE_OBJECT_NOT_PRESENT,
	[0x70] = OPCODE_CURRENT_OBJECT_NOT_PRESENT,
	[0x74] = OPCODE_CURRENT_NOT_OBJECT,
	[0x80] = OPCODE_INVENTORY,
	[0x81] = OPCODE_TAKE_OBJECT,
	[0x82] = OPCODE_MOVE_OBJECT_TO_ROOM,
	[0x84] = OPCODE_SAVE_ACTION,
	[0x85] = OPCODE_MOVE_TO_ROOM,
	[0x86] = OPCODE_VAR_ADD,
	[0x87] = OPCODE_SET_ROOM_DESCRIPTION,
	[0x89] = OPCODE_SPECIAL,
	[0x8a] = OPCODE_VAR_SUB,
	[0x8b] = OPCODE_SET_OBJECT_DESCRIPTION,
	[0x8c] = OPCODE_MOVE,
	[0x8e] = OPCODE_PRINT,
	[0x8f] = OPCODE_SET_OBJECT_LONG_DESCRIPTION,
	[0x90] = OPCODE_WAIT_KEY,
	[0x92] = OPCODE_CALL_FUNC,
	[0x95] = OPCODE_REMOVE_OBJECT,
	[0x98] = OPCODE_TURN_TICK,
	[0x99] = OPCODE_SET_FLAG,
	[0x9d] = OPCODE_CLEAR_FLAG,
	[0x9e] = OPCODE_INVENTORY_ROOM,
	[0xa0] = OPCODE_TAKE_CURRENT_OBJECT,
	[0xa2] = OPCODE_SET_OBJECT_GRAPHIC,
	[0xb1] = OPCODE_DO_VERB,
	[0xb5] = OPCODE_DESCRIBE_CURRENT_OBJECT,
	[0xc1] = OPCODE_VAR_DEC,
	[0xc2] = OPCODE_SET_ROOM_GRAPHIC,
	[0xc5] = OPCODE_SET_CURRENT_NOUN_STRING_REPLACEMENT,
	[0xc6] = OPCODE_SET_OBJECT_GRAPHIC,
	[0xc9] = OPCODE_MOVE_CURRENT_OBJECT_TO_ROOM,
	[0xcd] = OPCODE_SET_STRING_REPLACEMENT,
	[0xd1] = OPCODE_MOVE_DIRECTION,
	[0xd5] = OPCODE_DRAW_ROOM,
	[0xd9] = OPCODE_DRAW_OBJECT,
	[0xdd] = OPCODE_VAR_INC,
	[0xe1] = OPCODE_MOVE_OBJECT_TO_CURRENT_ROOM,
	[0xed] = OPCODE_REMOVE_OBJECT,
	[0xf0] = OPCODE_DROP_CURRENT_OBJECT,
	[0xfc] = OPCODE_REMOVE_CURRENT_OBJECT,
};

uint8_t *get_opcode_map(struct comprehend_game *game)
{
	switch (game->info->comprehend_version) {
	case 1:
		return opcode_map_v1;
		break;
	case 2:
		return opcode_map_v2;
	default:
		fatal_error("Unsupported Comprehend version %d\n",
			    game->info->comprehend_version);

		/* Not reached */
		return NULL;
	}
}
