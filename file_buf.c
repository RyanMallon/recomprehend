/*
 * This file is public domain
 *
 * Ryan Mallon, 2015
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdint.h>
#include <endian.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "file_buf.h"
#include "util.h"

int file_buf_map_may_fail(const char *filename, struct file_buf *fb)
{
	struct stat s;
	FILE *fd;

	memset(fb, 0, sizeof(*fb));

	fd = fopen(filename, "rb");
	if (!fd)
		return -errno;

	fstat(fileno(fd), &s);

	fb->data = xmalloc(s.st_size);
	fb->size = s.st_size;
	fread(fb->data, 1, fb->size, fd);
	fclose(fd);

	fb->p = fb->data;

	// FIXME - remove
	fb->marked = malloc(fb->size);
	memset(fb->marked, 0, fb->size);

	return 0;
}

void file_buf_map(const char *filename, struct file_buf *fb)
{
	int err;

	err = file_buf_map_may_fail(filename, fb);
	if (err)
		fatal_strerror(errno, "Cannot open file '%s'", filename);
}

void file_buf_unmap(struct file_buf *fb)
{
	free(fb->marked);
	free(fb->data);
}

void file_buf_set_pos(struct file_buf *fb, unsigned pos)
{
	if (pos > fb->size)
		fatal_error("Bad position set in file (%x > %x)\n",
			    pos, fb->size);

	fb->p = fb->data + pos;
}

unsigned file_buf_get_pos(struct file_buf *fb)
{
	return fb->p - fb->data;
}

void *file_buf_data_pointer(struct file_buf *fb)
{
	return fb->p;
}

size_t file_buf_strlen(struct file_buf *fb, bool *eof)
{
	uint8_t *end;

	if (eof)
		*eof = false;

	end = memchr(fb->p, '\0', fb->size - file_buf_get_pos(fb));
	if (!end) {
		/* No null terminator - string is remaining length */
		if (eof)
			*eof = true;
		return fb->size - file_buf_get_pos(fb);
	}

	return end - fb->p;
}

void file_buf_get_data(struct file_buf *fb, void *data, size_t data_size)
{
	if (file_buf_get_pos(fb) + data_size > fb->size)
		fatal_error("Not enough data in file (%x + %x > %x)\n",
			    file_buf_get_pos(fb), data_size, fb->size);

	if (data)
		memcpy(data, fb->p, data_size);

	/* Mark this region of the file as read */
	memset(fb->marked + file_buf_get_pos(fb), '?', data_size);

	fb->p += data_size;
}

void file_buf_get_u8(struct file_buf *fb, uint8_t *val)
{
	file_buf_get_data(fb, val, sizeof(*val));
}

void file_buf_get_le16(struct file_buf *fb, uint16_t *val)
{
	file_buf_get_data(fb, val, sizeof(*val));
	*val = le16toh(*val);
}

/*
 * Debugging function to show regions of a file that have not been read.
 */
void file_buf_show_unmarked(struct file_buf *fb)
{
	int i, start = -1;

	for (i = 0; i < fb->size; i++) {
		if (!fb->marked[i] && start == -1)
			start = i;

		if ((fb->marked[i] || i == fb->size - 1) && start != -1) {
			printf("%.4x - %.4x unmarked (%d bytes)\n", 
			       start, i - 1, i - start);
			start = -1;
		}
	}
}

void file_buf_put_u8(FILE *fd, uint8_t val)
{
	fwrite(&val, sizeof(uint8_t), 1, fd);
}

void file_buf_put_le16(FILE *fd, uint16_t val)
{
	fwrite(&val, sizeof(uint16_t), 1, fd);
}

void file_buf_put_skip(FILE *fd, size_t skip)
{
	int i;

	for (i = 0; i < skip; i++)
		file_buf_put_u8(fd, 0);
}

