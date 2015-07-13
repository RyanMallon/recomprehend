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

#ifndef _RECOMPREHEND_UTIL_H
#define _RECOMPREHEND_UTIL_H

#include <stdbool.h>
#include <stdio.h>

#define DEBUG_IMAGE_DRAW	(1 << 0)
#define DEBUG_GAME_STATE	(1 << 1)
#define DEBUG_FUNCTIONS		(1 << 2)
#define DEBUG_ALL		(~0)

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))

#define fatal_error(fmt, args...)				\
	do {							\
		__fatal_error(__func__, __LINE__, fmt, ##args);	\
	} while (0)

void __fatal_error(const char *func, unsigned line, const char *fmt, ...);
void fatal_strerror(int err, const char *fmt, ...);
void *xmalloc(size_t size);
char *xstrndup(const char *str, size_t size);

void debug_printf(unsigned flags, const char *fmt, ...);
void debug_enable(unsigned flags);
void debug_disable(unsigned flags);
bool debugging_enabled(void);

#endif /* _RECOMPREHEND_UTIL_H */
