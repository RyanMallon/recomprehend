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

#ifndef RECOMPREHEND_FILE_BUF_H
#define RECOMPREHEND_FILE_BUF_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

struct file_buf {
	uint8_t		*data;
	size_t		size;
	uint8_t		*p;

	uint8_t		*marked;
};

void file_buf_map(const char *filename, struct file_buf *fb);
int file_buf_map_may_fail(const char *filename, struct file_buf *fb);
void file_buf_unmap(struct file_buf *fb);
void file_buf_show_unmarked(struct file_buf *fb);

void *file_buf_data_pointer(struct file_buf *fb);
void file_buf_set_pos(struct file_buf *fb, unsigned pos);
unsigned file_buf_get_pos(struct file_buf *fb);

size_t file_buf_strlen(struct file_buf *fb, bool *eof);

void file_buf_get_data(struct file_buf *fb, void *data, size_t data_size);
void file_buf_get_u8(struct file_buf *fb, uint8_t *val);
void file_buf_get_le16(struct file_buf *fb, uint16_t *val);

#define file_buf_get_array(fb, type, base, array, member, size)		\
	do {								\
		int __i;						\
		for (__i = (base); __i < (base) + (size); __i++)	\
			file_buf_get_##type(fb, &((array)[__i]).member); \
	} while (0)

#define file_buf_get_array_u8(fb, base, array, member, size) \
	file_buf_get_array(fb, u8, base, array, member, size)

#define file_buf_get_array_le16(fb, base, array, member, size) \
	file_buf_get_array(fb, le16, base, array, member, size)

void file_buf_put_skip(FILE *fd, size_t skip);
void file_buf_put_u8(FILE *fd, uint8_t val);
void file_buf_put_le16(FILE *fd, uint16_t val);

#define file_buf_put_array(fd, type, base, array, member, size)		\
	do {								\
		int __i;						\
		for (__i = (base); __i < (base) + (size); __i++)	\
			file_buf_put_##type(fd, (array)[__i].member);	\
	} while (0)

#define file_buf_put_array_le16(fd, base, array, member, size) \
	file_buf_put_array(fd, le16, base, array, member, size)

#define file_buf_put_array_u8(fd, base, array, member, size) \
	file_buf_put_array(fd, u8, base, array, member, size)

#endif /* RECOMPREHEND_FILE_BUF_H */
